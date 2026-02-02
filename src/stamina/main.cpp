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

#include "StaminaArgParse.h"
#include "Stamina.h"

/**
 * Sets our default values
 * */
void
set_default_values(struct arguments * arguments) {
	arguments->kappa = 1.0;
	arguments->reduce_kappa = 1.25; // 2.0;
	arguments->approx_factor = 2.0;
	arguments->fudge_factor = 1.255;
	arguments->prob_win = 1.0e-3;
	arguments->max_approx_count = 10;
	arguments->no_prop_refine = false;
	arguments->cudd_max_mem = "1g";
	arguments->export_trans = "";
	arguments->rank_transitions = false;
	arguments->max_iterations = 10000;
	arguments->method = STAMINA_METHODS::ITERATIVE_METHOD;
	arguments->threads = 1;
	arguments->preterminate = false;
	arguments->event = EVENTS::UNDEFINED;
	arguments->distance_weight = 1.0;
	arguments->quiet = false;
}

/**
 * Main function
 * */
int
main(int argc, char ** argv) {
	struct arguments arguments;
	// Sets all default values
	set_default_values(&arguments);
	// Use the argp_parse parser to parse arguments
	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	// Instantiate STAMINA and run
	stamina::Stamina s(&arguments);
	s.run();
	return 0;
}
