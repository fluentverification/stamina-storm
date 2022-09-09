// Since we must work with the filesystem, we need the CWD
#ifdef WINDOWS
	#include <direct.h>
	#define getcwd _getcwd
	#define SLASH '\\'
#else
	#include <unistd.h>
	#define SLASH '/'
#endif // WINDOWS

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <regex>

#include <boost/algorithm/string/trim.hpp>

#include <stdio.h> // For remove()

#include "ModelModify.h"
#include "core/StaminaMessages.h"

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
	if (saveModifiedModel && modifiedModel == modelFileDefault) {
		StaminaMessages::warning("The model file to export and modify is the default");
	}
	// Same with properties
	if (saveModifiedProperties && modifiedProperties == propFileDefault) {
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
	char buff[100];
	getcwd(buff, 100);
	std::string cwd(buff);
	remove((cwd + SLASH + modifiedModel).c_str());
	// Copy the current model to the new file
	std::filesystem::copy_file(originalModel, cwd + SLASH + modifiedModel);

	std::ofstream modifiedModelStream;
	modifiedModelStream.open(modifiedModel, std::ios::app); // iostream append
	modifiedModelStream << "\n\n// This module added automatically by STAMINA" << std::endl;
	modifiedModelStream << std::endl << std::endl;
	modifiedModelStream << "// All naturally generated reachable states are not our artificially created absorbing state" << std::endl;
	modifiedModelStream << "// The value of the absorbing state (and the absorbing state itself) are modified in STAMINA" << std::endl;
	modifiedModelStream << "module Absorbing_Def_STAMINA\n\n\tAbsorbing : bool init false;\n\n";
	// modifiedModelStream << "\t[] Absorbing=0 -> 1.0 : (Absorbing'=0); \n" << std::endl;
	// modifiedModelStream << "\t[] Absorbing=1 -> 1.0 : (Absorbing'=0); \n\n";
	modifiedModelStream << "endmodule" << std::endl;
	if (!saveModifiedModel) {
		modifiedModelStream << "// NOTE: if you are seeing this file it means STAMINA did not close properly. This file may be safely deleted." << std::endl;
	}
	modifiedModelStream.close();
	return std::make_shared<storm::prism::Program>(storm::parser::PrismParser::parse(modifiedModel, true));
}

std::shared_ptr<std::vector<storm::jani::Property>>
ModelModify::createModifiedProperties(
	std::shared_ptr<storm::prism::Program> modelFile
) {
	char buff[100];
	getcwd(buff, 100);
	std::string cwd(buff);
	std::string fullPath = cwd + SLASH + modifiedProperties;
	remove(fullPath.c_str());

	std::ifstream originalPropertiesStream;
	std::ofstream modifiedPropertiesStream;
	originalPropertiesStream.open(originalProperties, std::ios::in); // iostream read
	modifiedPropertiesStream.open(modifiedProperties, std::ios::out); // iostream write
	modifiedPropertiesStream << "// This file is automatically generated by STAMINA. It contains" << std::endl;
	modifiedPropertiesStream << "// modified CSL properties for the modified model file in the " << std::endl;
	modifiedPropertiesStream << "// file '" << modifiedModel << "'" << std::endl;

	std::string str;
	// std::regex r(); // TODO:
	while (std::getline(originalPropertiesStream, str)) {
		// Remove whitespace
		boost::algorithm::trim(str);
		if (str.rfind("P=?" , 0)) {
			modifiedPropertiesStream << str << std::endl;
			continue;
		}
		// Modify the property so that "absorbing is reflected
		str.pop_back();
		// std::size_t firstParenPos = str.find("(");
		// str.replace(firstParenPos, 1, "((");
		std::string propMin = str + "& (Absorbing = false) ]; // Property for Pmin\n";
		std::string propMax = str + "| (Absorbing = true) ]; // Property for Pmax\n";
		modifiedPropertiesStream << propMin << std::endl;
		modifiedPropertiesStream << propMax << std::endl;
	}
	if (!saveModifiedProperties) {
		modifiedPropertiesStream << "// NOTE: if you are seeing this file it means STAMINA did not close properly. This file may be safely deleted." << std::endl;
	}
	originalPropertiesStream.close();
	modifiedPropertiesStream.close();
	return std::make_shared<std::vector<storm::jani::Property>>(storm::api::parsePropertiesForPrismProgram(fullPath, *modelFile));
}
