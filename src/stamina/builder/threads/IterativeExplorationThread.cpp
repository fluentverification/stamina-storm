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

}

template <typename StateType, typename RewardModelType, typename ValueType>
void
IterativeExplorationThread<StateType, RewardModelType, ValueType>::exploreState(StateAndProbability & stateProbability) {

}


} // namespace threads
} // namespace builder
} // namespace stamina
