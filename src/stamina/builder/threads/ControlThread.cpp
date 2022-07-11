#include "ControlThread.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename RewardModelType, typename ValueType>
ControlThread<StateType, RewardModelType, ValueType>::ControlThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t numberExplorationThreads
) : BaseThread<StateType, RewardModelType, ValueType>(parent)
	, numberExplorationThreads(numberExplorationThreads)
{
	// Intentionally left empty
}

template <typename StateType, typename RewardModelType, typename ValueType>
uint8_t
ControlThread<StateType, RewardModelType, ValueType>::requestOwnership(CompressedState & state, uint8_t threadIndex) {
	std::lock_guard<std::shared_mutex> lock(ownershipMutex);
}

template <typename StateType, typename RewardModelType, typename ValueType>
uint8_t
ControlThread<StateType, RewardModelType, ValueType>::whoOwns(CompressedState & state) {

}

template <typename StateType, typename RewardModelType, typename ValueType>
void
ControlThread<StateType, RewardModelType, ValueType>::requestInsertTransition(
	uint8_t thread
	, StateType from
	, StateType to
	, double rate
) {

}

template <typename StateType, typename RewardModelType, typename ValueType>
void
ControlThread<StateType, RewardModelType, ValueType>::mainLoop() {
	uint8_t numberFinishedThreads;
	while (numberFinishedThreads < numberExplorationThreads) {
		for (auto q : dequeues) {
			while (!q.empty()) {
				// TODO: flush to transitionMatrixBuilder
			}
			// If thread is finished
		}
	}
}

} // namespace threads
} // namespace builder
} // namespace stamina
