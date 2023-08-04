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

#ifndef STAMINA_PREFERENCES_H
#define STAMINA_PREFERENCES_H

#include <KXmlGuiWindow>
#include <QDialog>
#include <cstdint>
#include <string>

#include "stamina/core/Options.h"

#include <ui_Preferences.h>
#include <ui_MainWindow.h>

namespace stamina {
	namespace gui {

		// A class that keeps track of preferences
		class PrefInfo {
		public:
			static void setDefaults();
			static const std::string prefPath;
			// "General" tab
			struct General {
				static void setDefaults();
				// Auto-detect model file type from extension
				// inline static bool modelFileFromExtension; // Depricated
				// Save modified CSL properties file
				// inline static bool saveModifiedCSL; // Depricated
				// Truncate model
				inline static bool truncateModel;
				// Attempt to generate counterExamples
				inline static bool generateCounterexamples;
				// Create refined properties
				inline static bool createRefinedProperties;
				// Include verbose log messages
				inline static bool verboseLog;
				// The Font
				inline static QFont editorFont;
				// Tab size
				inline static uint8_t tabSize;
				// Use tabs or spaces
				inline static bool useTabs;
			};
			// "Model Building" tab
			struct ModelBuilding {
				static void setDefaults();
				/**
				 * NOTE: These are the values SENT to stamina::core::Options. They do NOT
				 * mirror those values.
				 * */
				inline static double kappa;
				// Kappa reduction factor
				inline static double rKappa;
				inline static double window;
				// Early termination using property
				inline static bool earlyTerminationProperty;
				// Maximum iterations
				inline static uint16_t maxIterations;
				// Maximum approx iterations
				inline static uint8_t maxApproxIterations;
				// Export transitions
				inline static bool exportTransitions;
				// File to export transitions to
				inline static std::string transitionsFile;
				// Export perimeter states
				inline static bool exportPerimeterStates;
				// File to export perimeter states to
				inline static std::string perimeterStatesFile;
				// Truncation method.
				inline static uint8_t truncationMethod;
				// Number of threads
				inline static uint8_t threads;
			};
			// Model checking tab
			struct ModelChecking {
				static void setDefaults();
				inline static bool useSylvan;
				inline static bool useCudd;
				inline static std::string cuddLimit;
				inline static bool stormVerbose;
			};
			// CounterExample Generation tab
			struct CounterExampleGeneration {
				static void setDefaults();
				// Maximum distance to explore
				inline static uint32_t maxDistance;
				inline static bool useScaffolding;
				inline static uint32_t numberScaffolds;
				inline static uint32_t numberCounterexamples;
			};
			// Property refinement tab
			struct PropertyRefinement {
				static void setDefaults();
			};
		};

		class Preferences : public QDialog {
			Q_OBJECT
		public:
			Preferences(QWidget * parent = 0);
			void show(int tabIndex = 0);
			void accept() override;
			Ui::MainWindow * getMainWindow() { return window; }
			void setMainWindow(Ui::MainWindow * window) { this->window = window; }
			/**
			 * Sets the "options" values in Stamina::core::Options
			 * from the preferences selected on this window.
			 * */
			void setOptionsFromPreferences();
			/**
			 * Sets preference info that updates the user interface
			 * */
			void setUIFromPreferences();
			void getPreferencesFromUI();
		private slots:
			void replaceAllIndentation();
		private:
			void setupActions();
			// Data members
			Ui::Preferences ui;
			Ui::MainWindow * window;
		};
	}
}

#endif // STAMINA_PREFERENCES_H
