#include "Options.h"
#include "Stamina.h"

using namespace stamina;
// IMPLEMENTATION FOR Stamina::Stamina::Options


Options::createFromArguments(struct arguments * arguments)
{

Options::model_file = arguments->model_file;
Options::properties_file = arguments->properties_file;
Options::kappa = arguments->kappa;
Options::reduce_kappa = arguments->reduce_kappa;
Options::approx_factor = arguments->approx_factor;
Options::prob_win = arguments->prob_win;
Options::max_approx_count = arguments->max_approx_count;
Options::no_prop_refine = arguments->no_prop_refine;
Options::cudd_max_mem = arguments->cudd_max_mem;
Options::export_filename = arguments->export_filename;
Options::export_perimeter_states = arguments->export_perimeter_states;
Options::import_filename = arguments->import_filename;
Options::property = arguments->property;
Options::consts = arguments->consts;
Options::export_trans = arguments->export_trans;
Options::rank_transitions = arguments->rank_transitions;
Options::max_iterations = arguments->max_iterations;
Options::power = arguments->power;
Options::jacobi = arguments->jacobi;
Options::gauss_seidel = arguments->gauss_seidel;
Options::backward_gauss_seidel = arguments->backward_gauss_seidel;
}

bool 
Options::checkOptions(std::function<void(std::string, uint8_t)> err, std::function<void(std::string)> warn) {
    bool good = true;
    // Check if we are passing in the right type of file
    if (model_file == "") {
        err("Model file not provided.", STAMINA_ERRORS::ERR_GENERAL);
        return false; // Return since no recovering from this.
    }
    else if (!endsWith(model_file, ".prism") && !endsWith(model_file, ".sm")) {
        warn("Model file does not appear to have the proper extension (.sm or .prism)");
    }
    // Check if properties file is the right type of file
    if (properties_file == "") {
        err("Properties file not provided.", STAMINA_ERRORS::ERR_GENERAL);
        return false; // Return since no recovering from this
    }
    else if (!endsWith(properties_file, ".csl")) {
        warn("Properties file does not appear to have the proper extension (.csl)");
    }
    // Check if kappa is greater than 0
    if (kappa < 0) {
        err("Kappa should be greater than or equal to 0.0. Got: " + std::to_string(kappa), STAMINA_ERRORS::ERR_GENERAL);
        good = false;
    }
    // Check if kappa reduction factor is greater than 1.0
    if (reduce_kappa < 1.0) {
        err("Kappa reduction factorshould be greater than or equal to 1.0. Got: " + std::to_string(reduce_kappa), STAMINA_ERRORS::ERR_GENERAL);
        good = false;
    }
    // Make sure max approx count factor is greater than zero
    if (max_approx_count < 0) {
        err("Max approx count should be greater than 0.0. Got: " + std::to_string(max_approx_count), STAMINA_ERRORS::ERR_GENERAL);
        good = false;
    }
    return good;
}

void 
Options::setArgs(struct arguments * arguments) {
    Options::model_file = arguments->model_file;
    Options::properties_file = arguments->properties_file;
    Options::kappa = arguments->kappa;
    Options::reduce_kappa = arguments->reduce_kappa;
    Options::approx_factor = arguments->approx_factor;
    Options::prob_win = arguments->prob_win;
    Options::max_approx_count = arguments->max_approx_count;
    Options::no_prop_refine = arguments->no_prop_refine;
    Options::cudd_max_mem = arguments->cudd_max_mem;
    Options::export_filename = arguments->export_filename;
    Options::export_perimeter_states = arguments->export_perimeter_states;
    Options::import_filename = arguments->import_filename;
    Options::property = arguments->property;
    Options::consts = arguments->consts;
    Options::export_trans = arguments->export_trans;
    Options::rank_transitions = arguments->rank_transitions;
    Options::max_iterations = arguments->max_iterations;
    Options::power = arguments->power;
    Options::jacobi = arguments->jacobi;
    Options::gauss_seidel = arguments->gauss_seidel;
    Options::backward_gauss_seidel = arguments->backward_gauss_seidel;
}
