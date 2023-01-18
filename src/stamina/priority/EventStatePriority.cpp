/**
 * Implementations for Event State Priority
 *
 * Created by Josh Jeppson on 1/11/2023
 * */
#include "EventStatePriority.h"

namespace stamina {
namespace priority {

const float SMALL_VALUE = 0.0001;

/* Implementation for EventStatePriority */

template <typename StateType>
float
EventStatePriority<StateType>::priority(std::shared_ptr<builder::ProbabilityStatePair<StateType>> state) {

}

template <typename StateType>
bool
EventStatePriority<StateType>::operatorValue(
	const std::shared_ptr<builder::ProbabilityStatePair<StateType>> first
	, const std::shared_ptr<builder::ProbabilityStatePair<StateType>> second
) {
	/**
	 * Explainer to make this code a little less obtuse:
	 * builder::ProbabilityStatePair->second is the state (CompressedState &)
	 * builder::ProbabilityStatePair->first is the state's probability information
	 *
	 * So:
	 * first->second is the state values of the first pair passed in
	 * second->second is the state values of the second pair passed in
	 * first->first is the probability information of the first pair
	 * second->first is the probability information of the second pair
	 * */
	// TODO: change to make it so these are calculated ONCE (when the state is
	// first encountered) and stored
	float distanceFirst = tree.distance(first->second);
	float distanceSecond = tree.distance(second->second);
	// TODO: should invert based on rare event? Or invert at the PrimitiveNode level
	if (rareEvent) {
		// Prevent division by zero errors
		distanceFirst = 1 / std::max(distanceFirst, SMALL_VALUE);
		distanceSecond = 1 / std::max(distanceSecond, SMALL_VALUE);
	}
	float compositeFirst = distanceFirst * first->first->pi;
	float compositeSecond = distanceSecond * second->first->pi;
	// Create a max heap on the composite (distance * reachability)
	return compositeFirst < compositeSecond;
}

template <typename StateType>
void
EventStatePriority<StateType>::initializePriorityTree(storm::jani::Property * property) {
	tree.initialize(property);
}

/* Implementation for PriorityTree::Node */

void
PriorityTree::Node::addChild(std::shared_ptr<Node> child) {
	this->children.append(child);
}

/* Implementation for PriorityTree::operatorNode */

float
PriorityTree::OperatorNode::accumulate(CompressedState & state) {
	if (m_operator == OPERATORS::LESS_THAN_EQ) {
		// Require only 2 operands
		if (children.size() != 2) {
			StaminaMessages::errorAndExit("< or <= operator should have two operand! Got " + std::to_string(children.size()));
		}
		/**
		 * For var < val, the distance is calculated as
		 *       val
		 *  d  = ---
		 *       var
		 * since we want a higher d for a lower var
		 * */
		float var = children[0]->accumulate(state);
		float val = children[1]->accumulate(state);
		return val / std::max(var, SMALL_VALUE);
	}
	else if (m_operator == OPERATORS::GREATER_THAN_EQ) {
		// Require only 2 operands
		if (children.size() != 2) {
			StaminaMessages::errorAndExit("> or >= operator should have two operand! Got " + std::to_string(children.size()));
		}
		/**
		 * For var > val, the distance is calculated as
		 *       var
		 *  d  = ---
		 *       val
		 * since we want a higher d for a lower var
		 * */
		float var = children[0]->accumulate(state);
		float val = children[1]->accumulate(state);
		return var / std::max(val, SMALL_VALUE);
	}
	else if (m_operator == OPERATORS::AND) {
		// For the "and" operator, we can chain all of the distances of the children
		// together using multiplication
		float distance = 1;
		for (auto child : children) {
			distance *= child->accumulate(state);
		}
		return distance;
	}
	else if (m_operator == OPERATORS::OR) {
		// For the "or" operator, we also chain the distances of the children using
		// the addition operator
		float distance = 0;
		for (auto child : children) {
			distance += child->accumulate(state);
		}
		return distance / children.size(); // TODO: Normalize or not?
	}
	else if (m_operator == OPERATORS::NOT) {
		// For this, we just invert the first operator
		if (children.size() != 1) {
			StaminaMessages::errorAndExit("! operator requires only one operand! Got " + std::to_string(children.size()));
		}
		return 1 / children[0]->accumulate(state);
	}
	else {
		StaminaMessages::errorAndExit("Unknown operator! (integer value " + std::to_string(m_operator) + ")");
	}
}

/* Implementation for PriorityTree::PrimitiveNode */

template <typename ValueType>
float
PriorityTree::PrimitiveNode<ValueType>::accumulate(CompressedState & state) {
	return value;
}

/* Implementation for PriorityTree::IntegerVariableNode */

float
PriorityTree::IntegerVariableNode::accumulate(CompressedState & state) {
	uint_fast64_t bitOffset = variable.bitOffset;
	uint16_t bitWidth = variable.bitWidth;
	return (float) state.getAsInt(bitOffset + 1, bitWidth - 1);
}

/* Implementation for PriorityTree::BooleanVariableNode */

float
PriorityTree::BooleanVariableNode::accumulate(CompressedState & state) {
	uint_fast64_t offset = variable.bitWidth;
	return 1.0 ? state.get(offset) : 0.0;
}

/* Implementation for PriorityTree */

float
PriorityTree::distance(CompressedState & state) {
	return root->accumulate(state);
}

void
PriorityTree::initialize(storm::jani::Property * property) {
	// Create root node
	auto expression = property->getExpression();
	auto simplifiedExpression = expression.simplify();
	auto nonNestedExpression = simplifiedExpression.reduceNesting();
	// createNodeFromExpression is recursive
	this->root = createNodeFromExpression(expression);
}

std::shared_ptr<Node>
PriorityTree::createNodeFromExpression(storm::expressions::Expression & expression) {
	// Determine what type
	// TODO
	/* If the expression is a primitive
	 *     Create and return a PrimitiveNode
	 * Else if it is a variable
	 *     Create and return a VariableNode
	 * Else:
	 *     Create an OperatorNode
	 *     For each operand:
	 *         call createNodeFromExpression
	 *         append result as child of OperatorNode
	 *     return the OperatorNode
	 * */
	if (expression.hasNumericalType()) {

	}
	else if (expression.hasBooleanType()) {

	}
	else if (expression.isVariable()) {
		// Create variable node from variable
		auto vars = expression.getVariables();
		auto var = // TODO: vars should only have one element
		std::shared_ptr<VariableNode> vNode(new VariableNode(var));
		return vNode;
	}
	else if (expression.isFunctionApplication()) {
		auto op = expression.getOperator();
		std::shared_ptr<OperatorNode> opNode( /* TODO: Constructor*/ );
		for (auto ex : ) {
			auto child = createNodeFromExpression(ex);
			opNode->addChild(child);
		}
		return opNode;
	}
}

} // namespace priority
} // namespace stamina
