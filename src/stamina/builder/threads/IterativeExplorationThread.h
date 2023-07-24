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
 * An implementation of the ExplorationThread abstract class which uses the STAMINA iterative (2.0/2.5)
 * algorithm.
 *
 * Created by Josh Jeppson
 * */

#ifndef STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H
#define STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H

#include "ExplorationThread.h"

namespace stamina {
	namespace builder {
		namespace threads {
			template <typename ValueType, typename RewardModelType, typename StateType>
			class IterativeExplorationThread : public ExplorationThread<ValueType, RewardModelType, StateType> {
			public:
				typedef StateAndProbability<StateType> StateProbability;
				typedef StaminaStateIndexAndThread<StateType> StateIndexAndThread;
				/**
				* Constructor. Invokes super's constructor.
				*
				* @param parent The model builder who owns this thread
				* @param threadIndex The index of this thread
				* */
				IterativeExplorationThread(
					StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
					, uint8_t threadIndex
					, ControlThread<ValueType, RewardModelType, StateType> & controlThread
					, uint32_t stateSize
					, util::StateIndexArray<StateType, ProbabilityState<StateType>> * stateMap
					, std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
					// , std::function<StateType (CompressedState const&)> stateToIdCallback
				);

				virtual StateType enqueueSuccessors(CompressedState const & state) override;
			protected:
				virtual void exploreStates() override;
				virtual void exploreState(StateProbability & stateProbability) override;
			private:
				uint32_t numberTerminal;
				bool isCtmc;
				bool currentStateHasZeroReachability;
			};
		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H
