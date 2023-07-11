#include <QApplication>
#include <QAction>
#include <QSaveFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>
#include <QStringListModel>
#include <QDesktopServices>
#include <QUrl>

#include <KTextEdit>
#include <KLocalizedString>
#include <KActionCollection>
#include <KStandardAction>
#include <KMessageBox>
#include <KIO/Job>

#include <stamina/Stamina.h>

#include "MainWindow.h"

#include <iostream>
#include <regex>
#include <filesystem>

#include "stamina/Stamina.h"

namespace stamina {
namespace gui {


MainWindow::MainWindow(QWidget *parent)
	: KXmlGuiWindow(parent)
	, sfdm(new KFileCustomDialog(this))
	, ofdm(new KFileCustomDialog(this))
	, sfdp(new KFileCustomDialog(this))
	, ofdp(new KFileCustomDialog(this))
	, about(new About(this))
	, prefs(new Preferences(this))
	, propWizard(new PropertyWizard(this))
	, modelFindReplace(new FindReplace(this))
	, propFindReplace(new FindReplace(this))
	, modCompleter(new QCompleter(this))
	, propCompleter(new QCompleter(this))
	, activeModelFile("")
	, activePropertiesFile("")
	, unsavedChangesModel(false)
	, unsavedChangesProperty(false)
	, baseWindowTitle("New File")
	, modelWasBuilt(false)
	, modelActive(true)
	, stayOpen(true)
{
	setupActions();
}

void
MainWindow::setupActions() {
	ui.setupUi(this);
	modelFindReplace->place(this->ui.modelFileLayout1, this->ui.modelFile);
	propFindReplace->place(this->ui.propertySideVBox, this->ui.propertiesEditor);
	// Set up the autocompleters
	auto completerWordModFileModel = modelFromFile(":/resources/wordlist.txt", modCompleter);
	auto completerWordPropFileModel = modelFromFile(":/resources/wordlist.txt", propCompleter);
	modCompleter->setModel(completerWordModFileModel);
	propCompleter->setModel(completerWordPropFileModel);
	modCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	propCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	modCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	propCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	modCompleter->setWrapAround(false);
	propCompleter->setWrapAround(false);
	ui.modelFile->setCompleter(modCompleter);
	ui.propertiesEditor->setCompleter(propCompleter);
	// Connect slots
	connect(
		ui.actionOpen
		, SIGNAL(triggered())
		, this
		, SLOT(openModelFile())
	);
	connect(
		ui.actionPreferences
		, SIGNAL(triggered())
		, this
		, SLOT(showPreferences())
	);
	connect(
		ui.actionSave
		, SIGNAL(triggered())
		, this
		, SLOT(saveModelFile())
	);
	connect(
		ui.actionSave_As
		, SIGNAL(triggered())
		, this
		, SLOT(saveModelFileAs())
	);
	connect(
		ui.actionAbout_STAMINA
		, SIGNAL(triggered())
		, this
		, SLOT(showAbout())
	);

	connect(
		ui.actionAbout_Qt
		, &QAction::triggered
		, this
		, &QApplication::aboutQt
	);
	// Actions which are connected to lambdas
	connect(
		ui.actionModel_Editor
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.mainTabs->setCurrentIndex(0);
			this->ui.actionProperties_Editor->setChecked(false);
			this->ui.actionResults_Viewer->setChecked(false);
		}
	);
	connect(
		ui.actionProperties_Editor
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.mainTabs->setCurrentIndex(1);
			this->ui.actionModel_Editor->setChecked(false);
			this->ui.actionResults_Viewer->setChecked(false);
		}
	);
	connect(
		ui.actionResults_Viewer
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.mainTabs->setCurrentIndex(2);
			this->ui.actionModel_Editor->setChecked(false);
			this->ui.actionProperties_Editor->setChecked(false);
		}
	);
	connect(
		ui.actionModule
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.modelFile->insertPlainText("module MODULE_NAME\n\nendmodule\n");
			ui.actionModel_Editor->trigger();
		}
	);
	connect(
		ui.actionUnbounded_Variable
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.modelFile->insertPlainText("\nVARIABLE_NAME : int init 0;\n");
			ui.actionModel_Editor->trigger();
		}
	);
	connect(
		ui.actionBounded_Variable
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.modelFile->insertPlainText("\nVARIABLE_NAME : [LOWER_BOUND..UPPER_BOUND] init 0;\n");
			ui.actionModel_Editor->trigger();
		}
	);
	connect(
		ui.actionForever_Property
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.propertiesEditor->insertPlainText("\n// Forever property\nP=?[TODO: Forever property];\n");
			ui.actionProperties_Editor->trigger();
		}
	);
	connect(
		ui.actionUntil_Property
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.propertiesEditor->insertPlainText("\n// Until property. PSI_1 and PSI_2 are state formulas,\n");
			this->ui.propertiesEditor->insertPlainText("// meaning they evaluate to either TRUE or FALSE for a particular state\n");
			this->ui.propertiesEditor->insertPlainText("// To create an \"eventually\" formula, replace PSI_1 with `true`\n");
			this->ui.propertiesEditor->insertPlainText("P=?[ PSI_1 U PSI_2]");
			ui.actionProperties_Editor->trigger();
		}
	);
	connect(
		ui.actionBounded_Until_Property
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.propertiesEditor->insertPlainText("\n// BOUNDED Until property. PSI_1 and PSI_2 are state formulas,\n");
			this->ui.propertiesEditor->insertPlainText("// meaning they evaluate to either TRUE or FALSE for a particular state\n");
			this->ui.propertiesEditor->insertPlainText("// Note that T_0 and T_1 are time bounds.\n");
			this->ui.propertiesEditor->insertPlainText("P=?[ PSI_1 U[T_0, T_1] PSI_2]");
			ui.actionProperties_Editor->trigger();
		}
	);
	connect(
		ui.actionPRISM_Language
		, &QAction::triggered
		, this
		, [this]() {
			if (unsavedChangesModel) {
				bool shouldSave = KMessageBox::questionYesNo(0
				, i18n("You have unsaved changes in your current model file, would you like to save them?")
				) == KMessageBox::Yes;
				if (shouldSave) {
					this->saveModelFile();
				}
			}
			this->ui.modelFile->setPlainText("// New Model file\n// Autogenerated by xSTAMINA");
			setCaption("New File");
			ui.actionModel_Editor->trigger();
		}
	);
	connect(
		ui.actionJANI_Language
		, &QAction::triggered
		, this
		, [this]() {
			KMessageBox::sorry(this, "Not currently supported!");
		}
	);
	connect(
		ui.actionProperties_File
		, &QAction::triggered
		, this
		, [this]() {
			if (unsavedChangesProperty) {
				bool shouldSave = KMessageBox::questionYesNo(0
				, i18n("You have unsaved changes in your current properties file, would you like to save them?")
				) == KMessageBox::Yes;
				if (shouldSave) {
					this->savePropertyFile();
				}
			}
			this->ui.propertiesEditor->setPlainText("// New Properties file\n// Autogenerated by xSTAMINA");
			setCaption("New File");
			ui.actionProperties_Editor->trigger();
		}
	);
	connect(
		ui.actionQuit
		, &QAction::triggered
		, this
		, [this]() {
			this->handleClose();
			StaminaMessages::info("Goodbye to xSTAMINA");
			exit(0);
		}
	);
	connect(
		ui.actionFind
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) {
				modelFindReplace->show(false);
				modelFindReplace->focusFind();
			}
			else if (idx == 1) {
				propFindReplace->show(false);
				propFindReplace->focusFind();
			}
		}
	);

	connect(
		ui.actionReplace
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) {
				modelFindReplace->show(true);
				modelFindReplace->focusFind();
			}
			else if (idx == 1) {
				propFindReplace->show(true);
				propFindReplace->focusFind();
			}
		}
	);
	connect(
		ui.actionShow_toolbar
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.mainToolBar->setVisible(this->ui.actionShow_toolbar->isChecked());
		}
	);
	connect(
		ui.actionZoom_In
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) {
				ui.modelFile->zoomIn();
				// modCompleter->zoomIn();
				modZoom++;
			}
			else if (idx == 1) {
				ui.propertiesEditor->zoomIn();
				// propCompleter->zoomIn();
				propZoom++;
			}
		}
	);
	connect(
		ui.actionZoom_Out
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) {
				ui.modelFile->zoomOut();
				modZoom--;
			}
			else if (idx == 1) {
				ui.propertiesEditor->zoomOut();
				propZoom--;
			}
		}
	);
	connect(
		ui.actionReset_Text_Size
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) {
				if (modZoom > 0) {
					ui.modelFile->zoomIn(-modZoom);
				}
				else {
					ui.modelFile->zoomOut(-modZoom);
				}
			}
			else if (idx == 1) {
				if (propZoom > 0) {
					ui.propertiesEditor->zoomIn(-propZoom);
				}
				else {
					ui.propertiesEditor->zoomOut(-propZoom);
				}
			}
		}
	);
	connect(
		ui.actionShow_Statusbar
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.statusbar->setVisible(this->ui.actionShow_Statusbar->isChecked());
		}
	);
	connect(
		ui.actionLock_Toolbar
		, &QAction::triggered
		, this
		, [this]() {
			this->ui.mainToolBar->setMovable(!this->ui.actionLock_Toolbar->isChecked());
		}
	);
	connect(
		ui.actionDocumentation
		, &QAction::triggered
		, this
		, [this]() {
			QDesktopServices::openUrl(QUrl("https://staminachecker.org/documentation/wiki/"));
		}
	);
	connect(
		ui.actionReport_A_Bug
		, &QAction::triggered
		, this
		, [this]() {
			QDesktopServices::openUrl(QUrl("https://github.com/fluentverification/stamina-storm/issues"));
		}
	);
	// Non-action slots to connect
	connect(
		ui.modelFile
		, SIGNAL(textChanged())
		, this
		, SLOT(setModifiedModel())
	);
	connect(
		ui.propertiesEditor
		, SIGNAL(textChanged())
		, this
		, SLOT(setModifiedProperties())
	);

	connect(
		ui.propertyWizardButton
		, SIGNAL(clicked())
		, this
		, SLOT(showPropertyWizard())
	);

	connect(
		ui.buildModelButton
		, SIGNAL(clicked())
		, this
		, SLOT(checkModelAndProperties())
	);

	connect(
		ui.addConstantButton
		, &QPushButton::clicked
		, this
		, [this]() {
			ui.constantsTable->setRowCount(ui.constantsTable->rowCount() + 1);
		}
	);

	connect(
		ui.createLabelButton
		, &QPushButton::clicked
		, this
		, [this]() {
			ui.labelTabel->setRowCount(ui.labelTabel->rowCount() + 1);
		}
	);

	connect(
		ui.mainTabs
		, SIGNAL(currentChanged(int))
		, this
		, SLOT(handleTabChange())
	);
}

void
MainWindow::saveToActiveModelFile() {
	if (activeModelFile != "") {
		StaminaMessages::info("Saving active model file to path '" + activeModelFile.toStdString() + "'");
		QSaveFile file(activeModelFile);
		file.open(QIODevice::WriteOnly);

		QByteArray bArray;
		bArray.append(ui.modelFile->toPlainText().toUtf8());
		file.write(bArray);
		file.commit();
		unsavedChangesModel = false;
		setCaption(baseWindowTitle);
		stayOpen = false;
	}
	else {
		StaminaMessages::error("No active model file exists! (This is a bug, please report.)");
	}
}

void
MainWindow::saveToActivePropertiesFile() {
	if (activePropertiesFile != "") {
		StaminaMessages::info("Saving active properties file to path '" + activePropertiesFile.toStdString() + "'");
		QSaveFile file(activePropertiesFile);
		file.open(QIODevice::WriteOnly);

		QByteArray bArray;
		bArray.append(ui.propertiesEditor->toPlainText().toUtf8());
		file.write(bArray);
		file.commit();
		unsavedChangesProperty = false;
		setCaption(baseWindowTitle);
		stayOpen = false;
	}
	else {
		StaminaMessages::error("No active model file exists! (This is a bug, please report.)");
	}
}

void
MainWindow::showPreferences() {
	StaminaMessages::info("Showing preferences dialog");
	prefs->exec();
}

void
MainWindow::openModelFile() {
	if (unsavedChangesModel) {
		bool shouldNotDiscard = KMessageBox::questionYesNo(0
			, i18n("You have unsaved changes to your model file! Would you like to save it now?")
		) == KMessageBox::Yes;
		if (shouldNotDiscard) {
			saveModelFile();
		}
	}
	StaminaMessages::info("Opening model file");
	ofdm->setOperationMode(KFileWidget::Opening);
	ofdm->fileWidget()->setFilter(QString("*.prism *.sm|PRISM Model files\n*.jani|JANI Model Files"));
	connect(
		ofdm->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(openModelFromAcceptedPath())
	);
	ui.statusbar->showMessage(tr("Showing open dialog."));
	ofdm->show();
// 	QString openFileName = KFileDialog::getOpenFileName(this, i18n("Open model file"));
}

void
MainWindow::openModelFromAcceptedPath() {
	QString selectedFile = ofdm->fileWidget()->selectedFile();
	StaminaMessages::info( "Opening file " + selectedFile.toStdString());
	activeModelFile = selectedFile;
	disconnect(
		ofdm->fileWidget()
		, SIGNAL(accepted())
		, 0
		, 0
	);
	if (selectedFile != "") {
		QFileInfo info(selectedFile);
		baseWindowTitle = info.fileName();
		setCaption(baseWindowTitle);
		KIO::Job * job = KIO::storedGet(QUrl::fromLocalFile(selectedFile));
		connect(job, SIGNAL(result(KJob *)), this, SLOT(downloadFinishedModel(KJob*)));
		job->exec();
		// Check to see if a property file with the same filename
		// is in the same directory. If so, ask if the user wishes to open that as well
		std::string baseFileName = std::regex_replace(selectedFile.toStdString(), std::regex("\\.prism|\\.sm"), "");
		std::string propFileName = baseFileName + ".csl";
		if (std::filesystem::exists(propFileName)) {
			// Ask the user if they want to open
			bool shouldOpenPropFile = KMessageBox::questionYesNo(0
			, i18n("There appears to be a property file in this directory with the same base name as the model file you opened. Would you like to open this property file as well?")
			) == KMessageBox::Yes;
			if (shouldOpenPropFile) {
				QString pFileName = QString::fromStdString(propFileName);
				QFileInfo pInfo(pFileName);
				activePropertiesFile = pFileName;
				StaminaMessages::info("Also opening prop file (due to name similarity): " + propFileName);
				KIO::Job * pJob = KIO::storedGet(QUrl::fromLocalFile(pFileName));
				connect(pJob, SIGNAL(result(KJob *)), this, SLOT(downloadFinishedProperty(KJob*)));
				pJob->exec();
			}
		}
	}

}

void
MainWindow::openPropertyFromAcceptedPath() {
	QString selectedFile = ofdp->fileWidget()->selectedFile();
	StaminaMessages::info( "Opening file " + selectedFile.toStdString());
	activePropertiesFile = selectedFile;
	disconnect(
		ofdp->fileWidget()
		, SIGNAL(accepted())
		, 0
		, 0
	);
	if (selectedFile != "") {
		QFileInfo info(selectedFile);
		baseWindowTitle = info.fileName();
		setCaption(baseWindowTitle);
		KIO::Job * job = KIO::storedGet(QUrl::fromLocalFile(selectedFile));
		connect(job, SIGNAL(result(KJob *)), this, SLOT(downloadFinishedProperty(KJob*)));
		job->exec();
	}

}

void
MainWindow::saveModelFile() {
	if (this->activeModelFile == "") {
		this->saveModelFileAs();
	}
	else {
		saveToActiveModelFile();
	}
	unsavedChangesModel = false;
	ui.statusbar->showMessage(tr("Saved model file."));
}

void
MainWindow::saveModelFileAs() {
	StaminaMessages::info("Saving model file as...");
	sfdm->setOperationMode(KFileWidget::Saving);
	connect(
		sfdm->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(setActiveModelFileAndSave())
	);
	sfdm->show();
	sfdm->fileWidget()->setFilter(QString("*.prism *.sm|PRISM Model files\n*.jani|JANI Model Files"));
// 	saveToActiveModelFile();
}

void
MainWindow::openPropertyFile() {
	if (unsavedChangesProperty) {
		bool shouldNotDiscard = KMessageBox::questionYesNo(0
		, i18n("You have unsaved changes to your property file! Would you like to save it now?")
		) == KMessageBox::Yes;
		if (shouldNotDiscard) {
			savePropertyFile();
		}
	}
	StaminaMessages::info("Opening property file");
	ofdp->setOperationMode(KFileWidget::Opening);
	ofdp->fileWidget()->setFilter(QString("*.csl *.pctl|PRISM Property Files"));
	connect(
		ofdp->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(openPropertyFromAcceptedPath())
	);
	ui.statusbar->showMessage(tr("Showing open dialog."));
	ofdp->show();
}

void
MainWindow::savePropertyFile() {
	if (this->activePropertiesFile == "") {
		this->savePropertyFileAs();
	}
	else {
		saveToActivePropertiesFile();
	}
	unsavedChangesProperty = false;
	ui.statusbar->showMessage(tr("Saved properties file."));
}

void
MainWindow::savePropertyFileAs() {
	sfdp->setOperationMode(KFileWidget::Saving);
	connect(
		sfdp->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(setActivePropertyFileAndSave())
	);
}

void
MainWindow::downloadFinishedModel(KJob* job) {
	if (job->error()) {
		KMessageBox::error(this, job->errorString());
		activeModelFile.clear();
		ui.statusbar->showMessage(tr("Error in opening model file."));
		return;
	}

	KIO::StoredTransferJob * storedJob = (KIO::StoredTransferJob*) job;

	ui.modelFile->setPlainText(QTextStream(storedJob->data(), QIODevice::ReadOnly).readAll());
	StaminaMessages::good("Succesfully loaded file into model editor!");
	unsavedChangesModel = false;
	ui.statusbar->showMessage(tr("Opened model file."));
}

void
MainWindow::downloadFinishedProperty(KJob* job) {
	if (job->error()) {
		KMessageBox::error(this, job->errorString());
		activeModelFile.clear();
		ui.statusbar->showMessage(tr("Error in opening property file."));
		return;
	}

	KIO::StoredTransferJob * storedJob = (KIO::StoredTransferJob*) job;

	ui.propertiesEditor->setPlainText(QTextStream(storedJob->data(), QIODevice::ReadOnly).readAll());
	StaminaMessages::good("Succesfully loaded file into property editor!");
	unsavedChangesProperty = false;
	ui.statusbar->showMessage(tr("Opened property file."));
}

void
MainWindow::showAbout() {
	this->about->show();
}

void
MainWindow::setModifiedModel() {
	if (unsavedChangesModel) { return; }
// 	baseWindowTitle = windowTitle();
	setCaption(baseWindowTitle + " * ");
	unsavedChangesModel = true;
}

void
MainWindow::setModifiedProperties() {
	if (unsavedChangesProperty) { return; }
	setCaption(baseWindowTitle + " * ");
	unsavedChangesProperty = true;
}

void
MainWindow::setActiveModelFileAndSave() {
	// Disconnect sfd just in case it sent us here
	disconnect(
		sfdm->fileWidget()
		, SIGNAL(accepted())
		, 0
		, 0
	);
	activeModelFile = sfdm->fileWidget()->selectedFile();
	saveToActiveModelFile();
}

void
MainWindow::setActivePropertyFileAndSave() {
	disconnect(
		sfdp->fileWidget()
		, SIGNAL(accepted())
	);
	activePropertiesFile = sfdp->fileWidget()->selectedFile();
	saveToActivePropertiesFile();
}

void
MainWindow::closeEvent(QCloseEvent *event) {
	handleClose();
}

void
MainWindow::handleClose() {
	// stayOpen = true;
	if (unsavedChangesModel) {
		bool shouldSave = KMessageBox::questionYesNo(0
			, i18n("You have unsaved changes to your model file! Would you like to save it now?")
		) == KMessageBox::Yes;
		if (shouldSave) {
			saveModelFile();
			// while (stayOpen) {}
		}
	}
	else if (unsavedChangesProperty) {
		bool shouldSave = KMessageBox::questionYesNo(0
		, i18n("You have unsaved changes to your model file! Would you like to save it now?")
		) == KMessageBox::Yes;
		if (shouldSave) {
			savePropertyFile();
			// while (stayOpen) {}
		}
	}
}

void
MainWindow::showPropertyWizard() {
	if (!modelWasBuilt) {
		bool shouldShow = KMessageBox::warningContinueCancel(0
			, i18n("The model has not been built yet! Variable names will not appear in the Property Wizard.")
		) == KMessageBox::Continue;
		if (!shouldShow) {
			return;
		}
	}
	propWizard->show();
	// TODO: connect the close() slot to appending to the text file
}

void
MainWindow::checkModelAndProperties() {
	// Trigger save of model and properties file
	if (unsavedChangesModel || unsavedChangesProperty) {
		bool shouldSave = KMessageBox::questionYesNo(0
			, i18n("You have unsaved changes in either your model or properties file. These must be saved before checking. Save now?")
		) == KMessageBox::Yes;
		if (!shouldSave) {
			ui.statusbar->showMessage(tr("Did not save model file."));
			return;
		}
	}
	else if (activeModelFile == "" || activePropertiesFile == "") {
		bool shouldSave = KMessageBox::questionYesNo(0
			, i18n("Your model or properties file has not been saved on the filesystem. Save now?")
		) == KMessageBox::Yes;
		if (!shouldSave) {
			ui.statusbar->showMessage(tr("Did not save properties file."));
			return;
		}
	}
	saveModelFile();
	savePropertyFile();
	std::string modFile = this->activeModelFile.toStdString();
	std::string propFile = this->activePropertiesFile.toStdString();
	StaminaMessages::info("Checking model file: '" + modFile + "' and prop file '" + propFile + "'");
	core::Options::model_file = modFile;
	core::Options::properties_file = propFile;
	prefs->getPreferencesFromUI();
	prefs->setOptionsFromPreferences();
	Stamina s; // TODO: create constructor for stamina::Stamina class without struct args*
	ui.statusbar->showMessage(tr("Running."));
	s.run();
	ui.statusbar->showMessage(tr("Finished."));
	// auto staminaProcess = []() {
	auto & resultsTable = s.getResultTable();
	ui.simulationResultsTable->setRowCount(resultsTable.size());
	int currentRow = 0;
	for (auto & result : resultsTable) {
		// The property name
		ui.simulationResultsTable->setItem(
			currentRow
			, 0
			, new QTableWidgetItem(QString::fromStdString(result.property))
		);
		// Minimum probability
		ui.simulationResultsTable->setItem(
			currentRow
			, 1
			, new QTableWidgetItem(QString::number(result.pMin))
		);
		// Maximum probability
		ui.simulationResultsTable->setItem(
			currentRow
			, 2
			, new QTableWidgetItem(QString::number(result.pMax))
		);
		currentRow++;
	}
	// KMessageBox::
	ui.actionResults_Viewer->trigger();
	// ui.mainTabs->setCurrentIndex(2); // 2 is the index of the "results" tab
	// };
	// QTimer::singleShot(0, this, staminaProcess);
}

void
MainWindow::handleTabChange() {
	int tabIndex = ui.mainTabs->currentIndex();
	modelActive = tabIndex != 1;
	// StaminaMessages::info("Model active is " + std::to_string(modelActive));
	// Disconnect the open and save actions
	disconnect(ui.actionOpen, SIGNAL(triggered()), 0, 0);
	disconnect(ui.actionSave, SIGNAL(triggered()), 0, 0);
	if (modelActive) {
		connect(
			ui.actionOpen
			, SIGNAL(triggered())
			, this
			, SLOT(openModelFile())
		);
		connect(
			ui.actionSave
			, SIGNAL(triggered())
			, this
			, SLOT(saveModelFile())
		);
	}
	else {
		connect(
			ui.actionOpen
			, SIGNAL(triggered())
			, this
			, SLOT(openPropertyFile())
		);
		connect(
			ui.actionSave
			, SIGNAL(triggered())
			, this
			, SLOT(savePropertyFile())
		);
	}
}

QAbstractItemModel *
MainWindow::modelFromFile(const QString & fileName, QCompleter * completer)
{
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly)) {
		StaminaMessages::error("Keyword list is empty!");
		return new QStringListModel(completer);
	}

#ifndef QT_NO_CURSOR
	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
	QStringList words;

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		if (!line.isEmpty()) {
			words << QString::fromUtf8(line.trimmed());
		}
	}

#ifndef QT_NO_CURSOR
	QGuiApplication::restoreOverrideCursor();
#endif
	return new QStringListModel(words, completer);
}

} // namespace gui
} // namespace stamina
