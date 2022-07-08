#include "ExplorationThread.h"

#include <lock_guard>

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename ValueType>
ExplorationThread<StateType, ValueType>::ExplorationThread(
	StaminaModelBuilder<ValueType, StateType=StateType>> * parent
	, uint8_t threadIndex
) : BaseThread(parent)
	, threadIndex(threadIndex)
{

}
template <typename StateType, typename ValueType>
uint8_t
ExplorationThread<StateType, ValueType>::getIndex() {
	return threadIndex;
}

template <typename StateType, typename ValueType>
uint32_t
ExplorationThread<StateType, ValueType>::getNumberOfOwnedStates() {

}

template <typename StateType, typename ValueType>
bool
ExplorationThread<StateType, ValueType>::isFinished() {
	return finished;
}

template <typename StateType, typename ValueType>
void
ExplorationThread<StateType, ValueType>::requestCrossExploration(CompressedState & state, double deltaPi) {
	// Lock the mutex since multiple threads will be calling this function
	std::lock_guard<std::shared_mutex> guard(crossExplorationQueueMutex);
	crossExplorationQueue.emplace_back(
		std::make_pair(state, deltaPi)
	);
}

template <typename StateType, typename ValueType>
void
ExplorationThread<StateType, ValueType>::mainLoop() {

}

} // namespace threads
} // namespace builder
} // namespace stamina
