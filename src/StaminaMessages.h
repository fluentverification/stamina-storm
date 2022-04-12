#ifndef STAMINA_MESSAGES_H
#define STAMINA_MESSAGES_H

#include <string>
#include <stdint.h>
#include <fstream>
#include <iostream>

// #define DEBUG_PRINTS
// #define DEBUG_PRINTS_VERBOSE

#ifdef DEBUG_PRINTS_VERBOSE
	#define DEBUG_PRINTS
#endif

namespace stamina {
	/* ERRORS WE CAN GET */
    enum STAMINA_ERRORS {
        ERR_GENERAL = 1
        , ERR_SEVERE = 2
        , ERR_MEMORY_EXCEEDED = 137
    };
	/* All result information */
	struct ResultInformation {
		double pMin;
		double pMax;
		uint32_t numberStates;
		uint8_t numberInitial;
		std::string property;
	};
	class StaminaMessages {
	public:
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
		static void writeResults(ResultInformation resultInformation, std::ostream out);
	protected:
		static constexpr char * horizontalSeparator =
			"========================================================================================";
	};
} // namespace stamina
#endif // STAMINA_MESSAGES_H
