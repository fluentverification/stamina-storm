#include "StateSpaceInformation.h"
#include "StaminaMessages.h"

using namespace stamina;

std::string
StateSpaceInformation::stateToString(CompressedState & state, double pi) {
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
		uint_fast64_t variableValue = state.getAsInt(bitOffset + 1, bitWidth - 1);
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
	varString += std::to_string(((int) (pi * 10000)) / 10000.0);
	varString += ")";
	return varString;
}

std::string
StateSpaceInformation::stateToBase64String(CompressedState & state, double pi) {
	std::string stateString = stateToString(state, pi);
}

void
StateSpaceInformation::printStateAsString(CompressedState & state, double pi) {
	std::cout << stateToString(state, pi) << std::endl;
}

void
StateSpaceInformation::printStateAsBase64String(CompressedState & state, double pi) {
	std::cout << stateToBase64String(state, pi) << std::endl;
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
