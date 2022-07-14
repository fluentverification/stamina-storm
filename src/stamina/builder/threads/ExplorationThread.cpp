#include "ExplorationThread.h"

#include <mutex>

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename RewardModelType, typename ValueType>
ExplorationThread<StateType, RewardModelType, ValueType>::ExplorationThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t threadIndex
	, ControlThread<StateType, RewardModelType, ValueType> & controlThread
	, uint32_t stateSize
	, util::StateIndexArray<StateType, ProbabilityState<StateType>> * stateMap
	, std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
) : BaseThread<StateType, RewardModelType, ValueType>(parent)
	, threadIndex(threadIndex)
	, controlThread(controlThread)
	, stateStorage(parent->getStateStorage())
	, stateMap(stateMap)
	, generator(generator)
{
	// Intentionally left empty
}

template <typename StateType, typename RewardModelType, typename ValueType>
uint8_t
ExplorationThread<StateType, RewardModelType, ValueType>::getIndex() {
	return threadIndex;
}

template <typename StateType, typename RewardModelType, typename ValueType>
uint32_t
ExplorationThread<StateType, RewardModelType, ValueType>::getNumberOfOwnedStates() {
	return numberOfOwnedStates;
}

template <typename StateType, typename RewardModelType, typename ValueType>
bool
ExplorationThread<StateType, RewardModelType, ValueType>::isIdling() {
	return idling;
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
ExplorationThread<StateType, RewardModelType, ValueType>::requestCrossExploration(CompressedState & state, double deltaPi) {
	// Lock the mutex since multiple threads will be calling this function
	std::lock_guard<std::shared_mutex> guard(crossExplorationQueueMutex);
	crossExplorationQueue.emplace_back(
		std::make_pair(state, deltaPi)
	);
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
ExplorationThread<StateType, RewardModelType, ValueType>::mainLoop() {
	idling = false;
	while (!this->finished) {
		// Explore the states in the exploration queue
		exploreStates();
	}
}

} // namespace threads
} // namespace builder
} // namespace stamina
