#include "StateIndexArray.h"

namespace stamina {
	namespace util {

		template <typename StateType, typename ProbabilityStateType>
		StateIndexArray<StateType, ProbabilityStateType>::StateIndexArray(uint8_t blockSizeExponent)
		: blockSize(2 << blockSizeExponent)
			, numElements(0)
		{
			std::shared_ptr<ProbabilityStateType> * subArray = new std::shared_ptr<ProbabilityStateType>[blockSize];
			stateArray.insert(subArray);
		}

		template <typename StateType, typename ProbabilityStateType>
		StateIndexArray<StateType, ProbabilityStateType>::~StateIndexArray() {
			this->clear();
		}

		template <typename StateType, typename ProbabilityStateType>
		void
		StateIndexArray<StateType, ProbabilityStateType>::clear() {
			for (auto subArray : stateArray) {
				delete subArray;
			}
		}

		template <typename StateType, typename ProbabilityStateType>
		void
		StateIndexArray<StateType, ProbabilityStateType>::reserve(uint32_t numToReserve) {
			this->clear();
			uint32_t actualNumToReserve = sizeToActualSize(numToReserve);
			uint16_t arrayIndex = actualNumToReserve / blockSize;
			uint32_t subArrayIndex = actualNumToReserve % blockSize;
			for (int i = 0; i < arrayIndex; i++) {
				stateArray.insert(new std::shared_ptr<ProbabilityStateType>[blockSize]);
			}
		}

		template <typename StateType, typename ProbabilityStateType>
		std::shared_ptr<ProbabilityStateType>
		StateIndexArray<StateType, ProbabilityStateType>::get(StateType index) {
			uint16_t arrayIndex = index / blockSize;
			uint32_t subArrayIndex = index % blockSize;
			return stateArray[arrayIndex][subArrayIndex];
		}

		template <typename StateType, typename ProbabilityStateType>
		void
		StateIndexArray<StateType, ProbabilityStateType>::put(
			StateType index
			, std::shared_ptr<ProbabilityStateType> probabilityState
		) {
			numElements++;
			uint16_t arrayIndex = index / blockSize;
			uint32_t subArrayIndex = index % blockSize;
			while (arrayIndex > stateArray.size() - 1) {
				stateArray.insert(new std::shared_ptr<ProbabilityStateType>[blockSize]);
			}
			stateArray[arrayIndex][subArrayIndex] = probabilityState;
		}

		template <typename StateType, typename ProbabilityStateType>
		uint32_t
		StateIndexArray<StateType, ProbabilityStateType>::sizeToActualSize(uint32_t size) {
			while (size % blockSize != 0) {
				size++;
			}
		}

	}
}
