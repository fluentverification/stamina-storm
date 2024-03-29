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

#include "StateIndexArray.h"

#include "builder/StaminaModelBuilder.h"

namespace stamina {
namespace util {

template <typename StateType, typename ProbabilityStateType>
StateIndexArray<StateType, ProbabilityStateType>::StateIndexArray(uint8_t blockSizeExponent)
: blockSize(2 << blockSizeExponent)
	, numElements(0)
{
	std::shared_ptr<ProbabilityStateType *> subArray(
		new ProbabilityStateType *[blockSize]
		, std::default_delete<ProbabilityStateType *[]>()
	);
	for (int i = 0; i < blockSize; i++) {
		subArray.get()[i] = nullptr;
	}
	stateArray.push_back(subArray);
}

template <typename StateType, typename ProbabilityStateType>
StateIndexArray<StateType, ProbabilityStateType>::~StateIndexArray() {
	this->clear();
}

template <typename StateType, typename ProbabilityStateType>
void
StateIndexArray<StateType, ProbabilityStateType>::clear() {
	this->stateArray.clear();
}

template <typename StateType, typename ProbabilityStateType>
void
StateIndexArray<StateType, ProbabilityStateType>::reserve(uint32_t numToReserve) {
	this->clear();
	uint32_t actualNumToReserve = sizeToActualSize(numToReserve);
	uint16_t arrayIndex = actualNumToReserve / blockSize;
	for (int i = 0; i < arrayIndex; i++) {
		std::shared_ptr<ProbabilityStateType *> subArray(
			new ProbabilityStateType *[blockSize]
			, std::default_delete<ProbabilityStateType *[]>()
		);
		for (int j = 0; j < blockSize; j++) {
			subArray.get()[j] = nullptr;
		}
		stateArray.push_back(subArray);
	}
}

template <typename StateType, typename ProbabilityStateType>
ProbabilityStateType *
StateIndexArray<StateType, ProbabilityStateType>::get(StateType index) const {
	uint16_t arrayIndex = index / blockSize;
	if (arrayIndex > stateArray.size() - 1) {
		return nullptr;
	}
	uint32_t subArrayIndex = index % blockSize;
	return stateArray[arrayIndex].get()[subArrayIndex];
}

template <typename StateType, typename ProbabilityStateType>
void
StateIndexArray<StateType, ProbabilityStateType>::put(
	StateType index
	, ProbabilityStateType * probabilityState
) {
	numElements++;
	uint16_t arrayIndex = index / blockSize;
	uint32_t subArrayIndex = index % blockSize;
	while (arrayIndex > stateArray.size() - 1) {
		std::shared_ptr<ProbabilityStateType *> subArray(
			new ProbabilityStateType *[blockSize]
			, std::default_delete<ProbabilityStateType *[]>()
		);
		for (int j = 0; j < blockSize; j++) {
			subArray.get()[j] = nullptr;
		}
		stateArray.push_back(subArray);
	}
	stateArray[arrayIndex].get()[subArrayIndex] = probabilityState;
}

template <typename StateType, typename ProbabilityStateType>
uint32_t
StateIndexArray<StateType, ProbabilityStateType>::getNumberTerminal() {
	return getPerimeterStates().size();
}

template <typename StateType, typename ProbabilityStateType>
std::vector<StateType>
StateIndexArray<StateType, ProbabilityStateType>::getPerimeterStates() {
	std::vector<StateType> perimeterStates;
	for (auto subArray : stateArray) {
		for (int i = 0; i < blockSize; i++) {
			if (subArray.get()[i] != nullptr && subArray.get()[i]->isTerminal()) {
				perimeterStates.push_back(subArray.get()[i]->index);
			}
		}
	}
	return perimeterStates;
}

template <typename StateType, typename ProbabilityStateType>
std::vector<ProbabilityStateType *>
StateIndexArray<StateType, ProbabilityStateType>::getPerimeterStatesAsProbStates() {
	std::vector<ProbabilityStateType *> perimeterStates;
	for (auto subArray : stateArray) {
		for (int i = 0; i < blockSize; i++) {
			if (subArray.get()[i] != nullptr && subArray.get()[i]->isTerminal()) {
				perimeterStates.push_back(subArray.get()[i]);
			}
		}
	}
	return perimeterStates;
}

template <typename StateType, typename ProbabilityStateType>
uint32_t
StateIndexArray<StateType, ProbabilityStateType>::sizeToActualSize(uint32_t size) {
	while (size % blockSize != 0) {
		size++;
	}
	return size;
}
// Forward-declare
template class StateIndexArray<
	uint32_t
	, builder::ProbabilityState<uint32_t>
>;

} // namespace util
} // namespace stamina
