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

#include "Options.h"

#include "StaminaMessages.h"

namespace stamina {
namespace core {

bool
Options::checkOptions() {
	bool good = true;
	// Check if we are passing in the right type of file
	if (model_file == "") {
		StaminaMessages::error("Model file not provided.", STAMINA_ERRORS::ERR_GENERAL);
		return false; // Return since no recovering from this.
	}
	else if (!endsWith(model_file, ".prism") && !endsWith(model_file, ".sm")) {
		StaminaMessages::warning("Model file does not appear to have the proper extension (.sm or .prism)");
	}
	// Check if properties file is the right type of file
	if (properties_file == "") {
		StaminaMessages::error("Properties file not provided.", STAMINA_ERRORS::ERR_GENERAL);
		return false; // Return since no recovering from this
	}
	else if (!endsWith(properties_file, ".csl")) {
		StaminaMessages::warning("Properties file does not appear to have the proper extension (.csl)");
	}
	// Check if kappa is greater than 0
	if (kappa < 0) {
		StaminaMessages::error("Kappa should be greater than or equal to 0.0. Got: " + std::to_string(kappa), STAMINA_ERRORS::ERR_GENERAL);
		good = false;
	}
	// Check if kappa reduction factor is greater than 1.0
	if (reduce_kappa < 1.0) {
		StaminaMessages::error("Kappa reduction factorshould be greater than or equal to 1.0. Got: " + std::to_string(reduce_kappa), STAMINA_ERRORS::ERR_GENERAL);
		good = false;
	}
	// Make sure max approx count factor is greater than zero
	if (max_approx_count < 0) {
		StaminaMessages::error("Max approx count should be greater than 0.0. Got: " + std::to_string(max_approx_count), STAMINA_ERRORS::ERR_GENERAL);
		good = false;
	}
	if (threads == 0) {
		StaminaMessages::error("Thread-count cannot be 0!");
		good = false;
	}
	return good;
}

void
Options::setArgs(struct arguments * arguments) {
	model_file = arguments->model_file;
	properties_file = arguments->properties_file;
	kappa = arguments->kappa;
	reduce_kappa = arguments->reduce_kappa;
	approx_factor = arguments->approx_factor;
	prob_win = arguments->prob_win;
	max_approx_count = arguments->max_approx_count;
	no_prop_refine = arguments->no_prop_refine;
	cudd_max_mem = arguments->cudd_max_mem;
	export_filename = arguments->export_filename;
	export_perimeter_states = arguments->export_perimeter_states;
	import_filename = arguments->import_filename;
	property = arguments->property;
	consts = arguments->consts;
	export_trans = arguments->export_trans;
	rank_transitions = arguments->rank_transitions;
	max_iterations = arguments->max_iterations;
	max_states = arguments->max_states;
	method = arguments->method;
	threads = arguments->threads;
	preterminate = arguments->preterminate;
	fudge_factor = arguments->fudge_factor;
	event = arguments->event;
	distance_weight = arguments->distance_weight;
	quiet = arguments->quiet;
}

} // namespace core
} // namespace stamina
