#include "ExplorationThread.h"

#include <lock_guard>

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename RewardModelType, typename ValueType>
ExplorationThread<StateType, RewardModelType, ValueType>::ExplorationThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType>> * parent
	, uint8_t threadIndex
	, ControlThread & controlThread
) : BaseThread(parent)
	, threadIndex(threadIndex)
	, controlThread(controlThread)
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
ExplorationThread<StateType, RewardModelType, ValueType>::isFinished() {
	return finished;
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

}

} // namespace threads
} // namespace builder
} // namespace stamina
