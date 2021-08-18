#ifndef STAMINA
#define STAMINA

#define DEBUG_PRINTS

#include <iostream>
#include <ostream>
#include <functional>

#include "StaminaArgParse.h"
#include "StaminaModelChecker.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#include <storm/api/storm.h>
#include <storm-parsers/api/storm-parsers.h>
#include <storm-parsers/parser/PrismParser.h>
#include <storm/storage/prism/Program.h>
#include <storm/storage/jani/Property.h>
#include <storm/modelchecker/results/CheckResult.h>
#include <storm/modelchecker/results/ExplicitQuantitativeCheckResult.h>

// #include <storm/utility/initialize.h>

namespace stamina {
    /* ERRORS WE CAN GET */
    enum STAMINA_ERRORS {
        ERR_GENERAL = 1
        , ERR_SEVERE = 2
        , ERR_MEMORY_EXCEEDED = 137
    };
    /* MAIN STAMINA CLASS */
    class Stamina {
    public:
        /**
         * Main construtor: creates an instance of the STAMINA class
         * 
         * @param arguments Arguments struct from StaminaArgParse
         * */
        Stamina(struct arguments * arguments);
        /**
         * Destructor. Cleans up memory
         * */
        ~Stamina();
        /**
         * Runs stamina
         * */
        void run();
        /**
         * Options subclass
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
            double approx_factor;
            double prob_win;
            uint64_t max_approx_count;
            bool no_prop_refine;
            std::string cudd_max_mem;
            std::string export_filename;
            std::string export_perimeter_states;
            std::string import_filename;
            std::string property;
            std::string consts;
            std::string expor_trans;
            bool rank_transitions;
            uint64_t max_iterations;
            bool power;
            bool jacobi;
            bool gauss_seidel;
            bool backward_gauss_seidel;
        };
    private:
        /**
         * Initializes Stamina
         * */
        void initialize();
        /**
         * Errors and exits program
         * */
        void errorAndExit(std::string err, uint8_t err_num = STAMINA_ERRORS::ERR_GENERAL);
        /**
         * Errors without exiting
         * */
        void error(std::string err, uint8_t err_num = STAMINA_ERRORS::ERR_GENERAL);
        /**
         * Prints a warning
         * */
        void warning(std::string warn);
        /**
         * Prints an info message
         * */
        void info(std::string info);
        /**
         * Prints a (good) message (i.e., we finished)
         * */
        void good(std::string good);
#ifdef DEBUG_PRINTS
        /**
         * Prints a debugging message
         * */
        void debugPrint(std::string msg);
#endif
        /* Data Members */
        Stamina::Options * options;
        StaminaModelChecker * modelChecker;
        storm::prism::Program modulesFile;
        std::vector<storm::jani::Property> propertiesVector;
    };
    /**
     * Tells us if a string ends with another
     * 
     * @param full Full string to check
     * @param end Ending of the string
     * */
    bool endsWith(std::string full, std::string end);
}

#endif // STAMINA