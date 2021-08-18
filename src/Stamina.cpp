/**
 * Method implementations for Stamina
 * 
 * Created on 8/17/2021 by Josh Jeppson
 * */
#include "Stamina.h"
#include "ANSIColors.h"

#include <stdlib.h>
#include <iomanip>
#include <stdlib.h>
#include <string_view>
#include <sstream>

using namespace stamina;

/* ===== IMPLEMENTATION FOR `Stamina::Stamina` Methods ===== */

// PUBLIC METHODS
Stamina::Stamina(struct arguments * arguments) {
    try {
        this->options = new Stamina::Options();
        this->options->setArgs(arguments);
    }
    catch (const std::exception& e) {
        errorAndExit("Failed to allocate Stamina::Stamina::options: " + std::string(e.what()));
    }
    info("Starting STAMINA with kappa = " + std::to_string(options->kappa) + " and reduction factor = " + std::to_string(options->reduce_kappa));
    // Pass in a lambda (bound function) to the checkOptions method
    bool good = options->checkOptions(
        std::bind(&Stamina::error, this, std::placeholders::_1, std::placeholders::_2)
        , std::bind(&Stamina::warning, this, std::placeholders::_1)
    );
    if (!good) {
        errorAndExit("One or more parameters passed in were invalid.");
    }
}

Stamina::~Stamina() {
    delete this->options;
}

void Stamina::run() {
    initialize();
    // Check each property in turn
    for (auto property : propertiesVector) {
#ifdef DEBUG_PRINTS
        debugPrint("Checking property in properties vector.");
#endif
        auto result = modelChecker->modelCheckProperty(property, modulesFile);
    }
    // Finished!
    good("Finished running!");
}

// PRIVATE METHODS

void Stamina::initialize() {
    info("Stamina version is: " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR));
    // Initialize with references to error, warning, info, and good message functions
    modelChecker = new StaminaModelChecker(
        std::bind(&Stamina::error, this, std::placeholders::_1, STAMINA_ERRORS::ERR_GENERAL)
        , std::bind(&Stamina::warning, this, std::placeholders::_1)
        , std::bind(&Stamina::info, this, std::placeholders::_1)
        , std::bind(&Stamina::good, this, std::placeholders::_1)
    );
    // Initialize loggers
    // storm::utility::setUp(); // TODO
    // Set some settings objects.
    storm::settings::initializeAll("Stamina", "Stamina");

    // Load modules file and properties file
    try {
        modulesFile = storm::parser::PrismParser::parse(options->model_file, true);
        propertiesVector = storm::api::parsePropertiesForPrismProgram(options->properties_file, modulesFile);
    }
    catch (const std::exception& e) {
        // Uses stringstream because std::to_string(e) throws an error with storm's exceptions
        std::stringstream msg;
        msg << "Got error when reading modules or properties file:\n\t\t" << e.what();
        errorAndExit(msg.str());
    }
    
}

void Stamina::errorAndExit(std::string err, uint8_t err_num) {
    std::cerr << BOLD(FRED("[ERROR]: "));
    std::cerr << BOLD("STAMINA encountered the following error and will now exit: ") << std::endl;
    std::cerr << '\t' << err << std::endl;
    exit(err_num);
}

void Stamina::error(std::string err, uint8_t err_num) {
    std::cerr << BOLD(FRED("[ERROR]: "));
    std::cerr << BOLD("STAMINA encountered the following (possibly recoverable) error: ") << std::endl;
    std::cerr << '\t' << err << std::endl;
}

void Stamina::warning(std::string warn) {
    std::cerr << BOLD(FYEL("[WARNING]: ")) << warn << std::endl;
}

void Stamina::info(std::string info) {
    std::cerr << BOLD(FBLU("[INFO]: ")) << info << std::endl;
}

void Stamina::good(std::string good) {
    std::cerr << BOLD(FGRN("[MESSAGE]: ")) << good << std::endl;
}

#ifdef DEBUG_PRINTS
void Stamina::debugPrint(std::string msg) {
    std::cout << BOLD(FMAG("[DEBUG MESSAGE]: ")) << msg << std::endl;
}
#endif
/* ===== IMPLEMENTATION FOR `Stamina::Stamina::*` (embedded class) Methods ===== */

// IMPLEMENTATION FOR Stamina::Stamina::Options

Stamina::Options::Options() {
    // Intentionally left empty
}

Stamina::Options::Options(struct arguments * arguments) :
   model_file(model_file)
   , properties_file(properties_file)
   , kappa(kappa)
   , reduce_kappa(reduce_kappa)
   , approx_factor(approx_factor)
   , prob_win(prob_win)
   , max_approx_count(max_approx_count)
   , no_prop_refine(no_prop_refine)
   , cudd_max_mem(cudd_max_mem)
   , export_filename(export_filename)
   , export_perimeter_states(export_perimeter_states)
   , import_filename(import_filename)
   , property(property)
   , consts(consts)
   , expor_trans(expor_trans)
   , rank_transitions(rank_transitions)
   , max_iterations(max_iterations)
   , power(power)
   , jacobi(jacobi)
   , gauss_seidel(gauss_seidel)
   , backward_gauss_seidel(backward_gauss_seidel)
{
    // Intentionally left empty
}

bool Stamina::Options::checkOptions(std::function<void(std::string, uint8_t)> err, std::function<void(std::string)> warn) {
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

void Stamina::Options::setArgs(struct arguments * arguments) {
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
    this->expor_trans = arguments->expor_trans;
    this->rank_transitions = arguments->rank_transitions;
    this->max_iterations = arguments->max_iterations;
    this->power = arguments->power;
    this->jacobi = arguments->jacobi;
    this->gauss_seidel = arguments->gauss_seidel;
    this->backward_gauss_seidel = arguments->backward_gauss_seidel;
}

/* ===== IMPLEMENTATION FOR OTHER CLASSES IN THE `stamina` NAMESPACE ===== */

bool stamina::endsWith(std::string full, std::string end) {
    int i = end.size();
    int j = full.size();
    if (i > j) {
        return false;
    }
    while (i > 0) {
        if (full[j] != end[i]) {
            return false;
        }
        i--;
        j--;
    }
    return true;
}