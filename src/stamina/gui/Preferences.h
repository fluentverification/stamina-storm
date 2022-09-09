#ifndef STAMINA_PREFERENCES_H
#define STAMINA_PREFERENCES_H

#include <KXmlGuiWindow>
#include <QDialog>

#include "ui/ui_Preferences.h"

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
				inline static bool modelFileFromExtension;
				// Save modified CSL properties file
				inline static bool saveModifiedCSL;
				// Truncate model
				inline static bool truncateModel;
				// Attempt to generate counterExamples
				inline static bool generateCounterexamples;
				// Create refined properties
				inline static bool createRefinedProperties;
				// Include verbose log messages
				inline static bool verboseLog;
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
				// Truncation method. TODO: import enum from stamina::core::Options
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
			void show();
			void hide();
		private:
			void setupActions();
			// Data members
			Ui::Preferences ui;
		};
	}
}

#endif // STAMINA_PREFERENCES_H
