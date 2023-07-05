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
