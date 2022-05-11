#include <fstream>
#include <filesystem>
#include <algorithm>

#include <stdio.h> // For remove()

#include "ModelModify.h"
#include "../StaminaMessages.h"

using namespace stamina;
using namespace stamina::util;

ModelModify::ModelModify(
	std::string originalModel
	, std::string originalProperties
	, bool saveModifiedModel
	, bool saveModifiedProperties
	, std::string modifiedModel
	, std::string modifiedProperties
) : originalModel(originalModel)
	, originalProperties(originalProperties)
	, saveModifiedModel(saveModifiedModel)
	, saveModifiedProperties(saveModifiedProperties)
	, modifiedModel(modifiedModel)
	, modifiedProperties(modifiedProperties)

{
	// Check to see if the modified model is the default
	if (saveModifiedModel && modifiedModel == "modelified-model-stamina.prism") {
		StaminaMessages::warning("The model file to export and modify is the default");
	}
	// Same with properties
	if (saveModifiedProperties && modifiedProperties == "modelified-properties-stamina.csl") {
		StaminaMessages::warning("The properties file to export and modify is the default");
	}
}

ModelModify::~ModelModify() {
	if (!saveModifiedModel) {
		remove(modifiedModel.c_str());
	}
	if (!saveModifiedProperties) {
		remove(modifiedProperties.c_str());
	}
}

std::shared_ptr<storm::prism::Program>
ModelModify::createModifiedModel() {
	// Copy the current model to the new file
	std::filesystem::copy_file(originalModel, modifiedModel);

	std::ofstream modifiedModelStream;
	modifiedModelStream.open(modifiedModel, std::ios::app); // iostream append
	modifiedModelStream << std::endl << std::endl;
	modifiedModelStream << "module Absorbing_Def_STAMINA\n\tAbsorbing : bool init false;\n\tendmodule" << std::endl;
	modifiedModelStream.close();
	return std::make_shared<storm::prism::Program>(storm::parser::PrismParser::parse(modifiedModel, true));
}

std::shared_ptr<std::vector<storm::jani::Property>>
ModelModify::createModifiedProperties(
	std::shared_ptr<storm::prism::Program> modelFile
) {
	std::ifstream originalPropertiesStream;
	std::ofstream modifiedPropertiesStream;
	originalPropertiesStream.open(originalProperties, std::ios::in); // iostream read
	modifiedPropertiesStream.open(modifiedProperties, std::ios::out); // iostream write
	std::string str;
	while (std::getline(originalPropertiesStream, str)) {
		// Remove whitespace
		str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
		if (str.find("P=?" , 0)) {
			continue;
		}
		str.pop_back();
		std::string propMin = str + "&!(Absorbing)]";
		std::string propMax = str + "||(Absorbing)";
		modifiedPropertiesStream << propMin << std::endl;
		modifiedPropertiesStream << propMax << std::endl;
	}
	originalPropertiesStream.close();
	modifiedPropertiesStream.close();
	return std::make_shared<std::vector<storm::jani::Property>>(storm::api::parsePropertiesForPrismProgram(modifiedProperties, *modelFile));
}
