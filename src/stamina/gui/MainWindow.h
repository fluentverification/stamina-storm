/**
 * STAMINA - the [ST]ochasic [A]pproximate [M]odel-checker for [IN]finite-state [A]nalysis
 * Copyright (C) 2023 Fluent Verification, Utah State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 *
 **/

#ifndef STAMINA_GUI_MAIN_WINDOW_H
#define STAMINA_GUI_MAIN_WINDOW_H

#include <KXmlGuiWindow>
#include <kfilecustomdialog.h>
#include <QCloseEvent>
#include <QCompleter>
#include <QProgressBar>
#include <QFuture>

#include <ui_MainWindow.h>

#include "About.h"
#include "Preferences.h"
#include "PropertyWizard.h"
#include "FindReplace.h"

#include "addons/CodeEditor.h"

#include <stamina/Stamina.h>

namespace stamina {
	namespace gui {
		const uint8_t NUMBER_RECENT_FILES = 5;
		class Preferences;

		class MainWindow : public KXmlGuiWindow {
			Q_OBJECT

		public:
			MainWindow(QWidget * parent = 0);
			void setActiveModelFileName(QString modelFileName);
			void setActivePropFileName(QString propFileName);

			void setStyleSheet(QString sheet);
		private:
			void saveToActiveModelFile();
			void saveToActivePropertiesFile();
			void setup();
			void setupActions();
			// Setup actions for each menu
			void setupFileActions();
			void setupEditActions();
			void setupViewActions();
			void setupModelActions();
			void setupCheckActions();
			void setupHelpActions();
			// Setup button slots
			void setupButtons();
			// Populates various tables
			void populateLabelTable();
			void populateResultsTable();
			void populateModelInformationTree(std::shared_ptr<storm::prism::Program> program);
			void initializeModel();
			void populateTruncatedStates();
			void populateRecentFiles();

		private:
			Stamina * s;
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
			KFileCustomDialog * exportDialog;
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
			// Actions to open recent files. Each element is a pair
			// containing the filename as a string and the action which opens that file
			std::vector<std::pair<QString, QAction *>> recentFiles;

			int modZoom = 0;
			int propZoom = 0;
			// If the model has been changed and should be rebuilt
			bool mustRebuildModel = false;
			bool init = true;


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
			void setModifiedModel(bool modifiedModel);
			void setModifiedProperties(bool modifiedProperties);
			void setActiveModelFileAndSave();
			void setActivePropertyFileAndSave();
			void closeEvent(QCloseEvent *event);
			void handleClose(); // Separate from closeEvent so it can be called without event param
			void showPropertyWizard();
			void checkModelAndProperties();
			void handleTabChange();
			// Method that gets a model from a filename
			QAbstractItemModel * modelFromFile(const QString & fileName, QCompleter * completer);
			void exportCSV();
			void saveRecentFiles();
		};
	} // namespace gui
} // namespace stamina
#endif // STAMINA_GUI_MAIN_WINDOW_H
