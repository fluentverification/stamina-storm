#include "ControlThread.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename ValueType>
ControlThread<StateType, ValueType>::ControlThread(
	StaminaModelBuilder<ValueType, StateType=StateType> * parent
	, uint8_t numberExplorationThreads
) : BaseThread<StateType, ValueType>(parent)
	, numberExplorationThreads(numberExplorationThreads)
{
	// Intentionally left empty
}

template <typename StateType, typename ValueType>
uint8_t
ControlThread<StateType, ValueType>::requestOwnership(CompressedState & state, uint8_t threadIndex) {

}

template <typename StateType, typename ValueType>
uint8_t
ControlThread<StateType, ValueType>::whoOwns(CompressedState & state) {

}

template <typename StateType, typename ValueType>
void
ControlThread<StateType, ValueType>::requestInsertTransition(
	uint8_t thread
	, StateType from
	, StateType to
	, double rate
) {

}

template <typename StateType, typename ValueType>
void
ControlThread<StateType, ValueType>::mainLoop() {

}

} // namespace threads
} // namespace builder
} // namespace stamina
