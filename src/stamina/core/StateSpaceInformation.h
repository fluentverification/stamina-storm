#ifndef STATESPACEINFORMATION_H
#define STATESPACEINFORMATION_H

#include <string>
#include <storm/storage/BitVector.h>
#include <storm/generator/VariableInformation.h>

namespace stamina {
	namespace core {
		typedef storm::storage::BitVector CompressedState;
		class StateSpaceInformation {
		public:
			static std::string	stateToString(CompressedState & state, double pi);
			static std::string stateToBase64String(CompressedState & state, double pi);
			static void printStateAsString(CompressedState & state, double pi);
			static void printStateAsBase64String(CompressedState & state, double pi);
			static void setVariableInformation(storm::generator::VariableInformation varInformation);
			static void printVariableNames();
		private:
			inline static storm::generator::VariableInformation variableInformation;
		};
	} // namespace core
} // namespace stamina

#endif // STATESPACEINFORMATION_H
