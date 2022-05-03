#ifndef STATESPACEINFORMATION_H
#define STATESPACEINFORMATION_H

#include <string>
#include <storm/storage/BitVector.h>
#include <storm/generator/VariableInformation.h>

namespace stamina {
	typedef storm::storage::BitVector CompressedState;
	class StateSpaceInformation {
	public:
		static std::string	stateToString(CompressedState & state);
		static std::string stateToBase64String(CompressedState & state);
		static void printStateAsString(CompressedState & state);
		static void printStateAsBase64String(CompressedState & state);
		static void setVariableInformation(storm::generator::VariableInformation varInformation);
		static void printVariableNames();
	private:
		inline static storm::generator::VariableInformation variableInformation;
	};
} // namespace stamina

#endif // STATESPACEINFORMATION_H
