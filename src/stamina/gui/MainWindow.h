#ifndef STAMINA_MAIN_WINDOW_H
#define STAMINA_MAIN_WINDOW_H

#include <KXmlGuiWindow>
#include <kfilecustomdialog.h>

#include "ui/ui_MainWindow.h"

#include "About.h"
#include "Preferences.h"
#include "PropertyWizard.h"

#include "addons/CodeEditor.h"

namespace stamina {
	namespace gui {

		class MainWindow : public KXmlGuiWindow {
			Q_OBJECT

		public:
			MainWindow(QWidget * parent = 0);

		private:
			void saveToActiveModelFile();
			void saveToActivePropertiesFile();
			void setupActions();
			bool unsavedChangesModel;
			bool unsavedChangesProperty;
			// UI
			Ui::MainWindow ui;
			// Showable dialogs
			About * about;
			Preferences * prefs;
			PropertyWizard * propWizard;
			KFileCustomDialog * fd;
			QString activeModelFile;
			QString activePropertiesFile;
			QString baseWindowTitle;
			bool modelWasBuilt;

		private slots:
			void showPreferences();
			void openModelFile();
			void openFromAcceptedPath();
			void saveModelFile();
			void saveModelFileAs();
			void savePropertyFile();
			void savePropertyFileAs();
			void downloadFinished(KJob * job);
			void showAbout();
			void setModifiedModel();
			void setModifiedProperties();
			void setActiveModelFileAndSave();
			void setActivePropertyFileAndSave();
			void onClose();
			void showPropertyWizard();
			void checkModelAndProperties();
		};
	} // namespace gui
} // namespace stamina
#endif // STAMINA_MAIN_WINDOW_H
