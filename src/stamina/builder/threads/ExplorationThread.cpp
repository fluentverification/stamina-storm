#include "ExplorationThread.h"

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
ExplorationThread<ValueType, RewardModelType, StateType>::requestCrossExploration(CompressedState & state, double deltaPi) {
	// Lock the mutex since multiple threads will be calling this function
	std::lock_guard<std::shared_mutex> guard(crossExplorationQueueMutex);
	crossExplorationQueue.emplace_back(
		std::make_pair(state, deltaPi)
	);
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ExplorationThread<ValueType, RewardModelType, StateType>::mainLoop() {
	idling = false;
	while (!this->finished || this->hold) {
		// Explore the states in the exploration queue
		exploreStates();
	}
}

template class ExplorationThread<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace threads
} // namespace builder
} // namespace stamina
