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

} // namespace threads
} // namespace builder
} // namespace stamina
