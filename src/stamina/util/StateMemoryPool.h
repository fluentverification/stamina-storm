#ifndef STATEMEMORYPOOL_H
#define STATEMEMORYPOOL_H

// #include <deque>
// #include <ordered_set>
#include <cstdint>

/**
 * Stamina Memory Pool allocator
 *
 * Because STAMINA/STORM allocates thousands or millions of states, and in C++ each call to `new` invokes
 * a system-call, dynamic allocation of memory for stamina::StaminaModelBuilder::ProbabilityStates are
 * very slow. This allocates a pool of larger blocks to reduce the number of system-calls (since each
 * call to malloc() -- invoked by new -- is a system call to ask for more memory)
 *
 * This memory pool makes a few assumptions to increase performance:
 *	1. All states are assumed to be in memory from allocation-time until the program. The STAMINA memory
 *	pool allows for dynamic deletion, but defragmentation must be explicitly invoked using the defrag()
 *	method and is not automatically called.
 *	2. For now, defrag() has not been implemented, requiring
 *
 * Future work:
 * 	1. Offload excess memory to a swapfile which is deleted on StateMemoryPool destruction
 *
 *
 * This file created by Josh Jeppson on May 27, 2022
 * */
namespace stamina {
	namespace util {
		template <typename T>
		class StateMemoryPool {
		public:
			/**
			 * Constructor. Creates the first block of the pool
			 *
			 * @param blockSize Exponent on 2 of
			 * */
			StateMemoryPool(uint8_t blockSize = 12); // 2 ^ 12
			/**
			 * Destructor. Frees all of the memory allocated by the memory pool
			 * */
			~StaminaMemoryPool();
			/**
			 * Allocates a T value and returns a pointer to it
			 * */
			T * allocate(uint32_t number = 1);
			/**
			 * Clears all memory
			 * */
			void freeAll();
			// Methods that will be implemented when defragmentation is added
			// void free(T * address); // knows the number used
			// void defrag();
		private:
			const uint32_t blockSize;
			uint32_t usedThisBlock;
			std::vector<T *> blocks;
			// Will be used when defragmentation is implemented
			// std::deque<std::pair<T*, uint32_t>> deletedBlocks; // Blocks which have been deleted
			// std::ordered_set<std::pair<T*, uint32_t>> nonSingleBlocks; // Blocks of arrays
		};
	}
}

#endif // STATEMEMORYPOOL_H
