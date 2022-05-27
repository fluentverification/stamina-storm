#include "StateIndexArray.h"

#include "../StaminaModelBuilder.h"

namespace stamina {
	namespace util {

		template <typename StateType, typename ProbabilityStateType>
		StateIndexArray<StateType, ProbabilityStateType>::StateIndexArray(uint8_t blockSizeExponent)
		: blockSize(2 << blockSizeExponent)
			, numElements(0)
		{
			readWriteMutex.lock();
			std::shared_ptr<std::shared_ptr<ProbabilityStateType>> subArray(
				new std::shared_ptr<ProbabilityStateType>[blockSize]
				, std::default_delete<std::shared_ptr<ProbabilityStateType>[]>()
			);
			for (int i = 0; i < blockSize; i++) {
				subArray.get()[i] = nullptr;
			}
			stateArray.push_back(subArray);
			readWriteMutex.unlock();
		}

		template <typename StateType, typename ProbabilityStateType>
		StateIndexArray<StateType, ProbabilityStateType>::~StateIndexArray() {
			this->clear();
		}

		template <typename StateType, typename ProbabilityStateType>
		void
		StateIndexArray<StateType, ProbabilityStateType>::clear() {
			this->stateArray.clear();
			numElements = 0;
		}

		template <typename StateType, typename ProbabilityStateType>
		void
		StateIndexArray<StateType, ProbabilityStateType>::reserve(uint32_t numToReserve) {
			readWriteMutex.lock();
			this->clear();
			uint32_t actualNumToReserve = sizeToActualSize(numToReserve);
			uint16_t arrayIndex = actualNumToReserve / blockSize;
			uint32_t subArrayIndex = actualNumToReserve % blockSize;
			for (int i = 0; i < arrayIndex; i++) {
				std::shared_ptr<std::shared_ptr<ProbabilityStateType>> subArray(
					new std::shared_ptr<ProbabilityStateType>[blockSize]
					, std::default_delete<std::shared_ptr<ProbabilityStateType>[]>()
				);
				for (int j = 0; j < blockSize; j++) {
					subArray.get()[j] = nullptr;
				}
				stateArray.push_back(subArray);
			}
			readWriteMutex.unlock();
		}

		template <typename StateType, typename ProbabilityStateType>
		std::shared_ptr<ProbabilityStateType>
		StateIndexArray<StateType, ProbabilityStateType>::get(StateType index) {
			readWriteMutex.lock();
			uint16_t arrayIndex = index / blockSize;
			if (arrayIndex > stateArray.size() - 1) {
				readWriteMutex.unlock();
				return nullptr;
			}
			uint32_t subArrayIndex = index % blockSize;
			auto value = stateArray[arrayIndex].get()[subArrayIndex];
			readWriteMutex.unlock();
			return value;
		}

		template <typename StateType, typename ProbabilityStateType>
		void
		StateIndexArray<StateType, ProbabilityStateType>::put(
			StateType index
			, std::shared_ptr<ProbabilityStateType> probabilityState
		) {
			insertWorker = std::thread(
				&StateIndexArray::putHelper
				, this
				, index
				, probabilityState
			);
			// Allow the worker to insert independently of the main thread
			insertWorker.detach();
		}
		template <typename StateType, typename ProbabilityStateType>
		void
		StateIndexArray<StateType, ProbabilityStateType>::putHelper(
			StateType index
			, std::shared_ptr<ProbabilityStateType> probabilityState
		) {
			readWriteMutex.lock();
			numElements++;
			uint16_t arrayIndex = index / blockSize;
			uint32_t subArrayIndex = index % blockSize;
			while (arrayIndex > stateArray.size() - 1) {
				std::shared_ptr<std::shared_ptr<ProbabilityStateType>> subArray(
					new std::shared_ptr<ProbabilityStateType>[blockSize]
					, std::default_delete<std::shared_ptr<ProbabilityStateType>[]>()
				);
				for (int j = 0; j < blockSize; j++) {
					subArray.get()[j] = nullptr;
				}
				stateArray.push_back(subArray);
			}
			stateArray[arrayIndex].get()[subArrayIndex] = probabilityState;
			readWriteMutex.unlock();
		}

		template <typename StateType, typename ProbabilityStateType>
		std::vector<StateType>
		StateIndexArray<StateType, ProbabilityStateType>::getPerimeterStates() {
			readWriteMutex.lock();
			std::vector<StateType> perimeterStates;
			for (auto subArray : stateArray) {
				for (int i = 0; i < blockSize; i++) {
					if (subArray.get()[i] != nullptr && subArray.get()[i]->isTerminal()) {
						perimeterStates.push_back(subArray.get()[i]->index);
					}
				}
			}
			readWriteMutex.unlock();
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
			, StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>::ProbabilityState
		>;
	}
}
