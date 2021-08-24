#ifndef STAMINA_MODEL_CHECKER_H
#define STAMINA_MODEL_CHECKER_H

#include "Options.h"
#include "StaminaModelBuilder.h"

#include <sstream>
#include <string>

#include "storm/api/storm.h"
#include "storm-parsers/parser/PrismParser.h"
#include "storm-parsers/api/storm-parsers.h"
#include "storm/api/properties.h"
#include "storm/storage/jani/Property.h"
#include "storm/modelchecker/results/CheckResult.h"
#include "storm/modelchecker/results/ExplicitQuantitativeCheckResult.h"
#include "storm/utility/initialize.h"
#include "storm-parsers/api/properties.h"
#include "storm/models/sparse/Ctmc.h"
#include "storm/modelchecker/csl/SparseCtmcCslModelChecker.h"
#include "storm/modelchecker/csl/helper/SparseCtmcCslHelper.h"
#include "storm/storage/expressions/Expression.h"
#include "storm/storage/expressions/BaseExpression.h"
#include "storm/storage/expressions/BinaryExpression.h"
#include "storm/storage/expressions/BinaryRelationExpression.h"
#include "storm/storage/expressions/VariableExpression.h"
#include "storm/storage/expressions/ExpressionManager.h"
#include "storm/storage/expressions/UnaryBooleanFunctionExpression.h"
#include "storm/storage/expressions/BinaryBooleanFunctionExpression.h"
#include "storm/logic/Formula.h"
#include "storm/storage/prism/Constant.h"
#include "storm/settings/SettingsManager.h"
#include "storm/storage/expressions/Valuation.h"
#include "storm/environment/Environment.h"
#include "storm/modelchecker/results/CheckResult.h"
#include "storm/builder/BuilderOptions.h"
#include "storm/generator/VariableInformation.h"

namespace stamina {
    class StaminaModelChecker {
    public:
        /**
         * Constructor for StaminaModelChecker
         * 
         * @param err Lambda to error function
         * @param warm Lambda to warning function
         * @param info Lambda to info function
         * @param good Lambda to good function
         * @param modulesFile Pointer to the Modules file
         * @param propertiesVector Pointer to the Properties vector
         * */
        StaminaModelChecker(        
            std::function<void(std::string)> err
            , std::function<void(std::string)> warn
            , std::function<void(std::string)> info
            , std::function<void(std::string)> good
            , Options * options
            , storm::prism::Program * modulesFile = nullptr
            , std::vector<storm::jani::Property> * propertiesVector = nullptr
        );
        /**
         * Standard destructor
         * */
        ~StaminaModelChecker();
        /**
         * Initializes the StaminaModelChecker class
         * 
         * @param propertiesVector A vector to the list of JANI properties
         * */
        void initialize(
            storm::prism::Program * modulesFile = nullptr
            , std::vector<storm::jani::Property> * propertiesVector = nullptr
        );
        /**
         * Model checks a specific property
         * 
         * @param prop Property to check
         * @param modulesFile The modules file to work with
         * @return A pointer to the result of the model checking
         * */
        std::unique_ptr<storm::modelchecker::CheckResult> modelCheckProperty(
            storm::jani::Property prop
            , storm::prism::Program const& modulesFile
        );
    private:
        /**
         * Result subclass (no private members since is a private subclass)
         * */
        class Result {
        public:
            /**
             * Constructor. Defaults to 0.0 / ""
             * */
            Result() {
                result = 0.0;
                explanation = "";
            }
            /**
             * Gets string representation
             * */
            operator std::string() const { 
                std::stringstream str;
                str << result << " (" << explanation << ")";
                return str.str();
            }
            /**
             * Gets string representation
             * */
            friend std::ostream & operator<<(std::ostream & stream, StaminaModelChecker::Result const & r) {
                stream << r.result << " (" << r.explanation << ")";
                return stream;
            }

            double result;
            std::string explanation;

        };
        /**
         * Explicitly invokes a model check from a property
         * 
         * @param property Property to invoke model checker for
         * @param r Pointer to result
         * */
        void check(storm::jani::Property * property, Result * r);
        /**
         * Whether or not to terminate model check
         * 
         * @return Terminate?
         * */
        bool terminateModelCheck();
        /**
         * Writes perimeter states to a specified file.
         * */
        void writePerimeterStates(int numRefineIteration);
        /**
         * Prints all of the transition actions to a file.
         * */
        void printTransitionActions(std::string filename);
        /**
         * Writes the min and max results to a file
         * 
         * @param filename The filename to append to
         * */
        void writeToOutput(std::string filename);
        /* Data Members */
        std::function<void(std::string)> err;
        std::function<void(std::string)> warn;
        std::function<void(std::string)> info;
        std::function<void(std::string)> good;
        Options * options;
        StaminaModelChecker::Result * min_results;
        StaminaModelChecker::Result * max_results;
        StaminaModelBuilder<double> * builder;
        storm::prism::Program * modulesFile;
        std::vector<storm::jani::Property> * propertiesVector;
    };

}

#endif // STAMINA_MODEL_CHECKER_H