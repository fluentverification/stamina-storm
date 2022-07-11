#include "BaseThread.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename RewardModelType, typename ValueType>
BaseThread<StateType, RewardModelType, ValueType>::BaseThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
) : parent(parent)
{
	// Intentionally left empty
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
BaseThread<StateType, RewardModelType, ValueType>::startThread() {
	finished = false;
}

template <typename StateType, typename RewardModelType, typename ValueType>
StaminaModelBuilder<ValueType, RewardModelType, StateType> *
BaseThread<StateType, RewardModelType, ValueType>::getParent() {
	return parent;
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
BaseThread<StateType, RewardModelType, ValueType>::terminate() {
	finished = true;
}

} // namespace threads
} // namespace builder
} // namespace stamina
