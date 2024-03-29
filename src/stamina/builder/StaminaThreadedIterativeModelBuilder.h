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

/**
 * There are some benefits if only using one thread to using just the StaminaModelBuilder
 * class or StaminaIterativeModelBuilder class. This class is used if and only if the threadcount
 * is greater than 1.
 *
 * Simply including threading makes it so that there is some nontrivial overhead in exploration,
 * and we want to provide the users the option without having them have to have a control thread
 * idling and waiting for the exploration thread.
 *
 * Created by Josh Jeppson on Jul 15, 2022
 * */
#ifndef STAMINA_BUILDER_STAMINATHREADEDITERATIVEMODELBUILDER_H
#define STAMINA_BUILDER_STAMINATHREADEDITERATIVEMODELBUILDER_H

#include "StaminaIterativeModelBuilder.h"

#include "builder/threads/ControlThread.h"
#include "builder/threads/IterativeExplorationThread.h"

namespace stamina {
	namespace builder {
		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		class StaminaThreadedIterativeModelBuilder : public StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType> {
		public:
			/**
			* Constructs a StaminaThreadedIterativeModelBuilder with a given storm::generator::PrismNextStateGenerator. Invokes super's constructor
			*
			* @param generator The generator we are going to use.
			* */
			StaminaThreadedIterativeModelBuilder(
				std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
				, storm::prism::Program const& modulesFile
				, storm::generator::NextStateGeneratorOptions const & options
			);
			/**
			* Constructs a StaminaThreadedIterativeModelBuilder with a PRISM program and generatorOptions. Invokes super's constructor
			*
			* @param program The PRISM program we are going to use to build the model with.
			* @param generatorOptions Options for the storm::generator::PrismNextStateGenerator we are going to use.
			* */
			StaminaThreadedIterativeModelBuilder(
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
			);
			/**
			 * Gets the state ID of a current state, or adds it to the internal state storage. Performs state exploration
			 * and state space truncation from that state. The reason this one is different
			 *
			 * @param state Pointer to the state we are looking it
			 * @return A pair with the state id and whether or not it was already discovered
			 * */
			StateType getOrAddStateIndexAndTrackTerminal(CompressedState const& state);
			std::vector<typename threads::ExplorationThread<ValueType, RewardModelType, StateType> *> const & getExplorationThreads() const override;
			/**
			 * Sets a vector of generators for the threads
			 * */
			void setGeneratorsVector(std::vector<std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>>> & generators);
		private:
			std::vector<std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>>> * generators;
			threads::ControlThread<ValueType, RewardModelType, StateType> controlThread;
			std::vector<typename threads::ExplorationThread<ValueType, RewardModelType, StateType> *> explorationThreads;
			bool controlThreadsCreated;
			/*
			 * Some explaination:
			 *     This is a fast way to keep track of the terminal states without having to use "getTerminalStates"
			 *     which can be slow and inefficient. Any new state that is marked as terminal is placed in this
			 *     deque, and every time we explore a state, we remove the front of the deque (corresponding to
			 *     that state). If we have a state which we keep terminal, it is dequed and re-enqueued.
			 *
			 *     Therefore, this will NOT store the terminal states in the order they are explored.
			 * */
			std::deque<CompressedState> fastTerminalStates;
		};
		// "Custom" deleter (which actually is not custom) to allow for polymorphic shared pointers
		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		void __delete_stamina_iterative_model_builder(StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType> * t) { delete t; }
	}
}

#endif // STAMINA_BUILDER_STAMINATHREADEDITERATIVEMODELBUILDER_H
