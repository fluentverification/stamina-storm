#ifndef STAMINA_CORE_STAMINAMODELCHECKER_H
#define STAMINA_CORE_STAMINAMODELCHECKER_H

#include "Options.h"
#include "builder/StaminaModelBuilder.h"
#include "builder/StaminaIterativeModelBuilder.h"
// #include "builder/StaminaPriorityModelBuilder.h"
#include "builder/StaminaReExploringModelBuilder.h"

#include <sstream>
#include <string>
#include <unordered_set>

#include "__storm_needed_for_checker.h"

namespace stamina {
	namespace core {
		using namespace stamina::builder;
		const uint32_t absorbingStateIndex = 0;
		class StaminaModelChecker {
		public:
			/**
			* Constructor for StaminaModelChecker
			*
			* @param modulesFile Pointer to the Modules file
			* @param propertiesVector Pointer to the Properties vector
			* */
			StaminaModelChecker(
				std::shared_ptr<storm::prism::Program> modulesFile = nullptr
				, std::shared_ptr<std::vector<storm::jani::Property>> propertiesVector = nullptr
			);
			/**
			* Standard destructor
			* */
			~StaminaModelChecker();
			/**
			* Initializes the StaminaModelChecker class
			*
			* @param propertiesVector A vector to the list of JANI properties
			* */
			void initialize(
				std::shared_ptr<storm::prism::Program> modulesFile = nullptr
				, std::shared_ptr<std::vector<storm::jani::Property>> propertiesVector = nullptr
			);
			/**
			* Model checks a specific property
			*
			* @param prop Property to check
			* @param modulesFile The modules file to work with
			* @return A pointer to the result of the model checking
			* */
			std::unique_ptr<storm::modelchecker::CheckResult> modelCheckProperty(
				storm::jani::Property propMin
				, storm::jani::Property propMax
				, storm::prism::Program const& modulesFile
			);
		private:
			/**
			* Result subclass (no private members since is a private subclass)
			* */
			class Result {
			public:
				/**
				* Constructor. Defaults to 0.0 / ""
				* */
				Result() {
					result = 0.0;
					explanation = "";
				}
				/**
				* Gets string representation
				* */
				operator std::string() const {
					std::stringstream str;
					str << result << " (" << explanation << ")";
					return str.str();
				}
				/**
				* Gets string representation
				* */
				friend std::ostream & operator<<(std::ostream & stream, StaminaModelChecker::Result const & r) {
					stream << r.result << " (" << r.explanation << ")";
					return stream;
				}

				double result;
				std::string explanation;

			};
			/**
			* Whether or not to terminate model check
			*
			* @return Terminate?
			* */
			bool terminateModelCheck();
			/**
			* Writes perimeter states to a specified file.
			* */
			void writePerimeterStates(int numRefineIteration);
			/**
			* Prints all of the transition actions to a file.
			* */
			void printTransitionActions(std::string filename);
			/**
			* Writes the min and max results to a file
			*
			* @param filename The filename to append to
			* */
			void writeToOutput(std::string filename);
			/**

			*/
			void modifyState(bool isMin);
			/* Data Members */
			std::shared_ptr<StaminaModelChecker::Result> min_results;
			std::shared_ptr<StaminaModelChecker::Result> max_results;
			std::shared_ptr<StaminaModelBuilder<double>> builder;
			std::shared_ptr<storm::prism::Program> modulesFile;
			std::shared_ptr<std::vector<storm::jani::Property>> propertiesVector;
			storm::expressions::ExpressionManager expressionManager;
			storm::models::sparse::StateLabeling * labeling;
			std::string preUntilLabel;
		};
	} // namespace core
} // namespace stamina

#endif // STAMINA_CORE_STAMINAMODELCHECKER_H
