/**
 * Argument Parser for STAMINA using a lot of C-style stuff and argp
 * 
 * Created on 8/17/2021 by Josh Jeppson
 * */
#ifndef STAMINA_ARG_PARSE_H
#define STAMINA_ARG_PARSE_H

#include <iostream>
#include <fstream>
#include <string>
#include <argp.h>
#include <stdlib.h>



static char doc[] = "STAMINA -- truncates infinite CTMC state space and passes into STORM";

static char args_doc[] = "MODEL_FILE PROPERTIES_FILE";

/**
 * Options understood by stamina, as well as documentation for those arguments
 * */
static struct argp_option options[] = {
    {"kappa", 'k', "double", 0, 
        "Reachability threshold for the first iteration (default: 1.0)"}
    , {"reduceKappa", 'r', "double", 0, 
        "Reduction factor for Reachability Threshold (kappa) during the refinement step (default 2.0)"}
    , {"approxFactor", 'f', "double", 0, 
        "Factor to estimate how far off our reachability predictions will be (default: 2.0)"}
    , {"probWin", 'w', "double", 0,
        "Probability window between lower and upperbound for termination (default: 1.0e-3)"}
    , {"maxApproxCount", 'n', "int", 0,
        "Maximum number of iterations in the approximation (default 10)"}
    , {"noPropRefine", 'R', 0, 0,
        "Do not use property based refinement. If given, the model exploration method will reduce kappa and do property independent definement (default: off)"}
    , {"cuddMaxMem", 'C', "memory", 0,
        "Maximum CUDD memory, in the same format as PRISM (default: 1g)"}
    , {"export", 'e', "filename", 0,
        "Export model to a (text) file"}
    , {"exportPerimeterStates", 'S', "filename", 0,
        "Export perimeter states to a file. Please provide a filename. This will append to the file if it is existing"}
    , {"import", 'i', "filename", 0,
        "Import model to a (text) file"}
    , {"property", 'p', "propname", 0,
        "Specify a certain property to check in a model file that contains many"}
    , {"const", 'c', "\"C1=VAL,C2=VAL,C3=VAL\"", 0,
        "Comma separated values for constants"}
    , {"exportTrans", 't', "filename", 0,
        "Export the list of transitions and actions to a specified file name, or to trans.txt if no file name is specified.\nTransitions are exported in the format <Source State Index> <Destination State Index> <Action Label>"}
    /* Additional options. GNU argp shows args alphabetically */
    , {"rankTransitions", 'T', 0, 0,
        "Rank transitions before expanding (default: false)"}
    , {"maxIterations", 'M', "int", 0,
        "Maximum iteration for solution (default: 10000)"}
    , {"power", 'P', 0, 0,
        "Use the \"Power Method\""}
    , {"jacobi", 'j', 0, 0,
        "Use the \"Jacobi Method\""}
    , {"gaussSeidel", 'g', 0, 0,
        "Use the \"Gauss-Seidel Method\""}
    , {"bGaussSeidel", 'G', 0, 0,
        "Use the backwards \"Gauss-Seidel Method\""}
    , { 0 }
};

/**
 * Arguments that we have
 * */
struct arguments {
    std::string model_file;
    std::string properties_file;
    double kappa;
    double reduce_kappa;
    double approx_factor; // Analagous to "misprediction factor" in the Java version
    double prob_win;
    uint64_t max_approx_count;
    bool no_prop_refine;
    std::string cudd_max_mem;
    std::string export_filename;
    std::string export_perimeter_states;
    std::string import_filename;
    std::string property;
    std::string consts;
    std::string export_trans;
    bool rank_transitions;
    uint64_t max_iterations;
    bool power;
    bool jacobi;
    bool gauss_seidel;
    bool backward_gauss_seidel;
};

/**
 * Parse a single option
 * */
static error_t 
parse_opt(int key, char * arg, struct argp_state * state) {
    
    struct arguments * arguments = static_cast<struct arguments*>(state->input);
    switch (key) {
        // kappa value
        case 'k':
            arguments->kappa = (double) atof(arg);
            break;
        // kappa reduction factor
        case 'r':
            arguments->reduce_kappa = (double) atof(arg);
            break;
        // approx factor
        case 'f':
            arguments->approx_factor = (double) atof(arg);
            break;
        // probability window (difference between Pmin and Pmax)
        case 'w':
            arguments->prob_win = (double) atof(arg);
            break;
        // max number of iterations in approximation
        case 'n':
            arguments->max_approx_count = (uint64_t) atoi(arg);
            break;
        // whether or not to use property based refinement
        case 'R':
            arguments->no_prop_refine = true;
            break;
        // cudd max memory limit
        case 'C':
            arguments->cudd_max_mem = std::string(arg);
            break;
        // filename to export to
        case 'e':
            arguments->export_filename = std::string(arg);
            break;
        // whether or not to export the perimeter states
        case 'S':
            arguments->export_perimeter_states = true;
            break;
        // import model filename
        case 'i':
            arguments->import_filename = std::string(arg);
            break;
        // specific property to check
        case 'p':
            arguments->property = std::string(arg);
            break;
        // constants to specify
        case 'c':
            arguments->consts = std::string(arg);
            break;
        // export transitions
        case 't':
            arguments->export_trans = std::string(arg);
            break;
        // use rank transitions before expanding
        case 'T':
            arguments->rank_transitions = true;
            break;
        // max number of iterations
        case 'M':
            arguments->max_iterations = (uint64_t) atoi(arg);
            break;
        // use the power method
        case 'P':
            arguments->power = true;
            break;
        // use the jacobi method
        case 'j':
            arguments->jacobi = true;
            break;
        // use the gauss-seidel method
        case 'g':
            arguments->gauss_seidel = true;
            break;
        // backwards gauss seidel method
        case 'G':
            arguments->backward_gauss_seidel = true;
            break;
        // model and properties file
        case ARGP_KEY_ARG:
            // get model file
            if (state->arg_num == 0) {
                arguments->model_file = std::string(arg);
            }
            // get properties file
            else if (state->arg_num == 1) {
                arguments->properties_file = std::string(arg);
            }
            // we have too many arguments
            else {
                argp_usage(state);
            }
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 2) {
                /* Require at least 2 arguments */
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/**
 * Argument parser
 * */
static struct argp argp = {options, parse_opt, args_doc, doc};

void set_default_values(struct arguments * arguments);

#endif // STAMINA_ARG_PARSE_H