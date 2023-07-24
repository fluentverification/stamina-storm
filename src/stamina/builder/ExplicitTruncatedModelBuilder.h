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

#ifndef STAMINA_BUILDER_EXPLICITTRUNCATEDMODELBUILDER_H
#define STAMINA_BUILDER_EXPLICITTRUNCATEDMODELBUILDER_H

/**
 * A copy of storm::builder::ExplicitModelBuilder with a max number of states to explore
 * for debugging purposes
 * */

#include <boost/container/flat_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <cstdint>
#include <deque>
#include <memory>
#include <utility>
#include <vector>
#include "storm/models/sparse/StandardRewardModel.h"

#include "storm/builder/ExplicitModelBuilder.h"

#include "storm/logic/Formulas.h"
#include "storm/models/sparse/ChoiceLabeling.h"
#include "storm/models/sparse/Model.h"
#include "storm/models/sparse/StateLabeling.h"
#include "storm/settings/SettingsManager.h"
#include "storm/storage/BitVectorHashMap.h"
#include "storm/storage/SparseMatrix.h"
#include "storm/storage/expressions/ExpressionEvaluator.h"
#include "storm/storage/prism/Program.h"
#include "storm/storage/sparse/ModelComponents.h"
#include "storm/storage/sparse/StateStorage.h"

#include "storm/utility/prism.h"

#include "storm/builder/ExplorationOrder.h"

#include "storm/generator/CompressedState.h"
#include "storm/generator/NextStateGenerator.h"
#include "storm/generator/VariableInformation.h"

namespace stamina {

namespace builder {

using namespace storm::utility::prism;
using namespace storm::generator;
using namespace storm::builder;
using namespace storm;

template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
class ExplicitTruncatedModelBuilder {
public:
	struct Options {
		/*!
		* Creates an object representing the default building options.
		*/
		Options();

		// The order in which to explore the model.
		ExplorationOrder explorationOrder;
	};

	/*!
	* Creates an explicit model builder that uses the provided generator.
	*
	* @param generator The generator to use.
	*/
	ExplicitTruncatedModelBuilder(std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> const& generator, Options const& options = Options());

	/*!
	* Creates an explicit model builder for the given PRISM program.
	*
	* @param program The program for which to build the model.
	*/
	ExplicitTruncatedModelBuilder(storm::prism::Program const& program,
						storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions(),
						Options const& builderOptions = Options());

	/*!
	* Creates an explicit model builder for the given JANI model.
	*
	* @param model The JANI model for which to build the model.
	*/
	ExplicitTruncatedModelBuilder(storm::jani::Model const& model,
						storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions(),
						Options const& builderOptions = Options());

	/*!
	* Convert the program given at construction time to an abstract model. The type of the model is the one
	* specified in the program. The given reward model name selects the rewards that the model will contain.
	*
	* @return The explicit model that was given by the probabilistic program as well as additional
	*		 information (if requested).
	*/
	std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> build();

	/*!
	* Export a wrapper that contains (a copy of) the internal information that maps states to ids.
	* This wrapper can be helpful to find states in later stages.
	* @return
	*/
	ExplicitStateLookup<StateType> exportExplicitStateLookup() const;

private:
	/*!
	* Retrieves the state id of the given state. If the state has not been encountered yet, it will be added to
	* the lists of all states with a new id. If the state was already known, the object that is pointed to by
	* the given state pointer is deleted and the old state id is returned. Note that the pointer should not be
	* used after invoking this method.
	*
	* @param state A pointer to a state for which to retrieve the index. This must not be used after the call.
	* @return A pair indicating whether the state was already discovered before and the state id of the state.
	*/
	StateType getOrAddStateIndex(CompressedState const& state);

	/*!
	* Builds the transition matrix and the transition reward matrix based for the given program.
	*
	* @param transitionMatrixBuilder The builder of the transition matrix.
	* @param rewardModelBuilders The builders for the selected reward models.
	* @param stateAndChoiceInformationBuilder The builder for the requested information of the individual states and choices
	*/
	void buildMatrices(storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder,
					std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders,
					StateAndChoiceInformationBuilder& stateAndChoiceInformationBuilder);

	/*!
	* Explores the state space of the given program and returns the components of the model as a result.
	*
	* @return A structure containing the components of the resulting model.
	*/
	storm::storage::sparse::ModelComponents<ValueType, RewardModelType> buildModelComponents();

	/*!
	* Builds the state labeling for the given program.
	*
	* @return The state labeling of the given program.
	*/
	storm::models::sparse::StateLabeling buildStateLabeling();

	/// The generator to use for the building process.
	std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> generator;

	/// The options to be used for the building process.
	Options options;

	/// Internal information about the states that were explored.
	storm::storage::sparse::StateStorage<StateType> stateStorage;

	/// A set of states that still need to be explored.
	std::deque<std::pair<CompressedState, StateType>> statesToExplore;

	/// An optional mapping from state indices to the row groups in which they actually reside. This needs to be
	/// built in case the exploration order is not BFS.
	boost::optional<std::vector<uint_fast64_t>> stateRemapping;
	uint64_t numberOfExploredStates;
};

}
}  // namespace stamina

#endif /* STAMINA_BUILDER_EXPLICITTRUNCATEDMODELBUILDER_H */
