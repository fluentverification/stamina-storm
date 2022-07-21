#include "BaseThread.h"

#include <thread>

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename RewardModelType, typename ValueType>
BaseThread<StateType, RewardModelType, ValueType>::BaseThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
) : parent(parent)
	, finished(false)
	, threadLoop(nullptr)
	, hold(true)
{
	// Intentionally left empty
}

template <typename StateType, typename RewardModelType, typename ValueType>
const StaminaModelBuilder<ValueType, RewardModelType, StateType> *
BaseThread<StateType, RewardModelType, ValueType>::getParent() {
	return parent;
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
BaseThread<StateType, RewardModelType, ValueType>::startThread() {
	finished = false;
	std::thread me(
		&BaseThread::mainLoop
		, this
	);
	me.join();
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
BaseThread<StateType, RewardModelType, ValueType>::terminate() {
	finished = true;
}

template <typename StateType, typename RewardModelType, typename ValueType>
void 
BaseThread<StateType, RewardModelType, ValueType>::setHold(bool hold) {
	this->hold = hold;
}

template <typename StateType, typename RewardModelType, typename ValueType>
bool 
BaseThread<StateType, RewardModelType, ValueType>::isHolding() {
	return hold;
}

template class BaseThread<uint32_t, storm::models::sparse::StandardRewardModel<double>, double>;

} // namespace threads
} // namespace builder
} // namespace stamina
