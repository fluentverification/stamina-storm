#ifndef STAMINA_MAIN_WINDOW_H
#define STAMINA_MAIN_WINDOW_H

#include <KXmlGuiWindow>
#include <kfilecustomdialog.h>

#include "ui/ui_MainWindow.h"

#include "About.h"
#include "Preferences.h"

#include "addons/CodeEditor.h"

namespace stamina {
	namespace gui {

		class MainWindow : public KXmlGuiWindow {
			Q_OBJECT

		public:
			MainWindow(QWidget * parent = 0);

		private:
			void setupActions();
			void saveToActiveModelFile();
			void saveToActivePropertiesFile();
			bool unsavedChangesModel;
			bool unsavedChangesProperty;
			// UI
			Ui::MainWindow ui;
			// Showable dialogs
			About * about;
			Preferences * prefs;
			KFileCustomDialog * fd;
			QString activeModelFile;
			QString activePropertiesFile;

		private slots:
			void showPreferences();
			void openModelFile();
			void openFromAcceptedPath();
			void saveModelFile();
			void saveModelFileAs();
			void downloadFinished(KJob * job);
			void showAbout();
			void setModifiedModel();
			void setModifiedProperties();
		};
	} // namespace gui
} // namespace stamina
#endif // STAMINA_MAIN_WINDOW_H
