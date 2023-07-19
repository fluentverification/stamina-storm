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
