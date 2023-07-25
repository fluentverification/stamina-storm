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

#ifndef STAMINA_CORE_OPTIONS_H
#define STAMINA_CORE_OPTIONS_H

#include <iostream>
#include <string>
#include "StaminaArgParse.h"
#include <functional>
#include <cstdint>

namespace stamina {
	namespace core {

		/**
		* Options
		* */
		class Options {
		public:

			/**
			* Checks validity of options
			*
			* @return Whether all options are good
			* */
			static bool checkOptions();
			/**
			* Sets the values of these options again
			*
			* @param arguments Command-line arguments to pass in
			* */
			static void setArgs(struct arguments * arguments);
			inline static std::string model_file;
			inline static std::string properties_file;
			inline static double kappa;
			inline static double reduce_kappa;
			inline static double approx_factor; // Misprediction factor
			inline static double fudge_factor; // Only applies to priority method
			inline static double prob_win;
			inline static uint64_t max_approx_count;
			inline static bool no_prop_refine;
			inline static std::string cudd_max_mem;
			inline static std::string export_filename;
			inline static std::string export_perimeter_states;
			inline static std::string import_filename;
			inline static std::string property;
			inline static std::string consts;
			inline static std::string export_trans;
			inline static bool rank_transitions;
			inline static uint64_t max_iterations;
			inline static uint64_t max_states;
			inline static uint8_t method;
			inline static uint8_t threads;
			inline static bool preterminate;
			inline static bool quiet;
			// Rare and common events
			inline static uint8_t event;
			inline static double distance_weight; // The weighting of the "distance" metric (a multiplier)
		};
		/**
		* Tells us if a string ends with another
		*
		* @param full Full string to check
		* @param end Ending of the string
		* */
		bool endsWith(std::string full, std::string end);
	} // namespace core
} // namespace stamina
#endif // STAMINA_CORE_OPTIONS_H
