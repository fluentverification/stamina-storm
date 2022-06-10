#ifndef STAMINA_ITERATIVE_MODEL_BUILDER_H
#define STAMINA_ITERATIVE_MODEL_BUILDER_H

/**
 * The model builder class which implements the STAMINA 2.5 algorithm (STAMINA 2.0/2.1 with dynamic programming)
 *
 * Created by Josh Jeppson on Jun 9, 2021
 * */

#include "StaminaModelBuilder.h"

namespace stamina {
	namespace builder {
		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		class StaminaIterativeModelBuilder : protected StaminaModelBuilder<ValueType, RewardModelType, StateType> {
		public:
			/**
			* Constructs a StaminaIterativeModelBuilder with a given storm::generator::PrismNextStateGenerator. Invokes super's constructor
			*
			* @param generator The generator we are going to use.
			* */
			StaminaIterativeModelBuilder(
				std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
				, storm::prism::Program const& modulesFile
				, storm::generator::NextStateGeneratorOptions const & options
			);
			/**
			* Constructs a StaminaIterativeModelBuilder with a PRISM program and generatorOptions. Invokes super's constructor
			*
			* @param program The PRISM program we are going to use to build the model with.
			* @param generatorOptions Options for the storm::generator::PrismNextStateGenerator we are going to use.
			* */
			StaminaIterativeModelBuilder(
				storm::prism::Program const& program
				, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
			);
			/**
			* Builds transition matrix of truncated state space for the given program.
			*
			* @param transitionMatrixBuilder The builder of the transition matrix.
			* @param rewardModelBuilders The builders for the selected reward models.
			* @param choiceInformationBuilder The builder for the requested information of the choices
			* @param markovianChoices is set to a bit vector storing whether a choice is Markovian (is only set if the model type requires this information).
			* @param stateValuationsBuilder if not boost::none, we insert valuations for the corresponding states
			* */
			void buildMatrices(
				storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
				, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
				, StateAndChoiceInformationBuilder& choiceInformationBuilder
				, boost::optional<storm::storage::BitVector>& markovianChoices
				, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
			) override;
			/**
			* Gets the state ID of a current state, or adds it to the internal state storage. Performs state exploration
			* and state space truncation from that state.
			*
			* @param state Pointer to the state we are looking it
			* @return A pair with the state id and whether or not it was already discovered
			* */
			StateType getOrAddStateIndex(CompressedState const& state) override;
			/**
			* Explores state space and truncates the model
			*
			* @return The components of the truncated model
			* */
			storm::storage::sparse::ModelComponents<ValueType, RewardModelType> buildModelComponents() override;
		private:
			/**
			 * Flushes the states terminated into statesToExplore
			 * */
			void flushStatesTerminated();
			/**
			 * Connects all states which are terminal
			 * */
			void connectAllTerminalStatesToAbsorbing();
			// Dynamic programming improvement: we keep an ordered set of the states terminated
			// during the previous iteration (in an order that prevents needing to use a remapping
			// vector for state indecies.
			std::deque<std::shared_ptr<ProbabilityState>> statesTerminatedLastIteration;
			uint64_t numberOfExploredStates;
			uint64_t numberOfExploredStatesSinceLastMessage;
		};
	}
}
#endif // STAMINA_ITERATIVE_MODEL_BUILDER_H
