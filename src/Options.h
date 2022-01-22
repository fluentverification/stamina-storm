#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>
#include <string>
#include "StaminaArgParse.h"
#include <functional>

namespace stamina {

    /**
     * Options
     * */
    class Options {
    public:

        /**
         * Creates an Options struct with command line arguments
         * 
         * @param arguments Command-line arguments to pass in
         * */
        static void createFromArguments(struct arguments * arguments);
        /**
         * Checks validity of options
         * 
         * @param errFunc
         * @return Whether all options are good
         * */
        static bool checkOptions();
        /**
         * Sets the values of these options again
         * 
         * @param arguments Command-line arguments to pass in
         * */
        static void setArgs(struct arguments * arguments);
        static std::string model_file;
        static std::string properties_file;
        static double kappa;
        static double reduce_kappa;
        static double approx_factor; // Misprediction factor
        static double prob_win;
        static uint64_t max_approx_count;
        static bool no_prop_refine;
        static std::string cudd_max_mem;
        static std::string export_filename;
        static std::string export_perimeter_states;
        static std::string import_filename;
        static std::string property;
        static std::string consts;
        static std::string export_trans;
        static bool rank_transitions;
        static uint64_t max_iterations;
        static bool power;
        static bool jacobi;
        static bool gauss_seidel;
        static bool backward_gauss_seidel;
    };
    /**
     * Tells us if a string ends with another
     * 
     * @param full Full string to check
     * @param end Ending of the string
     * */
    bool endsWith(std::string full, std::string end);
}
#endif // OPTIONS_H
