#ifndef STAMINA_STAMINA
#define STAMINA_STAMINA

#include <iostream>
#include <ostream>
#include <functional>

#include "StaminaArgParse.h"
#include "core/StaminaModelChecker.h"
#include "core/Options.h"

#define VERSION_MAJOR 2
#define VERSION_MINOR 2.1

#include <storm/api/storm.h>
#include <storm-parsers/api/storm-parsers.h>
#include <storm-parsers/parser/PrismParser.h>
#include <storm/storage/prism/Program.h>
#include <storm/storage/jani/Property.h>
#include <storm/modelchecker/results/CheckResult.h>
#include <storm/modelchecker/results/ExplicitQuantitativeCheckResult.h>

// #include <storm/utility/initialize.h>

namespace stamina {

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
        /* Data Members */
        /**
         * Initializes Stamina
         * */
        void initialize();

        /* Data Members */
        std::shared_ptr<core::StaminaModelChecker> modelChecker;
        std::shared_ptr<storm::prism::Program> modelFile;
        std::shared_ptr<std::vector<storm::jani::Property>> propertiesVector;
    };
}

#endif // STAMINA_STAMINA
