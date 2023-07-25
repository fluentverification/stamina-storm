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

#ifndef STAMINA_CORE_STATESPACEINFORMATION_H
#define STAMINA_CORE_STATESPACEINFORMATION_H

#include <string>
#include <storm/storage/BitVector.h>
#include <storm/generator/VariableInformation.h>

namespace stamina {
	namespace core {
		typedef storm::storage::BitVector CompressedState;
		class StateSpaceInformation {
		public:
			static std::string stateToString(CompressedState const & state, double pi = -1.0);
			static std::string stateToBase64String(CompressedState const & state, double pi);
			static void printStateAsString(CompressedState const & state, double pi);
			static void printStateAsBase64String(CompressedState const & state, double pi);
			static void setVariableInformation(storm::generator::VariableInformation varInformation);
			static storm::generator::VariableInformation getVariableInformation() { return variableInformation; }
			static void printVariableNames();
			static storm::generator::BooleanVariableInformation getInformationOnBooleanVariable(
				storm::expressions::Variable variable
			);
			static storm::generator::IntegerVariableInformation getInformationOnIntegerVariable(
				storm::expressions::Variable variable
			);
		private:
			inline static storm::generator::VariableInformation variableInformation;
		};
	} // namespace core
} // namespace stamina

#endif // STAMINA_CORE_STATESPACEINFORMATION_H
