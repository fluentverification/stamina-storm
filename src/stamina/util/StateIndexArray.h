#ifndef STATEINDEXARRAY_H
#define STATEINDEXARRAY_H

#include <vector>
#include <cstdint>
#include <memory>

/**
 * Because state indecies are generally assigned in order, it is more efficient to
 * allocate a linear array which guarantees a O(1) lookup time and has an (at worst)
 * O(n / blockSize) insert
 * */
namespace stamina {
	namespace util {
		template <typename StateType, typename ProbabilityStateType>
		class StateIndexArray {
		public:
			StateIndexArray(uint8_t blockSizeExponent = 15); // 2 ^ 15
			~StateIndexArray();
			void clear();
			void reserve(uint32_t numToReserve);
			std::shared_ptr<ProbabilityStateType> get(StateType index);
			void put(StateType index, std::shared_ptr<ProbabilityStateType> probabilityState);
		protected:
			uint32_t sizeToActualSize(uint32_t size);
		private:
			uint32_t numElements;
			uint32_t blockSize;
			std::vector<std::shared_ptr<ProbabilityStateType> * > stateArray;
		};
	}
}

#endif // STATEINDEXARRAY_H
