#include "IterativeExplorationThread.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename RewardModelType, typename ValueType>
IterativeExplorationThread<StateType, RewardModelType, ValueType>::IterativeExplorationThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t threadIndex
	, ControlThread<StateType, RewardModelType, ValueType> & controlThread
) : ExplorationThread<StateType, RewardModelType, ValueType>(parent, threadIndex, controlThread)
{
	// Intentionally left empty
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
IterativeExplorationThread<StateType, RewardModelType, ValueType>::exploreStates() {
	if (!this->crossExplorationQueue.empty() && !this->crossExplorationQueueMutex.locked()) {
		std::lock_guard<std::shared_mutex> lockGuard(this->crossExplorationQueueMutex);
		auto stateDeltaPiPair = this->crossExplorationQueue.top();
		this->crossExplorationQueue.pop();
		StateType s = stateDeltaPiPair.first;
		double deltaPi = stateDeltaPiPair.second;
		// Update the estimated reachability of s
		auto probabilityState = this->parent->getStateMap().get(s);
		probabilityState.pi += deltaPi;
		exploreState(s);
	}
	else if (!this->mainExplorationQueue.empty()) {
		auto s = this->mainExplorationQueue.top();
		this->mainExplorationQueue.pop();
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
