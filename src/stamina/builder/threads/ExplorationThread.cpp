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

#include "ExplorationThread.h"
#include "builder/StaminaModelBuilder.h"

#include "core/StaminaMessages.h"

#include <mutex>

namespace stamina {
namespace builder {
namespace threads {

template <typename ValueType, typename RewardModelType, typename StateType>
ExplorationThread<ValueType, RewardModelType, StateType>::ExplorationThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t threadIndex
	, ControlThread<ValueType, RewardModelType, StateType> & controlThread
	, uint32_t stateSize
	, util::StateIndexArray<StateType, ProbabilityState<StateType>> * stateMap
	, std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, std::function<StateType (CompressedState const&)> stateToIdCallback
) : BaseThread<ValueType, RewardModelType, StateType>(parent)
	, threadIndex(threadIndex)
	, controlThread(controlThread)
	, stateStorage(parent->getStateStorage())
	, stateMap(stateMap)
	, generator(generator)
	, stateToIdCallback(stateToIdCallback)
	, xLock(crossExplorationQueueMutex, std::defer_lock)
{
	// Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
uint8_t
ExplorationThread<ValueType, RewardModelType, StateType>::getIndex() {
	return threadIndex;
}

template <typename ValueType, typename RewardModelType, typename StateType>
uint32_t
ExplorationThread<ValueType, RewardModelType, StateType>::getNumberOfOwnedStates() {
	return numberOfOwnedStates;
}

template <typename ValueType, typename RewardModelType, typename StateType>
bool
ExplorationThread<ValueType, RewardModelType, StateType>::isIdling() {
	return idling;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ExplorationThread<ValueType, RewardModelType, StateType>::setIsCtmc(bool isCtmc) {
	this->isCtmc = isCtmc;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ExplorationThread<ValueType, RewardModelType, StateType>::requestCrossExploration(
	CompressedState const & state
	, double deltaPi
) {
	// Lock the mutex since multiple threads will be calling this function
	// auto lock = std::unique_lock<std::shared_mutex>(crossExplorationQueueMutex);
	std::lock_guard<std::shared_mutex> guard(crossExplorationQueueMutex);
	// crossExplorationQueueMutex.lock();
	crossExplorationQueue.emplace_back(
		std::make_pair(state, deltaPi)
	);
	// crossExplorationQueueMutex.unlock();
	// lock.unlock();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ExplorationThread<ValueType, RewardModelType, StateType>::mainLoop() {
	STAMINA_DEBUG_MESSAGE("Starting exploration thread: " << this->threadIndex);
	idling = false;
	while (!this->finished || this->hold) {
		// STAMINA_DEBUG_MESSAGE("Finished is " << this->finished << " and hold is " << this->hold);
		// Explore the states in the exploration queue
		exploreStates();
	}
}

template class ExplorationThread<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace threads
} // namespace builder
} // namespace stamina
