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

#ifndef STAMINA_CORE_STAMINAMODELCHECKER_H
#define STAMINA_CORE_STAMINAMODELCHECKER_H

#include "Options.h"
#include "builder/StaminaModelBuilder.h"
#include "builder/StaminaIterativeModelBuilder.h"
#include "builder/StaminaThreadedIterativeModelBuilder.h"
#include "builder/StaminaPriorityModelBuilder.h"
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
			class ResultTableRow {
			public:
				ResultTableRow(
					double pMin
					, double pMax
					, std::string property
				) : pMin(pMin)
				, pMax(pMax)
				, property(property)
				{ /* Intentionally left empty */ }
				double pMin;
				double pMax;
				std::string property;
			};
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
			* @param propMin Minimum variant of the property to check
			* @param propMax Maximum variant of the property to check
			* @propOriginal The original version of the property to check
			* @param modulesFile The modules file to work with
			* @param formulasVector The vector of all properties (optional). If the model is to check multiple
			* properties then this is required as labeling must be created which matches each one.
			* @return A pointer to the result of the model checking
			* */
			std::unique_ptr<storm::modelchecker::CheckResult> modelCheckProperty(
				storm::jani::Property propMin
				, storm::jani::Property propMax
				, storm::jani::Property propOriginal
				, storm::prism::Program const& modulesFile
				// We want to be conscious of all of the properties so that we can build associated labels
				// if this is passed in null
				, std::vector<std::shared_ptr< storm::logic::Formula const>> const & formulasVector
				, bool forceRebuildModel=false
			);
			void checkFromBuiltModel(
				storm::jani::Property propMin
				, storm::jani::Property propMax
				, storm::jani::Property propOriginal
			);
			/**
			 * Gets a list of the labels and the associated counts of states
			 *
			 * @return A (pointer to a) vector of pairs with a string and int, the string representing the name of the label and the
			 * int being the count of states with that label.
			 * */
			std::shared_ptr<std::vector<std::pair<std::string, uint64_t>>> getLabelsAndCount();
			std::vector<ResultTableRow> & getResultTable() { return this->resultTable; }
			uint64_t getStateCount() { return builder->getStateCount(); }
			uint64_t getTransitionCount() { return builder->getTransitionCount(); }
			std::vector<ProbabilityState<uint32_t> *> getPerimeterStates() { return builder->getPerimeterStatesAsProbabilityStates(); }
			std::shared_ptr<storm::models::sparse::Ctmc<double, storm::models::sparse::StandardRewardModel<double>>> getModel() { return model; };
			// const CompressedState & getState(uint32_t index) { return builder->getStateFromIndex(index); }
		private:
			/**
			* Result subclass (no private members since is a private subclass)
			* */
			class Result {
			public:
				/**
				* Constructor. Defaults to 0.0 / ""
				* */
				Result()
					: result(0.0)
					, explanation("")
				{
					// Intentionally left empty
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
			void printTransitionActions(std::string const & filename);
			/**
			* Writes the min and max results to a file
			*
			* @param filename The filename to append to
			* */
			void writeToOutput(std::string filename);
			/**
			* Takes a state and
			*/
			void modifyState(bool isMin);

			/* Data Members */
			std::shared_ptr<StaminaModelChecker::Result> min_results;
			std::shared_ptr<StaminaModelChecker::Result> max_results;
			// The model
			std::shared_ptr<storm::models::sparse::Ctmc<double, storm::models::sparse::StandardRewardModel<double>>> model;
			std::shared_ptr<CtmcModelChecker> checker;
			// The results for all of the properties we check
			std::vector<ResultTableRow> resultTable;
			std::shared_ptr<StaminaModelBuilder<double>> builder;
			std::shared_ptr<storm::prism::Program> modulesFile;
			std::shared_ptr<std::vector<storm::jani::Property>> propertiesVector;
			storm::expressions::ExpressionManager expressionManager;
			storm::models::sparse::StateLabeling * labeling;
			std::string preUntilLabel;
			bool modelBuilt;
		};
	} // namespace core
} // namespace stamina

#endif // STAMINA_CORE_STAMINAMODELCHECKER_H
