#ifndef STAMINA_MESSAGES_H
#define STAMINA_MESSAGES_H

namespace stamina {
	class StaminaMessages {
		/**
		* Errors and exits program
		* */
		static void errorAndExit(std::string err, uint8_t err_num = STAMINA_ERRORS::ERR_GENERAL);
		/**
		* Errors without exiting
		* */
		static void error(std::string err, uint8_t err_num = STAMINA_ERRORS::ERR_GENERAL);
		/**
		* Prints a warning
		* */
		static void warning(std::string warn);
		/**
		* Prints an info message
		* */
		static void info(std::string info);
		/**
		* Prints a (good) message (i.e., we finished)
		* */
		static void good(std::string good);
#ifdef DEBUG_PRINTS
		/**
		* Prints a debugging message
		* */
		static void debugPrint(std::string msg);
#endif
	};
} // namespace stamina
#endif // STAMINA_MESSAGES_H
