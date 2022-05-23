#include "StateIndexArray.h"

using namespace stamina;
using namespace stamina::util;

template <typename Statetype, typename ProbabilityStateType>
StateIndexArray<StateType, ProbabilityStateType>::StateIndexArray(uint8_t blockSizeExponent)
: blockSize(2 << blockSizeExponent)
{
	std::shared_ptr<ProbabilityStateType> * subArray = new std::shared_ptr<ProbabilityStateType>(blockSize);
	stateArray.insert(subArray);
}

template <typename Statetype, typename ProbabilityStateType>
StateIndexArray<StateType, ProbabilityStateType>::~StateIndexArray() {
	this->clear();
}

template <typename Statetype, typename ProbabilityStateType>
void
StateIndexArray<StateType, ProbabilityStateType>::clear() {
	for (auto subArray : stateArray) {
		delete subArray;
	}
}

template <typename Statetype, typename ProbabilityStateType>
void
reserve(uint32_t numToReserve) {
	this->clear();
	uint32_t actualNumToReserve = sizeToActualSize(numToReserve);

}

std::shared_ptr<ProbabilityStateType>
StateIndexArray<StateType, ProbabilityStateType>::get(StateType index) {
	uint16_t arrayIndex = index / stateArray;
	uint32_t subArrayIndex = index % blockSize;
	return stateArray[arrayIndex][subArrayIndex];
}

template <typename Statetype, typename ProbabilityStateType>
void
StateIndexArray<StateType, ProbabilityStateType>::put(
	StateType index
	, std::shared_ptr<ProbabilityStateType> probabilityState
) {

}

