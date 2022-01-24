#include "StaminaMessages.h"

#include "ANSIColors.h"

#include <stdlib.h>
#include <iomanip>
#include <stdlib.h>
#include <string_view>
#include <sstream>
#include <iostream>

using namespace stamina;

void
StaminaMessages::errorAndExit(std::string err, uint8_t err_num) {
	std::cerr << BOLD(FRED("[ERROR]: "));
	std::cerr << BOLD("STAMINA encountered the following error and will now exit: ") << std::endl;
	std::cerr << '\t' << err << std::endl;
	exit(err_num);
}

void
StaminaMessages::error(std::string err, uint8_t err_num) {
	std::cerr << BOLD(FRED("[ERROR]: "));
	std::cerr << BOLD("STAMINA encountered the following (possibly recoverable) error: ") << std::endl;
	std::cerr << '\t' << err << std::endl;
}

void
StaminaMessages::warning(std::string warn) {
	std::cerr << BOLD(FYEL("[WARNING]: ")) << warn << std::endl;
}

void
StaminaMessages::info(std::string info) {
	std::cerr << BOLD(FBLU("[INFO]: ")) << info << std::endl;
}

void
StaminaMessages::good(std::string good) {
	std::cerr << BOLD(FGRN("[MESSAGE]: ")) << good << std::endl;
}

#ifdef DEBUG_PRINTS
void StaminaMessages::debugPrint(std::string msg) {
	std::cout << BOLD(FMAG("[DEBUG MESSAGE]: ")) << msg << std::endl;
}
#endif
