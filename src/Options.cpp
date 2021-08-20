#include "Options.h"
#include "Stamina.h"

using namespace stamina;
// IMPLEMENTATION FOR Stamina::Stamina::Options

Options::Options() {
    // Intentionally left empty
}

Options::Options(struct arguments * arguments) :
   model_file(arguments->model_file)
   , properties_file(arguments->properties_file)
   , kappa(arguments->kappa)
   , reduce_kappa(arguments->reduce_kappa)
   , approx_factor(arguments->approx_factor)
   , prob_win(arguments->prob_win)
   , max_approx_count(arguments->max_approx_count)
   , no_prop_refine(arguments->no_prop_refine)
   , cudd_max_mem(arguments->cudd_max_mem)
   , export_filename(arguments->export_filename)
   , export_perimeter_states(arguments->export_perimeter_states)
   , import_filename(arguments->import_filename)
   , property(arguments->property)
   , consts(arguments->consts)
   , export_trans(arguments->export_trans)
   , rank_transitions(arguments->rank_transitions)
   , max_iterations(arguments->max_iterations)
   , power(arguments->power)
   , jacobi(arguments->jacobi)
   , gauss_seidel(arguments->gauss_seidel)
   , backward_gauss_seidel(arguments->backward_gauss_seidel)
{
    // Intentionally left empty
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
    this->model_file = arguments->model_file;
    this->properties_file = arguments->properties_file;
    this->kappa = arguments->kappa;
    this->reduce_kappa = arguments->reduce_kappa;
    this->approx_factor = arguments->approx_factor;
    this->prob_win = arguments->prob_win;
    this->max_approx_count = arguments->max_approx_count;
    this->no_prop_refine = arguments->no_prop_refine;
    this->cudd_max_mem = arguments->cudd_max_mem;
    this->export_filename = arguments->export_filename;
    this->export_perimeter_states = arguments->export_perimeter_states;
    this->import_filename = arguments->import_filename;
    this->property = arguments->property;
    this->consts = arguments->consts;
    this->export_trans = arguments->export_trans;
    this->rank_transitions = arguments->rank_transitions;
    this->max_iterations = arguments->max_iterations;
    this->power = arguments->power;
    this->jacobi = arguments->jacobi;
    this->gauss_seidel = arguments->gauss_seidel;
    this->backward_gauss_seidel = arguments->backward_gauss_seidel;
}