#ifndef STAMINA_CORE_STAMINAMESSAGES_H
#define STAMINA_CORE_STAMINAMESSAGES_H

#include <string>
#include <stdint.h>
#include <fstream>
#include <iostream>

#define DEBUG_PRINTS
// #define DEBUG_PRINTS_VERBOSE

#ifdef DEBUG_PRINTS_VERBOSE
	#define DEBUG_PRINTS
#endif

// Efficient debug printing method to std::cout that can be easily turned off without having to get
// rid of all instances of StaminaMessages::debugPrint("Some message") or whatever
#ifdef DEBUG_PRINTS
	#define STAMINA_DEBUG_MESSAGE(x); std::cout << x << std::endl;
#else
	#define STAMINA_DEBUG_MESSAGE(x);
#endif // DEBUG_PRINTS

namespace stamina {
	namespace core {
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
			 * Prints a nice init message.
			 * */
			static void initMessage();
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
			static const std::string horizontalSeparator;
		};
	} // namespace core
	typedef core::StaminaMessages StaminaMessages; // Allow users to use messages without the `core` namespace
} // namespace stamina

/* Logging and debugging messages */
#ifdef DEBUG_PRINTS
	#define STAMINA_DEBUG_LOG(x)
#else

#endif // DEBUG_PRINTS
#endif // STAMINA_CORE_STAMINAMESSAGES_H
