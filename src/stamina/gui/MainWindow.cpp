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

#include "MainWindow.h"

#include <iostream>

namespace stamina {
namespace gui {


MainWindow::MainWindow(QWidget *parent)
	: KXmlGuiWindow(parent)
	, fd(new KFileCustomDialog())
	, about(new About(this))
	, prefs(new Preferences(this))
	, propWizard(new PropertyWizard(this))
	, activeModelFile("")
	, activePropertiesFile("")
	, unsavedChangesModel(false)
	, unsavedChangesProperty(false)
	, baseWindowTitle("New File")
	, modelWasBuilt(false)
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

	// TODO: Connect on close to onClose()

	// setupGUI(Default, "mainWindow.ui");
}

void
MainWindow::saveToActiveModelFile() {

	if (activeModelFile != "") {
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
	std::cout << "Showing preferences dialog" << std::endl;
	prefs->exec();
}

void
MainWindow::openModelFile() {
	if (unsavedChangesModel) {
		bool shouldNotDiscard = KMessageBox::questionYesNo(0, i18n("You have unsaved changes to your model file! Would you like to save it now?")) == KMessageBox::Yes;
		if (shouldNotDiscard) {
			saveModelFile();
		}
	}
	std::cout << "Opening model file" << std::endl;
	fd->setOperationMode(KFileWidget::Opening);
	fd->fileWidget()->setFilter(QString("*.prism *.sm|PRISM Model files\n*.jani|JANI Model Files"));
	connect(
		fd->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(openFromAcceptedPath())
	);
	fd->show();
// 	QString openFileName = KFileDialog::getOpenFileName(this, i18n("Open model file"));
}

void
MainWindow::openFromAcceptedPath() {
	QString selectedFile = fd->fileWidget()->selectedFile();
	std::cout << "Opening file " << selectedFile.toStdString() << std::endl;
	disconnect(
		fd->fileWidget()
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
	std::cout << "Saving model file as" << std::endl;
	fd->setOperationMode(KFileWidget::Saving);
	connect(
		fd->fileWidget()
		, SIGNAL(accepted())
		, this
		, SLOT(setActiveModelFileAndSave())
	);
	fd->show();
	fd->fileWidget()->setFilter(QString("*.prism *.sm|PRISM Model files\n*.jani|JANI Model Files"));
// 	saveToActiveModelFile();
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
	fd->setOperationMode(KFileWidget::Saving);
	connect(
		fd->fileWidget()
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
	ui.modelFile->setPlainText(QTextStream(storedJob->data(), QIODevice::ReadOnly).readAll());
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
	// Disconnect fd just in case it sent us here
	disconnect(
		fd->fileWidget()
		, SIGNAL(accepted())
	);
	activeModelFile = fd->fileWidget()->selectedFile();
	saveToActiveModelFile();
}

void
MainWindow::setActivePropertyFileAndSave() {
	disconnect(
		fd->fileWidget()
		, SIGNAL(accepted())
	);
	activePropertiesFile = fd->fileWidget()->selectedFile();
	saveToActivePropertyFile();
}

void
MainWindow::onClose() {
	if (unsavedChangesModel) {
		bool shouldSave = KMessageBox::questionYesNo(0, i18n("You have unsaved changes to your model file! Would you like to save it now?")) == KMessageBox::Yes;
		if (shouldSave) {
			saveModelFile();
		}
	}
}

void
MainWindow::showPropertyWizard() {
	if (!modelWasBuilt) {
		bool shouldShow = KMessageBox::warningContinueCancel(0, i18n("The model has not been built yet! Variable names will not appear in the Property Wizard.")) == KMessageBox::Continue;
		if (!shouldShow) {
			return;
		}
	}
	propWizard->show();
	// TODO: connect the close() slot to appending to the text file
}

void
MainWindow::checkModelAndProperties() {
	this->saveModelFile();
}

} // namespace gui
} // namespace stamina
