#include <QApplication>
#include <QAction>
#include <QSaveFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>

#include <KTextEdit>
#include <KLocalizedString>
#include <KActionCollection>
#include <KStandardAction>
#include <KMessageBox>
#include <KIO/Job>

#include <stamina/Stamina.h>

#include "MainWindow.h"

#include <iostream>

#include "stamina/Stamina.h"

namespace stamina {
namespace gui {


MainWindow::MainWindow(QWidget *parent)
	: KXmlGuiWindow(parent)
	, sfd(new KFileCustomDialog())
	, ofd(new KFileCustomDialog())
	, about(new About(this))
	, prefs(new Preferences(this))
	, propWizard(new PropertyWizard(this))
	, activeModelFile("")
	, activePropertiesFile("")
	, unsavedChangesModel(false)
	, unsavedChangesProperty(false)
	, baseWindowTitle("New File")
	, modelWasBuilt(false)
	, modelActive(true)
{
	setupActions();
}

void
MainWindow::setupActions() {
	ui.setupUi(this);
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
		ui.actionAbout_STAMINA_2
		, SIGNAL(triggered())
		, this
		, SLOT(showAbout())
	);
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

	// Connect on close to onClose()
	connect(
		ui
		, SIGNAL(onClose())
		, this
		, SLOT(onClose())
	);
	// setupGUI(Default, "mainWindow.ui");

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
	}
}

void
MainWindow::saveToActivePropertiesFile() {
	if (activePropertiesFile != "") {
		QSaveFile file(activePropertiesFile);
		file.open(QIODevice::WriteOnly);

		QByteArray bArray;
		bArray.append(ui.propertiesEditor->toPlainText().toUtf8());
		file.write(bArray);
		file.commit();
		unsavedChangesProperty = false;
		setCaption(baseWindowTitle);
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
	ofd->setOperationMode(KFileWidget::Opening);
	ofd->fileWidget()->setFilter(QString("*.prism *.sm|PRISM Model files\n*.jani|JANI Model Files"));
	connect(
		ofd->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(openFromAcceptedPath())
	);
	ofd->show();
// 	QString openFileName = KFileDialog::getOpenFileName(this, i18n("Open model file"));
}

void
MainWindow::openFromAcceptedPath() {
	QString selectedFile = ofd->fileWidget()->selectedFile();
	StaminaMessages::info( "Opening file " + selectedFile.toStdString());
	activeModelFile = selectedFile;
	disconnect(
		ofd->fileWidget()
		, SIGNAL(accepted())
	);
	if (selectedFile != "") {
		QFileInfo info(selectedFile);
		baseWindowTitle = info.fileName();
		setCaption(baseWindowTitle);
		KIO::Job * job = KIO::storedGet(QUrl::fromLocalFile(selectedFile));
		connect(job, SIGNAL(result(KJob *)), this, SLOT(downloadFinished(KJob*)));
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
}

void
MainWindow::saveModelFileAs() {
	StaminaMessages::info("Saving model file as...");
	sfd->setOperationMode(KFileWidget::Saving);
	connect(
		sfd->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(setActiveModelFileAndSave())
	);
	sfd->show();
	sfd->fileWidget()->setFilter(QString("*.prism *.sm|PRISM Model files\n*.jani|JANI Model Files"));
// 	saveToActiveModelFile();
}

void
MainWindow::openPropertyFile() {
	if (unsavedChangesModel) {
		bool shouldNotDiscard = KMessageBox::questionYesNo(0
		, i18n("You have unsaved changes to your property file! Would you like to save it now?")
		) == KMessageBox::Yes;
		if (shouldNotDiscard) {
			savePropertyFile();
		}
	}
	StaminaMessages::info("Opening property file");
	ofd->setOperationMode(KFileWidget::Opening);
	ofd->fileWidget()->setFilter(QString("*.csl *.pctl|PRISM Property Files"));
	connect(
		ofd->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(openFromAcceptedPath())
	);
	ofd->show();
}

void
MainWindow::savePropertyFile() {
	if (this->activePropertiesFile == "") {
		this->savePropertyFileAs();
	}
	else {
		saveToActivePropertiesFile();
	}
}

void
MainWindow::savePropertyFileAs() {
	sfd->setOperationMode(KFileWidget::Saving);
	connect(
		sfd->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(setActivePropertyFileAndSave())
	);
}

void
MainWindow::downloadFinished(KJob* job) {
	if (job->error()) {
		KMessageBox::error(this, job->errorString());
		activeModelFile.clear();
		return;
	}

	KIO::StoredTransferJob* storedJob = (KIO::StoredTransferJob*)job;
	if (modelActive) {
		ui.modelFile->setPlainText(QTextStream(storedJob->data(), QIODevice::ReadOnly).readAll());
	}
	else {
		ui.propertiesFile->setPlainText(QTextStream(storedJob->data(), QIODevice::ReadOnly).readAll());
	}
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
		sfd->fileWidget()
		, SIGNAL(accepted())
	);
	activeModelFile = sfd->fileWidget()->selectedFile();
	saveToActiveModelFile();
}

void
MainWindow::setActivePropertyFileAndSave() {
	disconnect(
		sfd->fileWidget()
		, SIGNAL(accepted())
	);
	activePropertiesFile = sfd->fileWidget()->selectedFile();
	saveToActivePropertiesFile();
}

void
MainWindow::onClose() {
	if (unsavedChangesModel) {
		bool shouldSave = KMessageBox::questionYesNo(0
			, i18n("You have unsaved changes to your model file! Would you like to save it now?")
		) == KMessageBox::Yes;
		if (shouldSave) {
			saveModelFile();
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
	std::string modFile = this->activeModelFile.toStdString();
	std::string propFile = this->activePropertiesFile.toStdString();
	core::Options::model_file = modFile;
	core::Options::properties_file = propFile;
	Stamina s; // TODO: create constructor for stamina::Stamina class without struct args*
	s.run();
}

void
MainWindow::handleTabChange() {
	int tabIndex = ui.mainTabs->currentIndex();
	modelActive = tabIndex != 1;
	StaminaMessages::info("Model active is " + std::to_string(modelActive));
}

} // namespace gui
} // namespace stamina
