#ifndef STATEINDEXARRAY_H
#define STATEINDEXARRAY_H

#include <vector>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

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
			/**
			 * Joins the worker thread (i.e., waits for it to finish
			 * */
			void joinWorker();
		protected:
			/**
			 * Gets a size that is a multiple of blockSize
			 *
			 * @param size The size you want
			 * @return The next size of a multiple of blockSize
			 * */
			uint32_t sizeToActualSize(uint32_t size);
			/**
			 * Helper for put()--A function that can be passed into a detached thread
			 * */
			void putHelper(StateType index, std::shared_ptr<ProbabilityStateType> probabilityState);
		private:
			uint32_t numElements;
			uint32_t blockSize;
			std::vector<std::shared_ptr<std::shared_ptr<ProbabilityStateType>>> stateArray;
			std::shared_mutex mutex;
			std::shared_ptr<std::thread> worker;
		};
	}
}

#endif // STATEINDEXARRAY_H
