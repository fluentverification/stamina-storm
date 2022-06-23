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
	// setupGUI(Default, "mainWindow.ui");
}

void
MainWindow::saveToActiveModelFile() {

}

void
MainWindow::saveToActivePropertiesFile() {

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
	fd->show();
// 	QString openFileName = KFileDialog::getOpenFileName(this, i18n("Open model file"));
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
	saveToActiveModelFile();
}

} // namespace gui
} // namespace stamina
