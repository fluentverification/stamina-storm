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
	, activeModelFile("")
	, activePropertiesFile("")
	, about(new About(this))
	, prefs(new Preferences(this))
	, unsavedChangesModel(false)
	, unsavedChangesProperty(false)
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
	}
}

void
MainWindow::showPreferences() {
	std::cout << "Showing preferences dialog" << std::endl;
	prefs->exec();
}

void
MainWindow::openModelFile() {
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
		setCaption(QString("OPEN MODEL")); // TODO: The actual filename
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
	fd->show();
	fd->fileWidget()->setFilter(QString("*.prism *.sm|PRISM Model files\n*.jani|JANI Model Files"));
	saveToActiveModelFile();
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
	setCaption(windowTitle() + " * ");
	unsavedChangesModel = true;
}

void
MainWindow::setModifiedProperties() {
	if (unsavedChangesProperty) { return; }
	setCaption(windowTitle() + " * ");
	unsavedChangesProperty = true;
}

} // namespace gui
} // namespace stamina
