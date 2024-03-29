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

#include <storm/generator/VariableInformation.h>

namespace stamina {
	namespace priority {
		class PriorityTree {
		public:
			typedef uint8_t operator_t;
			enum OPERATORS {
				LESS_THAN_EQ = 0
				, GREATER_THAN_EQ = 1
				, AND = 2
				, OR = 3
				, NOT = 4
				, EQUAL = 5
			};
			class Node {
			public:
				Node() = default;
				virtual float accumulate(CompressedState & state) = 0;
				void addChild(std::shared_ptr<Node> child);
			protected:
				std::vector<std::shared_ptr<Node>> children;
			};
			class OperatorNode : public Node {
			public:
				OperatorNode(operator_t m_operator) : m_operator(m_operator) {}
				virtual float accumulate(CompressedState & state);
			private:
				const operator_t m_operator;
			};
			/**
			 * Holds a primitive value
			 * */
			template <typename ValueType>
			class PrimitiveNode : public Node {
			public:
				PrimitiveNode() = default;
				PrimitiveNode(ValueType value)
					: value(value) {}
				virtual float accumulate(CompressedState & state);
				ValueType value;
			};
			/**
			 * Holds an integer variable value
			 * */
			class IntegerVariableNode : public Node {
			public:
				IntegerVariableNode(storm::generator::IntegerVariableInformation variable)
					: bitOffset(variable.bitOffset), bitWidth(variable.bitWidth) {}
				/* Note: converts the value to a float */
				virtual float accumulate(CompressedState & state);
				uint_fast64_t bitOffset;
				uint64_t bitWidth;
			};
			/**
			 * Holds a boolean variable value
			 * */
			class BooleanVariableNode : public Node {
			public:
				BooleanVariableNode(storm::generator::BooleanVariableInformation variable)
					: bitOffset(variable.bitOffset) {}
				/* Converts false to 0.0 and true to 1.0 */
				virtual float accumulate(CompressedState & state);
				uint_fast64_t bitOffset;
			};
			PriorityTree(
				storm::expressions::ExpressionManager & expressionManager
			) : root(nullptr)
				, expressionManager(expressionManager)
			{}
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
		protected:
			std::shared_ptr<Node> createNodeFromExpression(storm::expressions::Expression & expression);
			storm::expressions::ExpressionManager & expressionManager;
		private:
			std::shared_ptr<Node> root;
		};

		template <typename StateType>
		class EventStatePriority : public StatePriority<StateType> {
		public:
			EventStatePriority(
				bool rareEvent
				, storm::expressions::ExpressionManager & expressionManager
			) : rareEvent(rareEvent)
				, expressionManager(expressionManager)
				, tree(expressionManager)
			{}
			float priority(std::shared_ptr<builder::ProbabilityStatePair<StateType>> state) override;
			bool operatorValue(
				const std::shared_ptr<builder::ProbabilityStatePair<StateType>> first
				, const std::shared_ptr<builder::ProbabilityStatePair<StateType>> second
			) override;
			void initializePriorityTree(storm::jani::Property * property);
			void initialize(storm::jani::Property * property) override { initializePriorityTree(property); }
			const bool isRareEvent() { return rareEvent; }
		private:
			const bool rareEvent;
			PriorityTree tree;
			storm::expressions::ExpressionManager & expressionManager;
		};
	}
}
#endif // STAMINA_PRIORITY_EVENTSTATEPRIORITY_H
