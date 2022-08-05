#include "BaseThread.h"

#include <thread>

namespace stamina {
namespace builder {
namespace threads {

template <typename ValueType, typename RewardModelType, typename StateType>
BaseThread<ValueType, RewardModelType, StateType>::BaseThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
) : parent(parent)
	, finished(false)
	, threadLoop(nullptr)
	, hold(true)
{
	// Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
const StaminaModelBuilder<ValueType, RewardModelType, StateType> *
BaseThread<ValueType, RewardModelType, StateType>::getParent() {
	return parent;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
BaseThread<ValueType, RewardModelType, StateType>::startThread() {
	finished = false;
	std::thread me(
		&BaseThread::mainLoop
		, this
	);
	me.join();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
BaseThread<ValueType, RewardModelType, StateType>::terminate() {
	finished = true;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void 
BaseThread<ValueType, RewardModelType, StateType>::setHold(bool hold) {
	this->hold = hold;
}

template <typename ValueType, typename RewardModelType, typename StateType>
bool 
BaseThread<ValueType, RewardModelType, StateType>::isHolding() {
	return hold;
}

template class BaseThread<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace threads
} // namespace builder
} // namespace stamina
