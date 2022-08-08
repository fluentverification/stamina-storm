#ifndef STAMINA_BUILDER_STATETYPES_H
#define STAMINA_BUILDER_STATETYPES_H

namespace stamina {
	namespace builder {

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


	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_STATETYPES_H
