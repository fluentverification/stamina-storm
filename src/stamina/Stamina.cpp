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
	)
	, wasInitialized(false)
{
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
	// Initialize loggers
	storm::utility::setUp();
	// Set some settings objects.
	storm::settings::initializeAll("Stamina", "Stamina");
}

Stamina::Stamina()
	: modelModify(nullptr)
	, wasInitialized(false)
{
	StaminaMessages::info("Starting STAMINA");
	StaminaMessages::warning("This constructor is only to be called from the GUI! It leaves the model and properties files unloaded until specified later.");
	// Initialize loggers
	storm::utility::setUp();
	// Set some settings objects.
	storm::settings::initializeAll("Stamina", "Stamina");
	// bool good = Options::checkOptions();
	// if (!good) {
	// 	StaminaMessages::errorAndExit("One or more parameters passed in were invalid.");
	// }
}

Stamina::~Stamina() {
	// Intentionally left empty
}

void
Stamina::run(bool rebuild) {
	if (rebuild) {
		wasInitialized = false;
		reInitialize();
	}
	else {
		initialize();
	}
	// Create formulas vector
	std::vector<std::shared_ptr< storm::logic::Formula const>> fv;
	for (auto & prop : *propertiesVector) {
		auto formula = prop.getFilter().getFormula();
		fv.push_back(formula);
	}
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
			, fv
			, rebuild
		);
	}
	// Finished!
	StaminaMessages::good("Finished running!");
	storm::utility::cleanUp();
}

// PRIVATE METHODS

void
Stamina::initialize() {
	if (wasInitialized) { return; }
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
	wasInitialized = true;
}

void Stamina::reInitialize() {
	if (modelModify) {
		modelModify->setModelAndProperties(Options::model_file, Options::properties_file);
	}
	wasInitialized = false;
	initialize();

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
