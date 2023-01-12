/**
 * Implementations for Event State Priority
 *
 * Created by Josh Jeppson on 1/11/2023
 * */
#include "EventStatePriority.h"

namespace stamina {
namespace priority {

/* Implementation for EventStatePriority */

template <typename StateType>
float
EventStatePriority<StateType>::priority(std::shared_ptr<ProbabilityStatePair<StateType>> state) {

}

template <typename StateType>
bool
EventStatePriority<StateType>::operatorValue(
	const std::shared_ptr<ProbabilityStatePair<StateType>> first
	, const std::shared_ptr<ProbabilityStatePair<StateType>> second
) {
	/**
	 * Explainer to make this code a little less obtuse:
	 * ProbabilityStatePair->second is the state (CompressedState &)
	 * ProbabilityStatePair->first is the state's probability information
	 *
	 * So:
	 * first->second is the state values of the first pair passed in
	 * second->second is the state values of the second pair passed in
	 * first->first is the probability information of the first pair
	 * second->first is the probability information of the second pair
	 * */
	float distanceFirst = tree.distance(first->second);
	float distanceSecond = tree.distance(second->second);
	// TODO: should invert based on rare event? Or invert at the PrimitiveNode level
	if (rareEvent) {
		// Prevent division by zero errors
		distanceFirst = 1 / std::max(distanceFirst, 0.0001);
		distanceSecond = 1 / std::max(distanceSecond, 0.0001);
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
PriorityTree::operatorNode::accumulate(CompressedState & state) {
	if (operator == OPERATORS::LESS_THAN_EQ) {
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
		float var = children[0].accumulate();
		float val = children[1].accumulate();
		return val / std::max(var, 0.0001);
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
		float var = children[0].accumulate();
		float val = children[1].accumulate();
		return var / std::max(val, 0.0001);
	}
	else if (m_operator == OPERATORS::AND) {

	}
	else if (m_operator == OPERATORS::OR) {

	}
	else if (m_operator == OPERATORS::NOT) {
		// For this, we just invert the first operator
		if (children.size() != 1) {
			StaminaMessages::errorAndExit("! operator requires only one operand! Got " + std::to_string(children.size()));
		}
		return 1 / children[0].accumulate();
	}
	else {
		// TODO: Handle error
	}
}

/* Implementation for PriorityTree::PrimitiveNode */

float
PriorityTree::PrimitiveNode::accumulate(CompressedState & state) {

}

/* Implementation for PriorityTree */

float
PriorityTree::distance(CompressedState & state) {
	return root->accumulate(state);
}

void
PriorityTree::initialize(storm::jani::Property * property) {
	// TODO
}


} // namespace priority
} // namespace stamina
