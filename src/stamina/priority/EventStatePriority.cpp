/**
 * Implementations for Event State Priority
 *
 * Created by Josh Jeppson on 1/11/2023
 * */
#include "EventStatePriority.h"

#include "core/StateSpaceInformation.h"

namespace stamina {
namespace priority {

const float SMALL_VALUE = 0.0001;

/* Implementation for EventStatePriority */

template <typename StateType>
float
EventStatePriority<StateType>::priority(std::shared_ptr<builder::ProbabilityStatePair<StateType>> state) {
	float distance = tree.distance(state->second);
	state->distance = std::min(distance, 1.0f);
	return distance;
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
	this->children.push_back(child);
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
		 *       var - val
		 *  d  = ---------
		 *		  var
		 * since we want a higher d for a lower var
		 * */
		float var = children[0]->accumulate(state);
		float val = children[1]->accumulate(state);
		return std::max(var - val, 0.0f) / std::max(var, SMALL_VALUE);
	}
	else if (m_operator == OPERATORS::GREATER_THAN_EQ) {
		// Require only 2 operands
		if (children.size() != 2) {
			StaminaMessages::errorAndExit("> or >= operator should have two operand! Got " + std::to_string(children.size()));
		}
		/**
		 * For var > val, the distance is calculated as
		 *       val - var
		 *  d  = ---------
		 *		  val
		 * since we want a higher d for a lower var
		 * */
		float var = children[0]->accumulate(state);
		float val = children[1]->accumulate(state);
		return std::max(val - var, 0.0f) / std::max(val, SMALL_VALUE);
	}
	else if (m_operator == OPERATORS::AND || m_operator == OPERATORS::OR) {
		// For the "and" operator, we can chain all of the distances of the children
		// together using multiplication
		float distance = 0;
		for (auto child : children) {
			distance += child->accumulate(state);
		}
		// We subtract 1 because we pass in propMin when creating the tree, which includes (Absorbing = False)
		return (distance - 1) / children.size();
	}
	/*else if (m_operator == OPERATORS::) {
		// For the "or" operator, we also chain the distances of the children using
		// the addition operator
		float distance = 1;
		for (auto child : children) {
			distance *= child->accumulate(state);
		}
		return distance; // TODO: Normalize or not?
	}*/
	else if (m_operator == OPERATORS::NOT) {
		// For this, we just invert the first operator
		if (children.size() != 1) {
			StaminaMessages::errorAndExit("! operator requires only one operand! Got " + std::to_string(children.size()));
		}
		return 1 / children[0]->accumulate(state);
	}
	else if (m_operator == OPERATORS::EQUAL) {
		if (children.size() > 2) {
			StaminaMessages::errorAndExit("Can only have less than two operators for equal!");
		}
		// If the size is 1, we assume the value is a boolean and say
		// is the value == 1. Therefore the distance is value - 1
		else if (children.size() == 1) {
			float var = children[0]->accumulate(state);
			return std::abs(var - 1);
		}
		else {
			float var = children[0]->accumulate(state);
			float val = children[1]->accumulate(state);
			return std::abs(var - val);
		}
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
	uint_fast64_t bitOffset = this->bitOffset;
	uint16_t bitWidth = this->bitWidth;
	if (bitWidth > 64) {
		StaminaMessages::warning("Int size is " + std::to_string(bitWidth));
	}
	return (float) state.getAsInt(bitOffset + 1, bitWidth - 1);
}

/* Implementation for PriorityTree::BooleanVariableNode */

float
PriorityTree::BooleanVariableNode::accumulate(CompressedState & state) {
	uint_fast64_t offset = this->bitOffset;
	return 1.0 ? state.get(offset) : 0.0;
}

/* Implementation for PriorityTree */

float
PriorityTree::distance(CompressedState & state) {
	if (!wasInitialized()) {
		StaminaMessages::warning("Priority tree was not initialized! Returning 0 for distance!");
		return 0.0;
	}
	return root->accumulate(state);
}

void
PriorityTree::initialize(storm::jani::Property * property) {
	// auto expressionManager = property->getRawFormula()->getAtomicExpressionFormulas()[0]->getExpression().getManager();
	// Create root node
	auto formula = property->getRawFormula();

	// Gets the portion of the formula withouth the probability operator
	auto const & subFormula = formula->asOperatorFormula().getSubformula();
	// Gets the portion to the right of the boundedUntilFormula portion [X U f] gets f
	if (!subFormula.isBoundedUntilFormula()) {
		StaminaMessages::errorAndExit("Formula must be bounded until formula!");
	}
	auto const & subSubFormula = subFormula.asBoundedUntilFormula().getRightSubformula();
	// Convert the formula to an expression and simplify
	auto expression = subSubFormula.toExpression(this->expressionManager);
	auto simplifiedExpression = expression.simplify();
	auto nonNestedExpression = simplifiedExpression.reduceNesting();
	// createNodeFromExpression is recursive
	this->root = createNodeFromExpression(expression);
}

std::shared_ptr<PriorityTree::Node>
PriorityTree::createNodeFromExpression(storm::expressions::Expression & expression) {
	// Determine what type
	/* If the expression is a primitive
	 *     Create and return a PrimitiveNode
	 * Else if it is a variable
	 *     Create and return a VariableNode
	 * Else:
	 *     Create an OperatorNode
	 *     For each operand:
	 *		 call createNodeFromExpression
	 *		 append result as child of OperatorNode
	 *     return the OperatorNode
	 * */
	if (expression.hasNumericalType() && expression.isLiteral()) {
		int val = expression.evaluateAsInt();
		StaminaMessages::info("Creating node for integer literal " + expression.toString());
		std::shared_ptr<PriorityTree::PrimitiveNode<int>> intNode(new PriorityTree::PrimitiveNode<int>(val));
		return intNode;
	}
	else if (expression.hasBooleanType() && expression.isLiteral()) {
		StaminaMessages::info("Creating node for boolean literal " + expression.toString());
		bool val = expression.evaluateAsBool();
		std::shared_ptr<PriorityTree::PrimitiveNode<bool>> boolNode(new PriorityTree::PrimitiveNode<bool>(val));
		return boolNode;
	}
	else if (expression.isVariable()) {
		// Create variable node from variable
		auto vars = expression.getVariables();
		if (vars.size() != 1) {
			StaminaMessages::errorAndExit("Variables count should be 1 for this expression!\n\tExpression:"
				+ expression.toString()
				+ "\n\tCount: " + std::to_string(vars.size())
			);
		}
		auto var = *vars.begin();
		if (var.getType().isIntegerType()) {
			// get the variable information from variable
			StaminaMessages::info("Creating node for integer var " + expression.toString());
			auto integerVariableInformation = core::StateSpaceInformation::getInformationOnIntegerVariable(var);
			std::shared_ptr<PriorityTree::IntegerVariableNode> vNode(new PriorityTree::IntegerVariableNode(integerVariableInformation));
			return vNode;
		}
		else if (var.getType().isBooleanType()) {
			// get the variable information from variable
			StaminaMessages::info("Creating node for boolean var " + expression.toString());
			auto booleanVariableInformation = core::StateSpaceInformation::getInformationOnBooleanVariable(var);
			std::shared_ptr<PriorityTree::BooleanVariableNode> vNode(new PriorityTree::BooleanVariableNode(booleanVariableInformation));
			return vNode;
		}
		else {
			StaminaMessages::errorAndExit("Unknown Variable Type!");
		}
	}
	else if (expression.isFunctionApplication()) {
		StaminaMessages::info("Attempting to create node for " + expression.toString());
		auto op = expression.getOperator();
		operator_t m_operator; // We have to convert the operator to our type
		switch (op) {
			case storm::expressions::OperatorType::And:
				StaminaMessages::info("Operator: AND");
				m_operator = PriorityTree::OPERATORS::AND;
				break;
			case storm::expressions::OperatorType::Or:
				StaminaMessages::info("Operator: OR");
				m_operator = PriorityTree::OPERATORS::OR;
				break;
			case storm::expressions::OperatorType::Not:
				StaminaMessages::info("Operator: NOT");
				m_operator = PriorityTree::OPERATORS::NOT;
				break;
			case storm::expressions::OperatorType::Less:
			case storm::expressions::OperatorType::LessOrEqual:
				StaminaMessages::info("Operator: LESS/LEQ");
				m_operator = PriorityTree::OPERATORS::LESS_THAN_EQ;
				break;
			case storm::expressions::OperatorType::Greater:
				StaminaMessages::info("Operator: GREATER/GEQ");
			case storm::expressions::OperatorType::GreaterOrEqual:
				m_operator = PriorityTree::OPERATORS::GREATER_THAN_EQ;
				break;
			case storm::expressions::OperatorType::Xor:
				StaminaMessages::errorAndExit("Operator (Xor) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Implies:
				StaminaMessages::errorAndExit("Operator (Implies) not yet supported by OperatorNode!");
				break;
			// PriorityTree::OPERATORS::EQUAL when has only one operand assumes == 1 or == true
			case storm::expressions::OperatorType::Equal:
			case storm::expressions::OperatorType::Iff:
				StaminaMessages::info("Operator: EQ");
				m_operator = PriorityTree::OPERATORS::EQUAL;
				break;
			case storm::expressions::OperatorType::Plus:
				StaminaMessages::errorAndExit("Operator (Plus) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Minus:
				StaminaMessages::errorAndExit("Operator (Minus) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Times:
				StaminaMessages::errorAndExit("Operator (Times) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Divide:
				StaminaMessages::errorAndExit("Operator (Divide) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Min:
				StaminaMessages::errorAndExit("Operator (Min) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Max:
				StaminaMessages::errorAndExit("Operator (Max) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Power:
				StaminaMessages::errorAndExit("Operator (Power) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Modulo:
				StaminaMessages::errorAndExit("Operator (Modulo) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::NotEqual:
				StaminaMessages::errorAndExit("Operator (NotEqual) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Floor:
				StaminaMessages::errorAndExit("Operator (Floor) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Ceil:
				StaminaMessages::errorAndExit("Operator (Ceil) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::Ite:
				StaminaMessages::errorAndExit("Operator (Ite) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::AtLeastOneOf:
				StaminaMessages::errorAndExit("Operator (AtLeastOneOf) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::AtMostOneOf:
				StaminaMessages::errorAndExit("Operator (AtMostOneOf) not yet supported by OperatorNode!");
				break;
			case storm::expressions::OperatorType::ExactlyOneOf:
				StaminaMessages::errorAndExit("Operator (ExactlyOneOf) not yet supported by OperatorNode!");
				break;
			default:
				StaminaMessages::errorAndExit("Operator not supported by OperatorNode");
				break;
		}
		std::shared_ptr<PriorityTree::OperatorNode> opNode( new OperatorNode(m_operator) );
		// For the linguistics challenged programmer such as myself:
		// Arity: the number of operands or count of elements taken
		//		by an operator
		for (int i = 0; i < expression.getArity(); i++) {
			auto ex = expression.getOperand(i);
			auto child = createNodeFromExpression(ex);
			opNode->addChild(child);
		}
		return opNode;
	}
}

// Forward declare EventStatePriority class
template class EventStatePriority<uint32_t>;

} // namespace priority
} // namespace stamina
