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

#include "StaminaMessages.h"

#include "ANSIColors.h"
#include "core/Options.h"

#include <stdlib.h>
#include <iomanip>
#include <string_view>
#include <sstream>
#include <iostream>
#include <stdexcept>

#define VERSION_MAJOR 2
#define VERSION_MINOR 2.5

namespace stamina {
namespace core {

const std::string StaminaMessages::horizontalSeparator = "========================================================================================";

void
StaminaMessages::initMessage() {
	std::cout << horizontalSeparator << std::endl;
	std::cout << BOLD("STAMINA -- The STochiastic Approximate Model-checker, for INfinite-state Analysis") << std::endl;
	std::cout << horizontalSeparator << std::endl;
	std::cout << "(C) 2023 Fluent Verification Research Group -- Licensed freely under the GPLv3 license" << std::endl;
	std::cout << "Version: " << VERSION_MAJOR << "." << VERSION_MINOR << std::endl;
	std::cout << "Developers: J Jeppson, Z Zhang, R Roberts, T Neupane, and others." << std::endl;
	std::cout << "Website: https://staminachecker.org" << std::endl;
	std::cout << "Repository: https://github.com/fluentverification/stamina-storm" << std::endl;
	std::cout << horizontalSeparator << std::endl;
	std::cout << "Model checker: Storm (https://stormchecker.org) - Licensed under the GPLv3" << std::endl;
	std::cout << "Storm Authors: C Hensel, S Junges, J Katoen, T Quatmann, M Volk" << std::endl;
	std::cout << horizontalSeparator << std::endl;
}

void
StaminaMessages::errorAndExit(std::string err, uint8_t err_num) {
	if (Options::quiet) {
		exit(err_num);
	}
	std::cerr << BOLD(FRED("[ERROR]: "));
	std::cerr << BOLD("STAMINA encountered the following error and will now exit: ") << std::endl;
	std::cerr << '\t' << err << std::endl;
#ifdef STAMINA_HAS_GUI
	if (!functionsSetup) { exit(err_num); }
	criticalCallback(err);
	return;
#else
	if (raiseExceptionsRatherThanExit) {
// #ifdef STAMINA_HAS_GUI
// 		KMessageBox::sorry(nullptr, QString::fromStdString(err));
// #endif // STAMINA_HAS_GUI
		throw err; // TODO: throw some kind of exception rather than string
	}
	else {
		exit(err_num);
	}
#endif
}

void
StaminaMessages::error(std::string err, uint8_t err_num) {
	if (Options::quiet) {
		return;
	}
	std::cerr << BOLD(FRED("[ERROR]: "));
	std::cerr << BOLD("STAMINA encountered the following (possibly recoverable) error: ") << std::endl;
	std::cerr << '\t' << err << std::endl;
#ifdef STAMINA_HAS_GUI
	if (!functionsSetup) { return; }
	errCallback(err);
#endif
}

void
StaminaMessages::warning(std::string warn) {
	if (Options::quiet) {
		return;
	}
	std::cerr << BOLD(FYEL("[WARNING]: ")) << warn << std::endl;
#ifdef STAMINA_HAS_GUI
	if (!functionsSetup) { return; }
	warnCallback(warn);
#endif
}

void
StaminaMessages::info(std::string info) {
	if (Options::quiet) {
		return;
	}
	std::cerr << BOLD(FBLU("[INFO]: ")) << info << std::endl;
#ifdef STAMINA_HAS_GUI
	if (!functionsSetup) { return; }
	infoCallback(info);
#endif
}

void
StaminaMessages::good(std::string good) {
	if (Options::quiet) {
		return;
	}
	std::cerr << BOLD(FGRN("[MESSAGE]: ")) << good << std::endl;
#ifdef STAMINA_HAS_GUI
	if (!functionsSetup) { return; }
	goodCallback(good);
#endif
}

#ifdef DEBUG_PRINTS
void StaminaMessages::debugPrint(std::string msg) {
	if (Options::quiet) {
		return;
	}
	std::cout << BOLD(FMAG("[DEBUG MESSAGE]: ")) << msg << std::endl;
}
#endif

void
StaminaMessages::writeResults(ResultInformation resultInformation, std::ostream & out) {
	out.setf( std:: ios::floatfield );
	out << std::fixed << std::setprecision(12);
	out << horizontalSeparator << std::endl;
	out << "RESULTS" << std::endl << horizontalSeparator << std::endl;
	out << "Property: " << resultInformation.property << std::endl;
	out << "Probability Minimum: " << resultInformation.pMin << std::endl;
	out << "Probability Maximum: " << resultInformation.pMax << std::endl;
	out << "Window: " << (resultInformation.pMax - resultInformation.pMin) << std::endl;
	out << horizontalSeparator << std::endl;
	out << "Model: " << resultInformation.numberStates << " states with " << resultInformation.numberInitial << " initial." << std::endl;
	out << horizontalSeparator << std::endl;
}

} // namespace core
} // namespace stamina
