#ifndef STAMINA_MAIN_WINDOW_H
#define STAMINA_MAIN_WINDOW_H

#include <KXmlGuiWindow>
#include <kfilecustomdialog.h>
#include <QCloseEvent>
#include <QCompleter>
#include <QProgressBar>
#include <QFuture>

#include "ui/ui_MainWindow.h"

#include "About.h"
#include "Preferences.h"
#include "PropertyWizard.h"
#include "FindReplace.h"

#include "addons/CodeEditor.h"

#include <stamina/Stamina.h>

namespace stamina {
	namespace gui {

		class MainWindow : public KXmlGuiWindow {
			Q_OBJECT

		public:
			MainWindow(QWidget * parent = 0);

		private:
			void saveToActiveModelFile();
			void saveToActivePropertiesFile();
			void setup();
			void setupActions();
			void populateLabelTable();
			void populateResultsTable();
			void populateModelInformationTree(std::shared_ptr<storm::prism::Program> program);
			void initializeModel();

		private:
			Stamina s;
			bool unsavedChangesModel;
			bool unsavedChangesProperty;
			bool modelActive; // If not model is active, then property is active
			// UI
			Ui::MainWindow ui;
			// Showable dialogs
			About * about;
			Preferences * prefs;
			PropertyWizard * propWizard;
			// Widgets whose methods we must call
			FindReplace * modelFindReplace;
			FindReplace * propFindReplace;
			// Completers
			QCompleter * modCompleter;
			QCompleter * propCompleter;
			// Perhaps there is a better way to do this, but there
			// appear to be threadsafety issues using the same dialog
			// Save file dialog
			KFileCustomDialog * sfdm;
			KFileCustomDialog * sfdp;
			// Open file dialog
			KFileCustomDialog * ofdm;
			KFileCustomDialog * ofdp;
			// The progress bar
			QProgressBar * progress;
			QPushButton * killButton;
			// Active files
			QString activeModelFile;
			QString activePropertiesFile;
			QString baseWindowTitle;
			bool modelWasBuilt;
			// The running STAMINA job (so it can be killed)
			QFuture<void> staminaJob;
			// Hackey way to get the file dialog to stay open
			bool stayOpen;
			int modZoom = 0;
			int propZoom = 0;


		private slots:
			void showPreferences();
			void openModelFile();
			void openModelFromAcceptedPath();
			void openPropertyFromAcceptedPath();
			void saveModelFile();
			void saveModelFileAs();
			void openPropertyFile();
			void savePropertyFile();
			void savePropertyFileAs();
			void downloadFinishedModel(KJob * job);
			void downloadFinishedProperty(KJob * job);
			void showAbout();
			void setModifiedModel();
			void setModifiedProperties();
			void setActiveModelFileAndSave();
			void setActivePropertyFileAndSave();
			void closeEvent(QCloseEvent *event);
			void handleClose(); // Separate from closeEvent so it can be called without event param
			void showPropertyWizard();
			void checkModelAndProperties();
			void handleTabChange();
			// Method that gets a model from a filename
			QAbstractItemModel * modelFromFile(const QString & fileName, QCompleter * completer);
		};
	} // namespace gui
} // namespace stamina
#endif // STAMINA_MAIN_WINDOW_H
