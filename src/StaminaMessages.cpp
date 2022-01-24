#include "StaminaMessages.h"

#include "ANSIColors.h"

#include <stdlib.h>
#include <iomanip>
#include <stdlib.h>
#include <string_view>
#include <sstream>

using namespace stamina;

void
StaminaMessages::errorAndExit(std::string err, uint8_t err_num) {
	std::err << BOLD(FRED("[ERROR]: "));
	std::err << BOLD("STAMINA encountered the following error and will now exit: ") << std::endl;
	std::err << '\t' << err << std::endl;
	exit(err_num);
}

void
StaminaMessages::error(std::string err, uint8_t err_num) {
	std::err << BOLD(FRED("[ERROR]: "));
	std::err << BOLD("STAMINA encountered the following (possibly recoverable) error: ") << std::endl;
	std::err << '\t' << err << std::endl;
}

void
StaminaMessages::warning(std::string warn) {
	std::err << BOLD(FYEL("[WARNING]: ")) << warn << std::endl;
}

void
StaminaMessages::info(std::string info) {
	std::err << BOLD(FBLU("[INFO]: ")) << info << std::endl;
}

void
StaminaMessages::good(std::string good) {
	std::err << BOLD(FGRN("[MESSAGE]: ")) << good << std::endl;
}

#ifdef DEBUG_PRINTS
void StaminaMessages::debugPrint(std::string msg) {
	std::cout << BOLD(FMAG("[DEBUG MESSAGE]: ")) << msg << std::endl;
}
#endif
