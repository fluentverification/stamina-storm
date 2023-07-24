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

#include "StateSpaceInformation.h"
#include "StaminaMessages.h"

namespace stamina {
namespace core {

std::string
StateSpaceInformation::stateToString(CompressedState const & state, double pi) {
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
	if (pi != -1.0) {
		varString += std::to_string(((int) (pi * 1000000)) / 1000000.0);
		varString += ")";
	}
	return varString;
}

std::string
StateSpaceInformation::stateToBase64String(CompressedState const & state, double pi) {
	std::string stateString = stateToString(state, pi);
	return stateString;
}

void
StateSpaceInformation::printStateAsString(CompressedState const & state, double pi) {
	std::cout << stateToString(state, pi) << std::endl;
}

void
StateSpaceInformation::printStateAsBase64String(CompressedState const & state, double pi) {
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

storm::generator::BooleanVariableInformation
StateSpaceInformation::getInformationOnBooleanVariable(
	storm::expressions::Variable variable
) {
	for (auto bvi : variableInformation.booleanVariables) {
		if (bvi.variable == variable) {
			return bvi;
		}
	}
	StaminaMessages::error("Unable to find variable information! (Boolean variable: " + variable.getName() + ")");
}

storm::generator::IntegerVariableInformation
StateSpaceInformation::getInformationOnIntegerVariable(
	storm::expressions::Variable variable
) {
	for (auto ivi : variableInformation.integerVariables) {
		if (ivi.variable == variable) {
			return ivi;
		}
	}
	StaminaMessages::error("Unable to find variable information! (Integer variable: " + variable.getName() + ")");
}

} // namespace core
} // namespace stamina
