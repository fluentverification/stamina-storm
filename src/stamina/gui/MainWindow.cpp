#include <QApplication>
#include <QAction>
#include <QSaveFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>
#include <QStringListModel>
#include <QDesktopServices>
#include <QUrl>
#include <QtConcurrent>

#include <KTextEdit>
#include <KLocalizedString>
#include <KActionCollection>
#include <KStandardAction>
#include <KMessageBox>
#include <KIO/Job>


#include "MainWindow.h"

#include <iostream>
#include <regex>
#include <filesystem>

namespace stamina {
namespace gui {


MainWindow::MainWindow(QWidget *parent)
	: KXmlGuiWindow(parent)
	, sfdm(new KFileCustomDialog(this))
	, ofdm(new KFileCustomDialog(this))
	, sfdp(new KFileCustomDialog(this))
	, ofdp(new KFileCustomDialog(this))
	, exportDialog(new KFileCustomDialog(this))
	, about(new About(this))
	, prefs(new Preferences(this))
	, propWizard(new PropertyWizard(this))
	, modelFindReplace(new FindReplace(this))
	, propFindReplace(new FindReplace(this))
	, modCompleter(new QCompleter(this))
	, propCompleter(new QCompleter(this))
	, progress(new QProgressBar(this))
	, killButton(new QPushButton(this))
	, activeModelFile("")
	, activePropertiesFile("")
	, unsavedChangesModel(false)
	, unsavedChangesProperty(false)
	, baseWindowTitle("New File")
	, modelWasBuilt(false)
	, modelActive(true)
	, stayOpen(true)
{
	// Make it so StaminaMessages doesn't kill the program
	StaminaMessages::raiseExceptionsRatherThanExit = true;
	ui.setupUi(this);
	setupActions();
	setup();
	progress->setRange(0, 0);
	progress->setTextVisible(false);
	ui.statusbar->addPermanentWidget(killButton);
	ui.statusbar->addPermanentWidget(progress);
	QIcon icon;
	QString iconThemeName = QString::fromUtf8("process-stop");
	if (QIcon::hasThemeIcon(iconThemeName)) {
		icon = QIcon::fromTheme(iconThemeName);
	} else {
		icon.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
	}
	progress->hide();
	killButton->hide();
	killButton->setIcon(icon);
	ui.earlyTerminatedGroup->hide();
}

void
MainWindow::setup() {
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
	prefs->setMainWindow(&ui);
	// Set the default sizes for the splitters
	// QList<int> modelSizes = ui.modelSplitter->sizes();
	// // int totalSize = modelSizes[0] + modelSizes[1];
	// int s = modelSizes[0] / 2;
	// modelSizes[0] -= s;
	// modelSizes[1] += s;
	// ui.modelSplitter->setSizes(modelSizes);
}

void
MainWindow::setupActions() {
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
	// Edit Actions
	connect(
		ui.actionCut
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) { this->ui.modelFile->cut(); }
			else if (idx == 1) { this->ui.propertiesEditor->cut(); }
		}
	);
	connect(
		ui.actionCopy
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) { this->ui.modelFile->copy(); }
			else if (idx == 1) { this->ui.propertiesEditor->copy(); }
		}
	);
	connect(
		ui.actionPaste
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) { this->ui.modelFile->paste(); }
			else if (idx == 1) { this->ui.propertiesEditor->paste(); }
		}
	);
	connect(
		ui.actionUndo
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) { this->ui.modelFile->undo(); }
			else if (idx == 1) { this->ui.propertiesEditor->undo(); }
		}
	);
	connect(
		ui.actionRedo
		, &QAction::triggered
		, this
		, [this]() {
			int idx = this->ui.mainTabs->currentIndex();
			if (idx == 0) { this->ui.modelFile->redo(); }
			else if (idx == 1) { this->ui.propertiesEditor->redo(); }
		}
	);
	// TODO: connect the undoAvailable(bool) signal to something that controls
	// the unsavedChanges* variables
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
		ui.deleteConstantButton
		, &QPushButton::clicked
		, this
		, [this]() {
			int row = ui.constantsTable->currentRow();
			ui.constantsTable->removeRow(row);
		}
	);

	connect(
		ui.createLabelButton
		, &QPushButton::clicked
		, this
		, [this]() {
			ui.labelTable->setRowCount(ui.labelTable->rowCount() + 1);
		}
	);

	connect(
		ui.deleteLabelButton
		, &QPushButton::clicked
		, this
		, [this]() {
			ui.labelTable->removeRow(ui.labelTable->currentRow());
		}
	);

	connect(
		ui.mainTabs
		, SIGNAL(currentChanged(int))
		, this
		, SLOT(handleTabChange())
	);

	connect(
		ui.exportCSVButton
		, SIGNAL(clicked())
		, this
		, SLOT(exportCSV())
	);

	connect(
		ui.sortAscendingButton
		, &QToolButton::clicked
		, this
		, [this]() { this->ui.simulationResultsTable->sortByColumn(0, Qt::AscendingOrder); }
	);

	connect(
		ui.sortDescendingButton
		, &QToolButton::clicked
		, this
		, [this]() { this->ui.simulationResultsTable->sortByColumn(0, Qt::DescendingOrder); }
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
		// setCaption(baseWindowTitle);
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
	if (!unsavedChangesModel) { return; }
	if (this->activeModelFile == "") {
		this->saveModelFileAs();
	}
	else {
		saveToActiveModelFile();
	}
	setCaption(activeModelFile);
	unsavedChangesModel = false;
	ui.statusbar->showMessage(tr("Saved model file."));
	initializeModel();
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
	if (!unsavedChangesProperty) { return; }
	if (this->activePropertiesFile == "") {
		this->savePropertyFileAs();
	}
	else {
		saveToActivePropertiesFile();
	}
	setCaption(activePropertiesFile);
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
	// TODO: There is a bug here that creates multiple '*' suffixes
	if (unsavedChangesModel) { return; }
	// baseWindowTitle = ;
	QString title = windowTitle();
	if (title[title.length() - 1] != '*') {
		setCaption(windowTitle() + " *");
	}
	unsavedChangesModel = true; // ui.modelFile->undoAvailable();
}

void
MainWindow::setModifiedProperties() {
	// TODO: There is a bug here that creates multiple '*' suffixes
	if (unsavedChangesProperty) { return; }
	// baseWindowTitle = windowTitle();
	QString title = windowTitle();
	if (title[title.length() - 1] != '*') {
		setCaption(windowTitle() + " *");
	}
	unsavedChangesProperty = true; // ui.propertiesEditor->undoAvailable();
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
MainWindow::initializeModel() {
	if (activePropertiesFile == "") {
		StaminaMessages::warning("Cannot initialize model tree without active property file!");
		return;
	}
	std::string modFile = this->activeModelFile.toStdString();
	std::string propFile = this->activePropertiesFile.toStdString();
	core::Options::model_file = modFile;
	core::Options::properties_file = propFile;
	s.initialize();
	populateModelInformationTree(s.getModelFile());
}

void
MainWindow::exportCSV() {
	exportDialog->setOperationMode(KFileWidget::Saving);
	exportDialog->fileWidget()->setFilter(QString("*.csv |CSV (Comma-Separated Value) Files"));
	connect(
		exportDialog->fileWidget()
		, &KFileWidget::accepted
		, this
		, [this] () {
			QString fileName = this->exportDialog->fileWidget()->selectedFile();
			QString csvData;
			for (int row = 0; row < ui.simulationResultsTable->rowCount(); row++) {
				for (int col = 0; col < ui.simulationResultsTable->columnCount(); col++) {
					csvData += ui.simulationResultsTable->item(row, col)
						->text().replace(QString(","), QString("\\,"));
					csvData += ",";
				}
				csvData += "\n";
			}
			QFile csvFile(fileName);
			if (csvFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				QTextStream out(&csvFile);
				out << csvData;
				csvFile.close();
			}
			ui.statusbar->showMessage(tr("Finished exporting results"));
		}
	);
	ui.statusbar->showMessage(tr("Exporting results to CSV file"));
	exportDialog->show();
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
	ui.statusbar->showMessage(tr("Running."));
	staminaJob = QtConcurrent::run([this]() {
		progress->show();
		killButton->show();
		try {
			s.run();
		}
		catch (std::string & e) {
			std::string msg = std::string("Error got while running STAMINA: ") + e;
			KMessageBox::sorry(nullptr, QString::fromStdString(msg));
		}
		ui.statusbar->showMessage(tr("Finished."));
		// KMessageBox::
		ui.actionResults_Viewer->trigger();
		populateResultsTable();
		// Populate some of the labels
		ui.statesLabel->setText(QString::number(s.getStateCount()));
		ui.initStatesLabel->setText(QString::number(1)); // TODO: actually get, although we only support models with one initial state
		ui.transitionsLabel->setText(QString::number(s.getTransitionCount()));
		// ui.mainTabs->setCurrentIndex(2); // 2 is the index of the "results" tab
		populateLabelTable();
		populateModelInformationTree(s.getModelFile());
		progress->hide();
		killButton->hide();
	});
	connect(
		killButton
		, &QPushButton::clicked
		, this
		, [this] () {
			QtConcurrent::run([this]() {
				StaminaMessages::info("Killing running job.");
				staminaJob.cancel();
				killButton->setEnabled(false);
				progress->setEnabled(false);
				killButton->setText("Cancelling");
				staminaJob.waitForFinished();
				progress->hide();
				killButton->hide();
				StaminaMessages::warning("Killed running job!");
				killButton->setText("");
				killButton->setEnabled(true);
				progress->setEnabled(true);
			});
		}
	);
	// QTimer::singleShot(0, this, staminaProcess);
}

void
MainWindow::populateResultsTable() {
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
}

void
MainWindow::populateModelInformationTree(std::shared_ptr<storm::prism::Program> program) {
	// Clear the tree
	ui.modelInfoTree->clear();

	// Get the data models for the completers so we can add the module names, variable names, and
	// formula names to them. Note modModel is the data model for the model file.
	QStringListModel * modModel = (QStringListModel *) modCompleter->model();
	QStringListModel * propModel = (QStringListModel *) propCompleter->model();
	QStringList modStringList = modModel->stringList();
	QStringList propStringList = propModel->stringList();

	// We should only get CTMCs, but this will allow for other types to be shown
	QTreeWidgetItem * typeItem = new QTreeWidgetItem(ui.modelInfoTree);
	ui.modelInfoTree->addTopLevelItem(typeItem);
	QString modelTypeText("Model");
	switch (program->getModelType()) {
		case storm::prism::Program::ModelType::DTMC:
			typeItem->setText(1, "DTMC");
			break;
		case storm::prism::Program::ModelType::CTMC:
			typeItem->setText(1, "CTMC");
			break;
		case storm::prism::Program::ModelType::MDP:
		case storm::prism::Program::ModelType::CTMDP:
		case storm::prism::Program::ModelType::MA:
		case storm::prism::Program::ModelType::POMDP:
		case storm::prism::Program::ModelType::PTA:
		case storm::prism::Program::ModelType::SMG:
			typeItem->setText(1, "Unsupportd by STAMINA");
			break;
		case storm::prism::Program::ModelType::UNDEFINED:
		default:
			typeItem->setText(1, "UNDEFINED");
	}
	typeItem->setText(0, modelTypeText);

	// Add constants to the tree
	QTreeWidgetItem * constsItem = new QTreeWidgetItem(ui.modelInfoTree);
	constsItem->setText(0, "Constants");
	ui.modelInfoTree->addTopLevelItem(constsItem);
	for (auto & constant : program->getConstants()) {
		QString constName = QString::fromStdString(constant.getName());
		QString typeString = QString::fromStdString(constant.getType().getStringRepresentation());
		QString expressionString = QString::fromStdString(constant.getExpression().toString());
		QTreeWidgetItem * constItem = new QTreeWidgetItem(constsItem);
		constItem->setText(0, constName);
		constItem->setText(1, typeString);
		constItem->setText(2, expressionString);
		// Insert into the completers
		modStringList << constName;
		propStringList << constName;
		// We also have the constants table on the second tab
		ui.constantsTable->setRowCount(ui.constantsTable->rowCount() + 1);
		int row = ui.constantsTable->rowCount() - 1;
		QTableWidgetItem * constTableItem = new QTableWidgetItem(constName);
		QTableWidgetItem * typeItem = new QTableWidgetItem(typeString);
		QTableWidgetItem * exprItem = new QTableWidgetItem(expressionString);
		constTableItem->setFlags(Qt::NoItemFlags);
		typeItem->setFlags(Qt::NoItemFlags);
		exprItem->setFlags(Qt::NoItemFlags);
		ui.constantsTable->setItem(row, 0, constTableItem);
		ui.constantsTable->setItem(row, 1, typeItem);
		ui.constantsTable->setItem(row, 2, exprItem);
	}

	// Add variable list to the tree
	QTreeWidgetItem * variablesItem = new QTreeWidgetItem(ui.modelInfoTree);
	variablesItem->setText(0, "Variables");
	ui.modelInfoTree->addTopLevelItem(variablesItem);
	for (auto & variable : program->getAllExpressionVariables()) {
		QTreeWidgetItem * varItem = new QTreeWidgetItem(variablesItem);
		QString varName = QString::fromStdString(variable.getName());
		varItem->setText(0, varName);
		varItem->setText(1, QString::fromStdString(
			variable.getType().getStringRepresentation()
		));
		// Insert this variable's name into our completers
		modStringList << varName;
		propStringList << varName;
	}
	// Add formula list to the tree
	QTreeWidgetItem * formulasItem = new QTreeWidgetItem(ui.modelInfoTree);
	formulasItem->setText(0, "Formulas");
	ui.modelInfoTree->addTopLevelItem(formulasItem);
	for (auto & formula : program->getFormulas()) {
		QTreeWidgetItem * formulaItem = new QTreeWidgetItem(formulasItem);
		formulaItem->setText(0, QString::fromStdString(formula.getName()));
		formulaItem->setText(1, QString::fromStdString(formula.getType().getStringRepresentation()));
		QString exprString = QString::fromStdString(formula.getExpression().toString());
		formulaItem->setText(2, exprString);
		formulaItem->setToolTip(0, exprString);
	}

	// Add modules to the tree
	QTreeWidgetItem * modulesItem = new QTreeWidgetItem(ui.modelInfoTree);
	modulesItem->setText(0, "Modules");
	ui.modelInfoTree->addTopLevelItem(modulesItem);
	for (auto & pModule : program->getModules()) {
		QTreeWidgetItem * modItem = new QTreeWidgetItem(modulesItem);
		QString moduleName = QString::fromStdString(pModule.getName());
		modItem->setText(0, moduleName);
		modStringList << moduleName;
		propStringList << moduleName;
		// Show the commands associated with the module
		QTreeWidgetItem * moduleCommandsItem = new QTreeWidgetItem(modItem);
		moduleCommandsItem->setText(0, "Commands");
		for (auto & command : pModule.getCommands()) {
			QTreeWidgetItem * commandsItem = new QTreeWidgetItem(moduleCommandsItem);
			commandsItem->setText(0, QString::fromStdString(
				"Action/Guard " + command.getActionName()
			));
			commandsItem->setText(2, QString::fromStdString(
				command.getGuardExpression().toString()
			));
			QTreeWidgetItem * updatesItem = new QTreeWidgetItem(commandsItem);
			updatesItem->setText(0, "Updates:");
			// Show the updates associated with the command
			for (auto & update : command.getUpdates()) {
				QTreeWidgetItem * updateItem = new QTreeWidgetItem(updatesItem);
				updateItem->setText(0, QString::fromStdString(
					update.getLikelihoodExpression().toString()
				));
				// Show the assignments associated with the update
				for (auto & assignment : update.getAssignments()) {
					QTreeWidgetItem * assignmentItem = new QTreeWidgetItem(updateItem);
					assignmentItem->setText(0, QString::fromStdString(
						assignment.getExpression().toString()
					));
				}
			}
		}
	}

	modModel->setStringList(modStringList);
	propModel->setStringList(propStringList);
}

void
MainWindow::populateLabelTable() {
	auto labelsAndCount = s.getLabelsAndCount();
	ui.labelTable->setRowCount(labelsAndCount->size());
	int currentRow = 0;
	for (auto labelCountPair : *labelsAndCount) {
		// The label name
		auto labelItem = new QTableWidgetItem(QString::fromStdString(labelCountPair.first));
		labelItem->setFlags(Qt::NoItemFlags);
		ui.labelTable->setItem(
			currentRow
			, 0
			, labelItem
		);
		// The label count
		auto countItem = new QTableWidgetItem(QString::number(labelCountPair.second));
		countItem->setFlags(Qt::NoItemFlags);
		ui.labelTable->setItem(
			currentRow
			, 1
			, countItem
		);
		currentRow++;
	}
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
	if (tabIndex == 0) {
		this->setCaption(
			((this->activeModelFile == "") ? "New Model File" : this->activeModelFile)
			+ ((this->unsavedChangesModel) ? " *" : "")
		);
		this->ui.actionModel_Editor->setChecked(true);
		this->ui.actionProperties_Editor->setChecked(false);
		this->ui.actionResults_Viewer->setChecked(false);

	}
	else if (tabIndex == 1) {
		this->setCaption(
			((this->activePropertiesFile == "") ? "New Properties File" : this->activePropertiesFile)
			+ ((this->unsavedChangesProperty) ? " *" : "")
		);
		this->ui.actionModel_Editor->setChecked(false);
		this->ui.actionProperties_Editor->setChecked(true);
		this->ui.actionResults_Viewer->setChecked(false);
	}
	else if (tabIndex == 2) {
		this->setCaption("Results");
		this->ui.actionModel_Editor->setChecked(false);
		this->ui.actionProperties_Editor->setChecked(false);
		this->ui.actionResults_Viewer->setChecked(true);
	}
	else {
		this->setCaption("Logs");
		this->ui.actionModel_Editor->setChecked(false);
		this->ui.actionProperties_Editor->setChecked(false);
		this->ui.actionResults_Viewer->setChecked(false);
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
