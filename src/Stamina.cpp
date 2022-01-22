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
		Options::->setArgs(arguments);
	}
	catch (const std::exception& e) {
		errorAndExit("Failed to allocate stamina::Options: " + std::string(e.what()));
	}
	info("Starting STAMINA with kappa = " + std::to_string(options->kappa) + " and reduction factor = " + std::to_string(options->reduce_kappa));
	// Pass in a lambda (bound function) to the checkOptions method
	bool good = Options::checkOptions();
	if (!good) {
		errorAndExit("One or more parameters passed in were invalid.");
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
		debugPrint("Checking property in properties vector.");
#endif
		auto result = modelChecker->modelCheckProperty(property, modulesFile);
	}
	// Finished!
	good("Finished running!");
}

// PRIVATE METHODS

void 
Stamina::initialize() {
	info("Stamina version is: " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR));
	try {
		// Initialize with references to error, warning, info, and good message functions
		modelChecker = new StaminaModelChecker();
	}
	catch(const std::exception& e) {
		errorAndExit("Failed to allocate memory for StaminaModelChecker!");
	}

	// Initialize loggers
	// storm::utility::setUp(); // TODO
	// Set some settings objects.
	storm::settings::initializeAll("Stamina", "Stamina");

	// Load modules file and properties file
	try {
		modulesFile = storm::parser::PrismParser::parse(options->model_file, true);
		propertiesVector = storm::api::parsePropertiesForPrismProgram(options->properties_file, modulesFile);
		modelChecker->initialize(&modulesFile, &propertiesVector);
	}
	catch (const std::exception& e) {
		// Uses stringstream because std::to_string(e) throws an error with storm's exceptions
		std::stringstream msg;
		msg << "Got error when reading modules or properties file:\n\t\t" << e.what();
		errorAndExit(msg.str());
	}

}

void 
Stamina::errorAndExit(std::string err, uint8_t err_num) {
	std::cerr << BOLD(FRED("[ERROR]: "));
	std::cerr << BOLD("STAMINA encountered the following error and will now exit: ") << std::endl;
	std::cerr << '\t' << err << std::endl;
	exit(err_num);
}

void 
Stamina::error(std::string err, uint8_t err_num) {
	std::cerr << BOLD(FRED("[ERROR]: "));
	std::cerr << BOLD("STAMINA encountered the following (possibly recoverable) error: ") << std::endl;
	std::cerr << '\t' << err << std::endl;
}

void 
Stamina::warning(std::string warn) {
	std::cerr << BOLD(FYEL("[WARNING]: ")) << warn << std::endl;
}

void 
Stamina::info(std::string info) {
	std::cerr << BOLD(FBLU("[INFO]: ")) << info << std::endl;
}

void 
Stamina::good(std::string good) {
	std::cerr << BOLD(FGRN("[MESSAGE]: ")) << good << std::endl;
}

#ifdef DEBUG_PRINTS
void Stamina::debugPrint(std::string msg) {
	std::cout << BOLD(FMAG("[DEBUG MESSAGE]: ")) << msg << std::endl;
}
#endif

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
