#ifndef STATEINDEXARRAY_H
#define STATEINDEXARRAY_H

#include <vector>
#include <cstdint>
#include <memory>
#include <thread>
#include <mutex>

/**
 * Because state indecies are generally assigned in order, it is more efficient to
 * allocate a linear array which guarantees a O(1) lookup time and has an (at worst)
 * O(n / blockSize) insert.
 *
 * Blocksizes are assumed to be large
 * */
namespace stamina {
	namespace util {
		template <typename StateType, typename ProbabilityStateType>
		class StateIndexArray {
		public:
			inline static std::mutex dataMutex;
			StateIndexArray(uint8_t blockSizeExponent = 15); // 2 ^ 15
			~StateIndexArray();
			/**
			 * Clears all elements from the StateIndexArray and frees all memory
			 * */
			void clear();
			/**
			 * Reserves a certain number of states into the StateIndexArray
			 *
			 * @param numToReserve The minimum number to reserve
			 * */
			void reserve(uint32_t numToReserve);
			/**
			 * Gets a pointer to the ProbabilityState stored in the array.
			 * If the index does not exist, or is greater than the amount stored,
			 * returns a nullptr
			 *
			 * @param index The index to find
			 * @return The ProbabilityState or nullptr if does not exist
			 * */
			std::shared_ptr<ProbabilityStateType> get(StateType index);
			/**
			 * Puts a ProbabilityState in at index and if needed, expands the array to accomodate
			 *
			 * @param probabilityState The state to emplace
			 * */
			void put(StateType index, std::shared_ptr<ProbabilityStateType> probabilityState);
			/**
			 * Gets a vector of all of the terminal states in the stateIndexArray
			 *
			 * @return A vector of all perimeter states
			 * */
			std::vector<StateType> getPerimeterStates();
		protected:
			/**
			 * Puts a ProbabilityState in at index and if needed, expands the array to accomodate
			 *
			 * @param probabilityState The state to emplace
			 * */
			void putHelper(StateType index, std::shared_ptr<ProbabilityStateType> probabilityState);

			/**
			 * Gets a size that is a multiple of blockSize
			 *
			 * @param size The size you want
			 * @return The next size of a multiple of blockSize
			 * */
			uint32_t sizeToActualSize(uint32_t size);
		private:
			uint32_t numElements;
			uint32_t blockSize;
			std::vector<std::shared_ptr<std::shared_ptr<ProbabilityStateType>>> stateArray;
			std::thread insertWorker;
		};
	}
}

#endif // STATEINDEXARRAY_H
