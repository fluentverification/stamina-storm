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
         * Allows you to create an Options class with nothing in them
         * */
        Options();
        /**
         * Creates an Options struct with command line arguments
         * 
         * @param arguments Command-line arguments to pass in
         * */
        Options(struct arguments * arguments);
        /**
         * Checks validity of options
         * 
         * @param errFunc
         * @return Whether all options are good
         * */
        bool checkOptions(
            std::function<void(std::string, uint8_t)> err
            , std::function<void(std::string)> warn
        );
        /**
         * Sets the values of these options again
         * 
         * @param arguments Command-line arguments to pass in
         * */
        void setArgs(struct arguments * arguments);
        std::string model_file;
        std::string properties_file;
        double kappa;
        double reduce_kappa;
        double approx_factor; // Misprediction factor
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
     * Tells us if a string ends with another
     * 
     * @param full Full string to check
     * @param end Ending of the string
     * */
    bool endsWith(std::string full, std::string end);
}
#endif // OPTIONS_H