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

#include "StateMemoryPool.h"
#include "builder/StaminaModelBuilder.h"
#include "core/StaminaMessages.h"

/**
 * Implementation for StaminaMemoryPool methods
 *
 * This file created by Josh Jeppson on May 27, 2022
 * */

namespace stamina {
	namespace util {
		template <typename T>
		StateMemoryPool<T>::StateMemoryPool(uint8_t blockSize)
			: blockSize(2 << blockSize)
			, usedThisBlock(0)
		{
			// Create the first block
			blocks.push_back(new T[this->blockSize]);
		}

		template <typename T>
		StateMemoryPool<T>::~StateMemoryPool() {
			// TODO: We get an error in freeAll()
			// There would be a memory leak but this lasts until the end of program execution
			// freeAll();
		}

		template <typename T>
		void
		StateMemoryPool<T>::freeAll() {
			for (T * block : blocks) {
				if (block) { delete block; }
			}
		}

		template <typename T>
		T *
		StateMemoryPool<T>::allocate(uint32_t number) {
			if (number > blockSize - usedThisBlock) {
				// TODO: be smarter about this
				StaminaMessages::error("Cannot allocate array of this size on current block!");
				return nullptr;
			}
			// We have enough memory in the current block to offer
			T * addressToReturn = blocks[blocks.size() - 1] + usedThisBlock;
			usedThisBlock += number;
			// Create new block if necessary
			if (usedThisBlock == blockSize) {
				blocks.push_back(new T[blockSize]);
				usedThisBlock = 0;
			}
			return addressToReturn;
		}

		// Forward declare
		template class StateMemoryPool<
			builder::ProbabilityState<uint32_t>
		>;
	}
}
