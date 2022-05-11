#include <fstream>
#include <filesystem>

#include "ModelModify.h"
#include "StaminaMessages.h"

using namespace stamina;
using namespace stamina::util;

ModelModifiy::ModelModify(
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
		std::filesystem::delete_file(modifiedModel);
	}
	if (!saveModifiedProperties) {
		std::filesystem::delete_file(modifiedProperties);
	}
}

std::shared_ptr<storm::prism::Program>
ModelModify::createModifiedModel() {
	// Copy the current model to the new file
	std::filesystem::copy_file(originalModel, modifiedModel);

	std::ofstream modifiedModelStream;
	modifiedModelStream.open(modifiedModel, 'a');
	modifiedModelStream << std::endl << std::endl;
	modifiedModelStream << "module Absorbing_Def_STAMINA\n\tAbsorbing : bool init false;\n\tendmodule" << std::endl;
}

std::shared_ptr<std::vector<storm::jani::Property>>
ModelModify::createModifiedProperties() {

}
