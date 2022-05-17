#include "Options.h"

#include "StaminaMessages.h"

using namespace stamina;
// IMPLEMENTATION FOR Stamina::Stamina::Options

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
}
