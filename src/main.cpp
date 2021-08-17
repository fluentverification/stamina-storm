#include "StaminaArgParse.h"
#include "Stamina.h"

/**
 * Sets our default values
 * */
void set_default_values(struct arguments * arguments) {
    arguments->kappa = 1.0;
    arguments->reduce_kappa = 2.0;
    arguments->approx_factor = 2.0;
    arguments->prob_win = 1.0e-3;
    arguments->max_approx_count = 10;
    arguments->no_prop_refine = false;
    arguments->cudd_max_mem = "1g";
    arguments->expor_trans = "trans.txt";
    arguments->rank_transitions = false;
    arguments->max_iterations = 10000;
    arguments->power = false;
    arguments->jacobi = false;
    arguments->gauss_seidel = false;
    arguments->backward_gauss_seidel = false;
}

/**
 * Main function
 * */
int main(int argc, char ** argv) {
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