/**
 * Event State Priority Class
 * Created by Josh Jeppson on 1/11/2023
 *
 * Creates a metric based on whether we are looking for a rare event or a commen event based on
 * the state distance to the threshold.
 * */
#ifndef STAMINA_PRIORITY_EVENTSTATEPRIORITY_H
#define STAMINA_PRIORITY_EVENTSTATEPRIORITY_H

#include "StatePriority.h"

namespace stamina {
	namespace priority {
		class PriorityTree;

		template <typename StateType>
		class EventStatePriority : public StatePriority<StateType> {
		public:
			EventStatePriority(bool rareEvent) : rareEvent(rareEvent) {}
			virtual static float priority(std::shared_ptr<ProbabilityStatePair<StateType>> state);
			virtual bool operatorValue(
				const std::shared_ptr<ProbabilityStatePair<StateType>> first
				, const std::shared_ptr<ProbabilityStatePair<StateType>> second
			);
			void initializePriorityTree(storm::jani::Property * property);
			const bool isRareEvent() { return rareEvent; }
		private:
			const bool rareEvent;
			PriorityTree tree;
		};

		class PriorityTree {
		public:
			typedef operand_t uint8_t;
			enum OPERANDS {
				LESS_THAN_EQ = 0;
				GREATER_THAN_EQ = 1;
			}
			class Node {
			public:
				Node() = default;
				virtual float accumulate(CompressedState & state) = 0;
				void addChild(std::shared_ptr<Node> child);
			protected:
				std::vector<std::shared_ptr<Node>> children;
			}
			class OperandNode : public Node {
			public:
				OperandNode(operand_t operand) : operand(operand) {}
				virtual float accumulate(CompressedState & state);
			private:
				const operand_t operand;
			}
			class PrimitiveNode : public Node {
			public:
				PrimitiveNode() = default;
				virtual float accumulate(CompressedState & state);
			}
			PriorityTree() : root(nullptr) {}
			/**
			 * Calculates a composite of the "normalized" distance
			 * from the state values to the threshold of the parameter that was setup
			 *
			 * @param state The state to calculate the distance
			 * @return The distance to the threshold
			 * */
			float distance(CompressedState & state);
			/**
			 * Initializes the priority tree based on a particular property
			 *
			 * @param property The property to initialize based on
			 * */
			void initialize(storm::jani::Property * property);
			bool wasInitialized() { return root != nullptr; }
		private:
			std::shared_ptr<Node> root;
		}
	}
}
#endif // STAMINA_PRIORITY_EVENTSTATEPRIORITY_H
