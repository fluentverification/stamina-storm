/**
* Method implementations for Stamina
*
* Created on 8/17/2021 by Josh Jeppson
* */
#include "Stamina.h"
#include "ANSIColors.h"

#include <stdlib.h>
#include <iomanip>
#include <stdlib.h>
#include <string_view>
#include <sstream>

using namespace stamina;

/* ===== IMPLEMENTATION FOR `Stamina::Stamina` Methods ===== */

// PUBLIC METHODS
Stamina::Stamina(struct arguments * arguments) {
	try {
		Options::setArgs(arguments);
	}
	catch (const std::exception& e) {
		StaminaMessages::errorAndExit("Failed to allocate stamina::Options: " + std::string(e.what()));
	}
	StaminaMessages::info("Starting STAMINA with kappa = " + std::to_string(Options::kappa) + " and reduction factor = " + std::to_string(Options::reduce_kappa));
	// Pass in a lambda (bound function) to the checkOptions method
	bool good = Options::checkOptions();
	if (!good) {
		StaminaMessages::errorAndExit("One or more parameters passed in were invalid.");
	}
}

Stamina::~Stamina() {
	// Clean up all memory
	delete this->modelChecker;
}

void 
Stamina::run() {
	initialize();
	// Check each property in turn
	for (auto property : propertiesVector) {
#ifdef DEBUG_PRINTS
		StaminaMessages::debugPrint("Checking property in properties vector.");
#endif
		auto result = modelChecker->modelCheckProperty(property, modulesFile);
	}
	// Finished!
	StaminaMessages::good("Finished running!");
}

// PRIVATE METHODS

void 
Stamina::initialize() {
	StaminaMessages::info("Stamina version is: " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR));
	try {
		// Initialize with references to error, warning, info, and good message functions
		modelChecker = new StaminaModelChecker();
	}
	catch(const std::exception& e) {
		StaminaMessages::errorAndExit("Failed to allocate memory for StaminaModelChecker!");
	}

	// Initialize loggers
	// storm::utility::setUp(); // TODO
	// Set some settings objects.
	storm::settings::initializeAll("Stamina", "Stamina");

	// Load modules file and properties file
	try {
		modulesFile = storm::parser::PrismParser::parse(Options::model_file, true);
		propertiesVector = storm::api::parsePropertiesForPrismProgram(Options::properties_file, modulesFile);
		modelChecker->initialize(&modulesFile, &propertiesVector);
	}
	catch (const std::exception& e) {
		// Uses stringstream because std::to_string(e) throws an error with storm's exceptions
		std::stringstream msg;
		msg << "Got error when reading modules or properties file:\n\t\t" << e.what();
		StaminaMessages::errorAndExit(msg.str());
	}

}



/* ===== IMPLEMENTATION FOR OTHER CLASSES IN THE `stamina` NAMESPACE ===== */

bool stamina::endsWith(std::string full, std::string end) {
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
