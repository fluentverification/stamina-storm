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
				CompressedState & state
				, StateType index
				, double deltaPi

			) : state(state)
				, index(index)
				, deltaPi(deltaPi)
			{}
			CompressedState & state;
			StateType index;
			double deltaPi;
		};
		namespace threads {

			template <typename StateType>
			struct StaminaStateIndexAndThread {
				StaminaStateIndexAndThread(
					CompressedState & state
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
				CompressedState & state;
				StateType index;
				uint8_t threadIndex;
			};

		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_STATETYPES_H
