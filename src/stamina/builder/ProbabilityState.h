#ifndef STAMINA_BUILDER_PROBABILITYSTATE_H
#define STAMINA_BUILDER_PROBABILITYSTATE_H

#include "__storm_needed_for_builder.h"

namespace stamina {
	namespace builder {
		using namespace storm::builder;
		/* Class for states with probabilities */
		template <typename StateType>
		class ProbabilityState {
		public:
			StateType index;
			uint8_t iterationLastSeen;
			bool assignedInRemapping;
			bool isNew;
			bool wasPutInTerminalQueue;
			ProbabilityState(
				StateType index = 0
				, double pi = 0.0
				, bool terminal = true
				, uint8_t iterationLastSeen = 0
			) : index(index)
				, pi(pi)
				, terminal(terminal)
				, assignedInRemapping(false)
				, iterationLastSeen(iterationLastSeen)
				, isNew(true)
				, wasPutInTerminalQueue(false)
			{
				// Intentionally left empty
			}
			// Copy constructor
			ProbabilityState(const ProbabilityState & other)
				: index(other.index)
				, pi(other.pi)
				, terminal(other.terminal)
				, assignedInRemapping(other.assignedInRemapping)
				, isNew(other.isNew)
			{
				// Intentionally left empty
			}

			double getPi() {
				return pi;
			}
			void addToPi(double add) {
				pi += add;
			}
			void setPi(double pi) {
				this->pi = pi;
			}
			bool isTerminal() {
				return terminal;
			}
			void setTerminal(bool term) {
				terminal = term;
			}
			inline bool operator==(const ProbabilityState & rhs) const {
				return index == rhs.index;
			}
			inline bool operator>=(const ProbabilityState & rhs) const {
				return index >= rhs.index;
			}
			inline bool operator<=(const ProbabilityState & rhs) const {
				return index <= rhs.index;
			}
			inline bool operator>(const ProbabilityState & rhs) const {
				return index > rhs.index;
			}
			inline bool operator<(const ProbabilityState & rhs) const {
				return index < rhs.index;
			}
			double pi;
			bool terminal;

		};

		template <typename StateType>
		struct ProbabilityStateComparison {
			bool operator() (
				const ProbabilityState<StateType> * first
				, const ProbabilityState<StateType> * second
			) const {
				// Create a max heap on the reachability probability
				return first->pi < second->pi;
			}
		};

		template <typename StateType>
		class ProbabilityStatePair {
		public:
			ProbabilityState<StateType> * first;
			CompressedState second;
		};

		template <typename StateType>
		struct ProbabilityStatePairComparison {
			bool operator() (
				const ProbabilityStatePair<StateType> first
				, const ProbabilityStatePair<StateType> second
			) const {
				// Create a max heap on the reachability probability
				return first.first->pi < second.first->pi;
			}
		};
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_PROBABILITYSTATE_H
