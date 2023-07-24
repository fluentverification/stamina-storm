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

#ifndef STAMINA_BUILDER_STATETYPES_H
#define STAMINA_BUILDER_STATETYPES_H

#include "__storm_needed_for_builder.h"

namespace stamina {
	namespace builder {
		typedef storm::storage::BitVector CompressedState;

		template <typename StateType>
		/**
		 * A basic struct for out of order transitions to insert into the transition matrix.
		 * This is faster than using the remapping and std::sort in the STORM API
		 * */
		class StaminaTransitionInfo {
		public:
			StaminaTransitionInfo(StateType from, StateType to, double transition) :
				from(from), to(to), transition(transition) { /* Intentionally left empty */ }
			StateType from;
			StateType to;
			double transition;
		};

		template <typename StateType>
		struct StaminaTransitionInfoComparison {
			bool operator() (
				const StaminaTransitionInfo<StateType> * first
				, const StaminaTransitionInfo<StateType> * second
			) const {
				return first->to > second->to;
			}
		};

		template <typename StateType>
		struct StaminaStateAndThreadIndex {
			StaminaStateAndThreadIndex(StateType state, uint8_t thread) : state(state), thread(thread) {
				// Intentionally left empty
			}
			StateType state; // State Index
			uint8_t thread; // Thread index
			// Cast to StateType
			operator StateType() const {
				return state;
			}
		};

		template <typename StateType>
		class StateAndProbability {
		public:
			StateAndProbability(
				CompressedState const & state
				, StateType index
				, double deltaPi = 0
			) : state(state)
				, index(index)
				, deltaPi(deltaPi)
			{}
			// Copy constructor
			StateAndProbability(const StateAndProbability & other)
				: state(other.state)
				, index(other.index)
				, deltaPi(other.deltaPi)
			{}
			CompressedState const & state;
			StateType index;
			double deltaPi;
		};
		namespace threads {

			template <typename StateType>
			struct StaminaStateIndexAndThread {
				StaminaStateIndexAndThread(
					CompressedState const & state
					, StateType index
					, uint8_t threadIndex
				) : state(state)
					, index(index)
					, threadIndex(threadIndex)
				{}
				StaminaStateIndexAndThread(const StaminaStateIndexAndThread & other)
				 : state(other.state)
					, index(other.index)
					, threadIndex(other.threadIndex)

				{}
				CompressedState const & state;
				StateType index;
				uint8_t threadIndex;
			};

		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_STATETYPES_H
