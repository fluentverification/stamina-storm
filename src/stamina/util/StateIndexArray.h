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

#ifndef STAMINA_UTIL_STATEINDEXARRAY_H
#define STAMINA_UTIL_STATEINDEXARRAY_H

#include <boost/pool/object_pool.hpp>

#include <vector>
#include <cstdint>
#include <memory>

/**
 * Because state indecies are generally assigned in order, it is more efficient to
 * allocate a linear array which guarantees a O(1) lookup time and has an (at worst)
 * O(n / blockSize) insert.
 *
 * Blocksizes are assumed to be large
 * */
namespace stamina {
	// Pool pointer is not within the util namespace
	/* template <typename T, typename P = boost::object_pool<T>>
	struct poolDeleter {

	private:
		P & pool;
	}; */
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
			ProbabilityStateType * get(StateType index) const;
			/**
			 * Puts a ProbabilityState in at index and if needed, expands the array to accomodate
			 *
			 * @param probabilityState The state to emplace
			 * */
			void put(StateType index, ProbabilityStateType * probabilityState);
			/**
			 * Gets a vector of all of the terminal states in the stateIndexArray
			 *
			 * @return A vector of all perimeter states
			 * */
			std::vector<StateType> getPerimeterStates();
			std::vector<ProbabilityStateType *> getPerimeterStatesAsProbStates();
			/**
			 * Gets the actual number of terminal states in the map
			 * */
			uint32_t getNumberTerminal();
		protected:
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
			std::vector<std::shared_ptr<ProbabilityStateType *>> stateArray;
		};
	}
}

#endif // STAMINA_UTIL_STATEINDEXARRAY_H
