//
// Created by Riley Layne Roberts on 2/21/20.
//

#ifndef STAMINA_STAMINAMODELCHECKER_H
#define STAMINA_STAMINAMODELCHECKER_H
//added these
#include <chrono>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

#include "StaminaOptions.h"
#include "InfCTMCModelGenerator.h"
#include "InfCTMCNextStateGenerator.h"

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


class StaminaModelChecker /*: public storm::modelchecker::SparseCtmcCslModelChecker<storm::models::sparse::Ctmc<double>>*/ {

    typedef storm::models::sparse::Ctmc<double> Ctmc;
    typedef storm::modelchecker::SparseCtmcCslModelChecker<Ctmc> CtmcModelChecker;


private:

    // InfCTMCModelGenerator<double>* infModelGen;

    void modifyExpression(storm::expressions::BaseExpression const& expr, bool isMin, storm::prism::Program const& modulesFile);
    bool terminateModelCheck(double minProb, double maxProb, double termParam);// throws PrismLangException;
    class Result {
    private:
        double result;
        std::string explanation;

    public:
        Result() = default;
        void setResultAndExplanation(double newResult, std::string newExplanation) {
            result = newResult;
            explanation = newExplanation;
        }

        double getResult() {
            return result;
        }

        std::string getExplanation() {
                return explanation;
        }

        std::string toString() {
            if (explanation.empty()) {
                return std::to_string(result);
            }
            else {
                std::stringstream returnString;
                returnString << result << " (" << explanation << ")";
                return returnString.str();
            }
        }

    };

public:
    StaminaModelChecker() {}/*: CtmcModelChecker(model)*/;



    void initialize();
    void setEngine();
    void setMaxIters(int maxIters) {};
    void loadPRISMModel(storm::prism::Program modulesFile) {};
    storm::prism::Program parseModelFile(std::string const& fileName);



    std::vector<storm::jani::Property> parsePropertiesFile(storm::prism::Program const& modulesFile, std::string const& propertiesFileName);


    std::unique_ptr<storm::modelchecker::CheckResult> modelCheckStamina(std::vector<storm::jani::Property> propertiesVector, storm::jani::Property prop, storm::prism::Program const& modulesFile); /*throws PrismException*/



};

#endif //STAMINA_STAMINAMODELCHECKER_H
