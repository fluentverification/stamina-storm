#include "StateSpaceInformation.h"
#include "StaminaMessages.h"

using namespace stamina;

std::string
StateSpaceInformation::stateToString(CompressedState & state) {
	std::string varString = "(";
	auto integerVariables  = variableInformation.integerVariables;
	auto booleanVariables  = variableInformation.booleanVariables;
	auto locationVariables = variableInformation.locationVariables;
	/* Start with integer variables */
	for (auto variable : integerVariables) {
		// It is unknown how many bits this will be
		uint_fast64_t bitOffset = variable.bitOffset;
		// Get the value as an integer
		uint16_t bitWidth = variable.bitWidth;
		if (bitWidth > 64) {
			StaminaMessages::warning("Int size is " + std::to_string(bitWidth));
		}
		uint_fast64_t variableValue = state.getAsInt(bitOffset, bitWidth);
		varString += std::to_string(variableValue);
		varString += ",";
	}
	/* Then boolean variables */
	for (auto variable : booleanVariables) {
		// Single bit
		uint_fast64_t offset = variable.bitOffset;
		if (state.get(offset)) {
			varString += "t";
		}
		else {
			varString += "f";
		}
		varString += ",";
	}
	/* Then location Variables */
	for (auto variable : locationVariables) {
		StaminaMessages::error("Location Variable printing not implemented yet!");
	}
	varString += "\b)";
	return varString;
}

std::string
StateSpaceInformation::stateToBase64String(CompressedState & state) {
	std::string stateString = stateToString(state);
}

void
StateSpaceInformation::printStateAsString(CompressedState & state) {
	std::cout << stateToString(state) << std::endl;
}

void
StateSpaceInformation::printStateAsBase64String(CompressedState & state) {
	std::cout << stateToBase64String(state) << std::endl;
}


void
StateSpaceInformation::setVariableInformation(storm::generator::VariableInformation varInformation) {
	variableInformation = varInformation;
}

void
StateSpaceInformation::printVariableNames() {
	std::string varString = "(";
	auto integerVariables  = variableInformation.integerVariables;
	auto booleanVariables  = variableInformation.booleanVariables;
	auto locationVariables = variableInformation.locationVariables;
	/* Start with integer variables */
	for (auto variable : integerVariables) {
		// It is unknown how many bits this will be
		uint_fast64_t lowerBound = variable.lowerBound;
		uint_fast64_t upperBound = variable.upperBound;
		// Get the value as an integer
		varString += variable.variable.getName();
		varString += ",";
	}
	/* Then boolean variables */
	for (auto variable : booleanVariables) {
		// Single bit
		uint_fast64_t offset = variable.bitOffset;
		varString += variable.variable.getName();
		varString += ",";
	}
	/* Then location Variables */
	for (auto variable : locationVariables) {
		StaminaMessages::error("Location Variable printing not implemented yet!");
	}
	varString += "\b)";
	std::cout << varString << std::endl;
}
