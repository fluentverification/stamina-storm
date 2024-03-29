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
 * Exploration threads can explore a state space with other threads, and will only explore threads
 * that it owns.
 *
 * Created by Josh Jeppson on Jul 8, 2022
 * */

#ifndef STAMINA_BUILDER_THREADS_EXPLORATIONTHREAD_H
#define STAMINA_BUILDER_THREADS_EXPLORATIONTHREAD_H

#include "BaseThread.h"

#include "util/StateIndexArray.h"
#include "builder/ProbabilityState.h"
#include "builder/StateAndTransitions.h"

namespace stamina {
	namespace builder {
		namespace threads {
			template <typename ValueType, typename RewardModelType, typename StateType>
			class ExplorationThread : public BaseThread<ValueType, RewardModelType, StateType> {
			public:
				typedef StateAndProbability<StateType> StateProbability;

				typedef StaminaStateIndexAndThread<StateType> StateIndexAndThread;
				/**
				* Constructor. Invokes super's constructor and stores the
				* thread index which cannot change for the life of the thread
				*
				* @param parent The model builder who owns this thread
				* @param threadIndex The index of this thread
				* @param stateSize The size of the states
				* @param stateMap Access to the parent class's stateMap
				* */
				ExplorationThread(
					StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
					, uint8_t threadIndex
					, ControlThread<ValueType, RewardModelType, StateType> & controlThread
					, uint32_t stateSize
					, util::StateIndexArray<StateType, ProbabilityState<StateType>> * stateMap
					, std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
					, std::function<StateType (CompressedState const&)> stateToIdCallback
				);

				/*
				 * No copy constructor is allowed since this thread contains a std::mutex
				 * which cannot be copied.
				 * */

				uint8_t getIndex();
				uint32_t getNumberOfOwnedStates();
				bool isIdling();
				void setIsCtmc(bool isCtmc);
				/**
				* A function called by other threads to request cross exploration of
				* states already explored but encountered by another thread.
				*
				* @param state The state to cross explore.
				* @param deltaPi The difference in reachability to add to that
				* state and push forward to its successors.
				* */
				void requestCrossExploration(CompressedState const & state, double deltaPi);
				void requestCrossExploration(StateType stateIndex, double deltaPi);
				/**
				* Does state exploration or idles until worker thread asks to kill it.
				* */
				virtual void mainLoop() override; // doExploration
			protected:
				virtual void exploreStates() = 0;
				virtual void exploreState(StateProbability & stateProbability) = 0;
				virtual StateType enqueueSuccessors(CompressedState const & state) = 0; // stateToIdCallback
				// Weak priority on crossExplorationQueue (superseded by mutex lock)
				std::shared_mutex crossExplorationQueueMutex;
				// The lock that locks our mutex
				std::unique_lock<std::shared_mutex> xLock;
				std::deque<std::pair<CompressedState, double>> crossExplorationQueue;
				std::deque<std::pair<ProbabilityState<StateType> *, CompressedState const &>> mainExplorationQueue;
				uint32_t numberOfOwnedStates;
				bool idling;
				ControlThread<ValueType, RewardModelType, StateType> & controlThread;
				util::StateIndexArray<StateType, ProbabilityState<StateType>> * stateMap;
				storm::storage::sparse::StateStorage<StateType> & stateStorage;
				std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator;
				std::deque<StateProbability> statesTerminatedLastIteration;
				std::function<StateType (CompressedState const&)> stateToIdCallback;
				// The states we should request cross exploration from
				std::deque<StateIndexAndThread> statesToRequestCrossExploration;
			protected:
				const uint8_t threadIndex;
				bool isCtmc;
			};
		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_THREADS_EXPLORATIONTHREAD_H
