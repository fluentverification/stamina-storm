#include "BaseThread.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename ValueType>
BaseThread<StateType, ValueType>::BaseThread(
	StaminaModelBuilder<ValueType, StateType=StateType> * parent
) : parent(parent)
{
	// Intentionally left empty
}

template <typename StateType, typename ValueType>
void
BaseThread<StateType, ValueType>::startThread() {

}

template <typename StateType, typename ValueType>
StaminaModelBuilder<ValueType, StateType=StateType> *
BaseThread<StateType, ValueType>::getParent() {
	return parent;
}


} // namespace threads
} // namespace builder
} // namespace stamina
