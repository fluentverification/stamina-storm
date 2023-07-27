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

#ifdef STAMINA_HAS_GUI
#include <functional>
#endif // STAMINA_HAS_GUI

namespace stamina {
	namespace core {
		/* ERRORS WE CAN GET */
		enum STAMINA_ERRORS {
			ERR_GENERAL = 1
			, ERR_SEVERE = 2
			, ERR_MEMORY_EXCEEDED = 137
		};
		/* All result information */
		class ResultInformation {
		public:
			double pMin;
			double pMax;
			uint32_t numberStates;
			uint32_t numberInitial;
			std::string property;
			ResultInformation(
				double pMin
				, double pMax
				, uint32_t numberStates
				, uint32_t numberInitial
				, std::string property
			) : pMin(pMin)
				, pMax(pMax)
				, numberStates(numberStates)
				, numberInitial(numberInitial)
				, property(property)
			{}
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
			static void writeResults(ResultInformation resultInformation, std::ostream & out);
			// The GUI needs us to raise exceptions because the exit() function will kill
			// the entire program, which is usually fine in the CLI.
			inline static bool raiseExceptionsRatherThanExit = false;
#ifdef STAMINA_HAS_GUI
			// Since std::functions have a default constructor, we can leave these
			inline static bool functionsSetup = false;
			inline static std::function<void (std::string)> errCallback;
			inline static std::function<void (std::string)> warnCallback;
			inline static std::function<void (std::string)> infoCallback;
			inline static std::function<void (std::string)> goodCallback;
			inline static std::function<void (std::string)> criticalCallback;
#endif // STAMINA_HAS_GUI
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
