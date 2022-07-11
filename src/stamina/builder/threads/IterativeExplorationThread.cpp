#include "IterativeExplorationThread.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename RewardModelType, typename ValueType>
IterativeExplorationThread<StateType, RewardModelType, ValueType>::IterativeExplorationThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t threadIndex
	, ControlThread<ValueType, RewardModelType, StateType> & controlThread
) : ExplorationThread(parent, threadIndex, controlThread)
{
	// Intentionally left empty
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
IterativeExplorationThread<StateType, RewardModelType, ValueType>::exploreStates() {
	if (!crossExplorationQueue.empty() && !crossExplorationQueueMutex.locked()) {
		std::lock_guard<std::shared_mutex> lockGuard(crossExplorationQueueMutex);
		auto stateDeltaPiPair = crossExplorationQueue.top();
		crossExplorationQueue.pop();
		StateType s = stateDeltaPiPair.first;
		double deltaPi = stateDeltaPiPair.second;
		// Update the estimated reachability of s
		auto probabilityState = parent->getStateMap().get(s);
		probabilityState.pi += deltaPi;
		exploreState(s);
	}
	else if (!mainExplorationQueue.empty()) {
		auto s = mainExplorationQueue.top();
		mainExplorationQueue.pop();
		exploreState(s);
	}
	else {
		this->idling = true;
	}
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
IterativeExplorationThread<StateType, RewardModelType, ValueType>::exploreState(StateAndProbability & stateProbability) {

}


} // namespace threads
} // namespace builder
} // namespace stamina
