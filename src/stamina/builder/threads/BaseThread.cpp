/**
 * STAMINA - the [ST]ochasic [A]pproximate [M]odel-checker for [IN]finite-state [A]nalysis
 * Copyright (C) 2023 Fluent Verification, Utah State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 *
 **/
#include "BaseThread.h"
#include "core/StaminaMessages.h"

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
	this->threadLoop = new std::thread(
		&BaseThread::mainLoop
		, this
	);
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
BaseThread<ValueType, RewardModelType, StateType>::startThreadAndWait() {
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
	hold = false;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
BaseThread<ValueType, RewardModelType, StateType>::join() {
	if (this->threadLoop->joinable()) {
		this->threadLoop->join();
	}
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
