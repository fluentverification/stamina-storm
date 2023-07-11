/**
* Method implementations for Stamina
*
* Created on 8/17/2021 by Josh Jeppson
* */
#include "Stamina.h"
#include "ANSIColors.h"
#include "core/StaminaMessages.h"

#include <stdlib.h>
#include <iomanip>
#include <stdlib.h>
#include <string_view>
#include <sstream>

namespace stamina {

using namespace stamina::core;

/* ===== IMPLEMENTATION FOR `Stamina::Stamina` Methods ===== */

// PUBLIC METHODS
Stamina::Stamina(struct arguments * arguments) : modelModify(new util::ModelModify(
	arguments->model_file
	, arguments->properties_file)
) {
	try {
		Options::setArgs(arguments);
	}
	catch (const std::exception& e) {
		StaminaMessages::errorAndExit("Failed to set stamina::Options: " + std::string(e.what()));
	}
	StaminaMessages::initMessage();
	StaminaMessages::info("Starting STAMINA with kappa = " + std::to_string(Options::kappa) + " and reduction factor = " + std::to_string(Options::reduce_kappa));
	bool good = Options::checkOptions();
	if (!good) {
		StaminaMessages::errorAndExit("One or more parameters passed in were invalid.");
	}
}

Stamina::Stamina()
	: modelModify(nullptr)
{
	StaminaMessages::info("Starting STAMINA");
	StaminaMessages::warning("This constructor is only to be called from the GUI! It leaves the model and properties files unloaded until specified later.");
	// bool good = Options::checkOptions();
	// if (!good) {
	// 	StaminaMessages::errorAndExit("One or more parameters passed in were invalid.");
	// }
}

Stamina::~Stamina() {
	// Intentionally left empty
}

void
Stamina::run() {
	initialize();
	// Check each property in turn
	for (auto & prop : *propertiesVector) {
		auto propMin = modelModify->modifyProperty(prop, true);
		auto propMax = modelModify->modifyProperty(prop, false);
		// Re-initialize
		// initialize();
		modelChecker->modelCheckProperty(
			propMin
			, propMax
			, prop
			, *modelFile
		);
	}
	// Finished!
	StaminaMessages::good("Finished running!");
}

// PRIVATE METHODS

void
Stamina::initialize() {
	// Check to see if we need to create a modelModify
	// Since the GUI does not specify one immediately
	if (!modelModify) {
		// Create modelModify
		std::shared_ptr<util::ModelModify> mModify(new util::ModelModify(Options::model_file, Options::properties_file));
		modelModify = mModify;
	}
	StaminaMessages::info("Stamina version is: " + std::to_string(version::version_major) + "." + std::to_string(version::version_minor) + "." + std::to_string(version::version_sub_minor));
	try {
		std::allocator<StaminaModelChecker> alloc;
		// Initialize as shared pointer
		modelChecker = std::allocate_shared<StaminaModelChecker>(alloc);
	}
	catch(const std::exception& e) {
		StaminaMessages::errorAndExit("Failed to allocate memory for StaminaModelChecker!");
	}

	// Initialize loggers
	storm::utility::setUp(); // TODO
	// Set some settings objects.
	storm::settings::initializeAll("Stamina", "Stamina");

	// Load model file and properties file
	try {
		modelFile = modelModify->readModel();
		propertiesVector = modelModify->createPropertiesList(modelFile);
		auto labels = modelFile->getLabels();
		modelChecker->initialize(modelFile, propertiesVector);
	}
	catch (const std::exception& e) {
		// Uses stringstream because std::to_string(e) throws an error with storm's exceptions
		std::stringstream msg;
		msg << "Got error when reading model or properties file:\n\t\t" << e.what();
		StaminaMessages::errorAndExit(msg.str());
	}
}



/* ===== IMPLEMENTATION FOR OTHER CLASSES IN THE `stamina` NAMESPACE ===== */

bool stamina::core::endsWith(std::string full, std::string end) {
	int i = end.size();
	int j = full.size();
	if (i > j) {
		return false;
	}
	while (i > 0) {
		if (full[j] != end[i]) {
			return false;
		}
		i--;
		j--;
	}
	return true;
}

} // namespace stamina
