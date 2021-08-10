//
// Created by Riley Layne Roberts on 2/28/20.
//
// Test of git control
// #include <MacTypes.h>
#include "StaminaModelChecker.h"

// #include "StaminaExplicitModelBuilder.h"

template<typename BaseClass, typename CurrentClass>
inline bool instanceof(const CurrentClass*) {
    return std::is_base_of<BaseClass, CurrentClass>::value;
}

storm::prism::Program StaminaModelChecker::parseModelFile(std::string const& fileName) {
    bool prismCompatibility = true;
    return storm::parser::PrismParser::parse(fileName, prismCompatibility);
};
/**
 * brief parsePropertiesFile - Parses the properties file passed in using the storm API
 * 
 * @param modulesFile The modules file.
 * @param propertiesFileName The properties file to parse.
 * */
std::vector<storm::jani::Property> StaminaModelChecker::parsePropertiesFile(
    storm::prism::Program const& modulesFile
    , std::string const& propertiesFileName
) {
    return storm::api::parsePropertiesForPrismProgram(propertiesFileName, modulesFile);
};
/**
 * brief initialize - Initializes everything we need for the STAMINA model checker.
 * */
void StaminaModelChecker::initialize() {
    // Init loggers
    storm::utility::setUp();
    // Set some settings objects.
    storm::settings::initializeAll("Stamina", "Stamina");
};

//This is only used for non-combined

/*void StaminaModelChecker::modifyExpression(
    const storm::expressions::BaseExpression* expr
    , bool isMin, storm::prism::Program const& modulesFile
) {

        if(instanceof<storm::expressions::BinaryExpression>(&expr) ) {
            if(expr->isBinaryRelationExpression()) {
                std::shared_ptr<const storm::expressions::BaseExpression> op1 = expr->asBinaryRelationExpression().getFirstOperand();
                if (instanceof<storm::expressions::VariableExpression>(op1.get())) {

                    /*need state class if there is one auto absSt = infModelGen.getAbsorbingState();
                    std::string varName = (op1->asVariableExpression().getVariableName());


                    storm::expressions::BinaryRelationExpression* newOp1 = new storm::expressions::BinaryRelationExpression(expr->asBinaryRelationExpression());

                    storm::expressions::BinaryRelationExpression* abs = new storm::expressions::BinaryRelationExpression(expr->getManager(),
                                                                            expr->getType(),
                                                                            newOp1->getFirstOperand(),
                                                                            new storm::expressions::IntegerLiteralExpression(expr->getManager(),
                                                                                                  absSt.get(modulesFile.getModuleIndexByVariable(varName))), //I'm not sure if this is the correct index
                                                                            storm::expressions::BinaryRelationExpression::RelationType::Equal) ;

                    if (isMin) {
                        storm::expressions::UnaryBooleanFunctionExpression* newOp2 = new storm::expressions::UnaryBooleanFunctionExpression(expr->getManager(), expr->getType(), abs, storm::expressions::UnaryBooleanFunctionExpression::OperatorType::Not
                                                                                        ); //This originally had a parentheses unaryOp, I couldn't find that in storm, not sure how that affects.
                        delete expr;
                        const storm::expressions::BaseExpression* newExpr = new storm::expressions::BinaryBooleanFunctionExpression(expr->getManager(), expr->getType(), newOp1, newOp2, storm::expressions::BinaryBooleanFunctionExpression::OperatorType::And);
                        *expr = *newExpr;
                        /*(expr.asBinaryRelationExpression()).setOperator("&");
                        (expr.asBinaryRelationExpression()).setOperand1(newOp1);
                        (expr.asBinaryRelationExpression()).setOperand2(newOp2);
                    } else {

                        storm::expressions::BinaryRelationExpression* newOp2 = abs; //Had a parentheses unary op before


                        //Old code that used setters. Trying to do a direct assignment here
                        /*
                        (expr.asBinaryRelationExpression()).setOperator("|");
                        (expr.asBinaryRelationExpression()).setOperand1(newOp1);
                        (expr.asBinaryRelationExpression()).setOperand2(newOp2);

                    }


                }

            }
            else {
                modifyExpression(expr.asBinaryRelationExpression().getFirstOperand(), isMin, modulesFile);
                modifyExpression(((ExpressionBinaryOp) expr).getOperand2(), isMin);
            }
        }
        else if(expr instanceof ExpressionProb) {
            modifyExpression(((ExpressionProb) expr).getExpression(), isMin);
        }
        else if(expr instanceof ExpressionTemporal) {
            modifyExpression(((ExpressionTemporal) expr).getOperand1(), isMin);
            modifyExpression(((ExpressionTemporal) expr).getOperand2(), isMin);
        }
        else if(expr instanceof ExpressionUnaryOp) {
            modifyExpression(((ExpressionUnaryOp) expr).getOperand(), isMin);
        }



        //return expr;
}*/
/**
 * Whether or not we can terminate the model checking.
 * 
 * @param minProb The lower bound on the probability.
 * @param maxProb The upper bound on the probability.
 * @param termParam The (tight) range we want for Pmax - Pmin
 * @return Whether or not the model achieved maxProb - minProb <= termParam
 * */
bool StaminaModelChecker::terminateModelCheck(double minProb, double maxProb, double termParam) {
    if(!minProb || !maxProb) {
        return false;
    }
    else if (((maxProb) - (minProb)) <= termParam) {
        return true;
    }
    else {
        return false;
    }
}
/**
 * brief modelCheckStamina - Performs the model checking of properies file and returns results.
 * 
 * @param propertiesVector List of properties in modules.
 * @param prop Property to check.
 * @param modulesFile PRISM modules file. (To be read in by STORM)
 * */
std::unique_ptr<storm::modelchecker::CheckResult> StaminaModelChecker::modelCheckStamina(
    std::vector<storm::jani::Property> propertiesVector
    , storm::jani::Property prop
    , storm::prism::Program const& modulesFile
) {

std::unique_ptr<storm::modelchecker::CheckResult> StaminaModelChecker::modelCheckStamina(std::vector<storm::jani::Property> propertiesVector, storm::jani::Property prop, storm::prism::Program const& modulesFile) {
        Result* res_min_max[2] = {new Result(), new Result()};

    double reachTh = StaminaOptions::getReachabilityThreshold();

    // Lower and upper bounds on time
    double lTime, uTime;

    // Instantiate and load model generator
    infModelGen = new InfCTMCModelGenerator<double>(modulesFile);
    //super.loadModelGenerator(infModelGen);

        // Instantiate and load model generator
       // infModelGen = new InfCTMCNextStateGenerator<double>(modulesFile);
        //super.loadModelGenerator(infModelGen);

    // Split property into 2 to find P_min and P_max

    std::string propName = prop.getName().empty() ? "Prob" : prop.getName();

    // storm::jani::Property prop_min = new storm::jani::Property(propName + "_min", modifyExpression(prop.getFilter().getFormula()->toExpression().getBaseExpression(), true, modulesFile), prop.getUndefinedConstants(), prop.getComment());

    //Old way to call modify expression
    /*prop_min.setName(propName+"_min");
    modifyExpression(prop_min.getExpression(), true);*/

    //storm::jani::Property prop_max = new storm::jani::Property(propName + "_max", modifyExpression(prop.getFilter().getFormula()->toExpression().getBaseExpression(), false, modulesFile), prop.getUndefinedConstants(), prop.getComment());

    //Old way to call modify expression
    /*prop_max.setName(propName+"_max");
        modifyExpression(prop_max.getExpression(), false);*/


    // timer
    long timer = 0;

    // iteration count
    int numRefineIteration = 0;

    // flag to switch optimized CTMC analysis
    bool switchToCombinedCTMC = false;

    std::shared_ptr<storm::logic::Formula const> exprProp = prop.getRawFormula();
    //if(exprProp->isProbabilityPathFormula()) {


    //while(numRefineIteration==0 || ((!terminateModelCheck(res_min_max[0]->getResult(), res_min_max[1]->getResult(), StaminaOptions::getProbErrorWindow())) && (numRefineIteration < StaminaOptions::getMaxApproxCount()))) {


    auto expr = exprProp;
    auto exprTemp = expr;

    if(exprTemp->isPathFormula() && (exprTemp->isUntilFormula()) && (!StaminaOptions::getNoPropRefine())) {
        //infModelGen->setPropertyExpression(exprTemp);
    }

    if(exprTemp->isPathFormula() && (exprTemp->isUntilFormula())) {
        switchToCombinedCTMC = true;
    }

    if (instanceof</*TODO: ExpressionTemporal*/>(exprProp)) {
        while (numRefineIteration == 0 || 
            ((!terminateModelCheck(
                res_min_max[0].getResult()
                , res_min_max[0].getResult()
                , StaminaOptions::getProbErrorWindow()
            ))
            && (numRefineIteration < StaminaOptions::getMaxApproxCount()))
        ) {
            reachTh = StaminaOptions::getReachabilityThreshold();
            std::shared_ptr<storm::logic::Formula const> expr = exprProp;
            if (instanceof</*TODO: ExpressionTemporal*/>(expr)) {
                /* TODO: Change to cast to the storm equivalent of ExpressionTemporal */
                std::shared_ptr<storm::logic::Formula const> exprTemp = expr;

                if (exprTemp->isPathFormula() && (exprTemp->getOperator()==/*TODO: ExpressionTemporal*/P_U) && (!StaminaOptions::getNoPropRefine())) {
                    infModelGen->setPropertyExpression(exprTemp);
                }

                if (exprTemp->isPathFormula() && (exprTemp->getOperator()==/*TODO: ExpressionTemporal*/P_U)) {
                    switchToCombinedCTMC = true;
                }

    //////////////////////////Approximation Step///////////////////////////
    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "Approximation<" << (numRefineIteration+1) << "> : kappa = " << reachTh << std::endl;
    std::cout << "========================================================================" << std::endl;
    //infModelGen->setReachabilityThreshold(reachTh);

                    // TODO: run transient analysis

<<<<<<< HEAD
                    //////////////////////////Approximation Step///////////////////////////
                    std::cout << std::endl;
                    std::cout << "========================================================================" << std::endl;
                    std::cout << "Approximation<" << (numRefineIteration+1) << "> : kappa = " << reachTh << std::endl;
                    std::cout << "========================================================================" << std::endl;
                    //infModelGen->setReachabilityThreshold(reachTh);


                    auto formulae = storm::api::extractFormulasFromProperties(propertiesVector);

                    // Now translate the prism program into a CTMC in the sparse format.
                    // Use the formulae to add the correct labelling.
                    storm::builder::BuilderOptions options(formulae, modulesFile);

                    // auto generator = std::make_shared<storm::generator::PrismNextStateGenerator<double, uint32_t>>(modulesFile, options);
                    auto generator = std::make_shared<storm::generator::InfCTMCNextStateGenerator<double, uint32_t>>(modulesFile, options);
                    // auto variableInformation = VariableInformation(modulesFile, options.isAddOutOfBoundsStateSet());
                    storm::builder::StaminaExplicitModelBuilder<double> builder(generator);
                    auto model = builder.build()->as<Ctmc>();
                    auto mcCTMC = std::make_shared<CtmcModelChecker>(*model);
                    auto result = mcCTMC->check(storm::modelchecker::CheckTask<>(*(formulae[0]), true));   // should return pMin? or maaaaybe pMax
                    auto quantRes = result->asExplicitQuantitativeCheckResult<double>();
                    std::cout << "Quantitative Result: " << quantRes << std::endl;
                    return result;
=======
                    double ansMin, ansMax;
                    ansMin = 0.0;

                    for (int i = 1; i < /* TODO: get total states in explicit build model */; ++i) {
                        if (b2[i]) {
                            ansMin += (double) // TODO: get value from transient analysis.
                        }
                    }
                    if (infModelGen->finalModelHasAbsorbing()) {
                        // TODO: needs to get index of absorbing state
                        ansMax = ansMin + (double) // TODO: first element from transient analysis.
                        if (ansMax > 1.0) {
                            ansMax = 1.0;
                        }
                    }
                    else {
                        // TODO: get index of absorbing state
                        ansMax = ansMin;
                    }

                    timer = std::chrono::system_clock::now() - timer;
                    // Show elapsed time
                    std::cout << "Elapsed time for model checking: " << timer / 1000.0 << " seconds." << std::endl;

                    // Set Results
                    res_min_max[0]->setResultAndExplaination(ansMin, "minimum bound");
                    res_min_max[1]->setResultAndExplaination(ansMax, "maximum bound");

                    std::cout <<"\nResult:" << res_min_max[0]->getResultsString() << std::endl; // TODO: temporary implementation exists for Results::getResultsString
                    std::cout <<"\nResult:" << res_min_max[1]->getResultsString() << std::endl;
                }
                else {
                    // TODO: build model
                    
                    // Verification
                    std::cout << "\n\n---------------------------------------------------------------------\n\n";
                    std::cout << "Verifying lower bound for " << propMin.getName() << " ....";
                    // TODO: Invoke model check from superclass (which there is none yet)
                    std::cout << "\n\n---------------------------------------------------------------------\n\n";
                    std::cout << "Verifying lower bound for " << propMax.getName() << " ....";
                    // TODO: Invoke model check from superclass (which there is none yet)
                }

                // Write results to file
                try {
                    std::ofstream outfile("results.txt", std::ofstream::trunc);
                    outfile << ansMin << "\r\n" << ansMax << "\r\n";
                    outfile.close();
                }
                catch (const std::exception& e) {
                    std::cerr << e.what() << '\n';
                }

                // Reduce kappa for refinement
                double diff = (double) res_min_max[1]->getResult() - (double) res_min_max[0]->getResult();
                double percentOff = 4 * (diff) / StaminaOptions::getProbErrorWindow();
                if (percentOff > 100.0) {
                    percentOff = 100.0;
                }

                StaminaOptions::setMispredictionFactor(StaminaOptions::getMispredictionFactor() * percentOff);

                // Increment refiinement count
                if (StaminaOptions::getExportPerimeterStates()) {
                    try {
                        std::ofstream outf(StaminaOptions::getExportPerimeterFilename(), std::ofstream::trunc);
                        std::vector<std::string> values = infModelGen->getPerimeterStatesVector();
                        outf << "Iteration: " << numRefineIteration << "\r\n";
                        for (int i = 0; i < infModelGen->getNumVars(); ++i) {
                            outf << infModelGen->getVarName(i) << ',';
                        }
                        outf << "Current Reachability Probability" << std::endl;
                        for (int i = 0; i < values.size(); ++i) {
                            outf << values.at(i) << std::endl;
                        }
                        outf.close();
                    }
                    catch (const std::exception& e) {
                        std::cerr << e.what() << '\n';
                    }
                    
                }
                ifModelGen.clearPerimeterStatesVector();
                ++numRefineIteration;
            }
        }
    }

    // Print the final results

    std::cout << "\n\n---------------------------------------------------------------------\n\n";
    std::cout << "Property: " << prop << "\n\n";
    std::cout << "ProbMin: " << res_min_max[0].getResultsString() << "\n\n";
    std::cout << "ProbMax: " << res_min_max[1].getResultsString() << "\n\n";

    if (StaminaOptions::getExportTransitionsToFile() != nullptr) {
        std::cout << "\n\nExporting transition list..." << std::endl;
        printTransitionActions(infModelGen, StaminaOptions::getExportTransitionsToFile());
        std::cout << "Export complete." << std::endl;
    }


    return res_min_max[0];

    /* auto formulae = storm::api::extractFormulasFromProperties(propertiesVector);

    // Now translate the prism program into a CTMC in the sparse format.
    // Use the formulae to add the correct labelling.
    storm::builder::BuilderOptions options(formulae, modulesFile);

    // auto generator = std::make_shared<storm::generator::PrismNextStateGenerator<double, uint32_t>>(modulesFile, options);
    auto generator = std::make_shared<storm::generator::InfCTMCNextStateGenerator<double, uint32_t>>(modulesFile, options);
    // auto variableInformation = VariableInformation(modulesFile, options.isAddOutOfBoundsStateSet());
    storm::builder::ExplicitModelBuilder<double> builder(generator);
    auto model = builder.build()->as<Ctmc>();
    auto mcCTMC = std::make_shared<CtmcModelChecker>(*model);
    auto result = mcCTMC->check(storm::modelchecker::CheckTask<>(*(formulae[0]), true));   // should return pMin? or maaaaybe pMax
    auto quantRes = result->asExplicitQuantitativeCheckResult<double>();
    std::cout << "Quantitative Result: " << quantRes << std::endl;
<<<<<<< HEAD
    return result; */
    // if(result->isExplicitQuantitativeCheckResult()) {
    //     retu
    // } else {
    //     return null;
    // }

    //return quantRes;
    // Eventually replace this^^ line with transient check (including absorbing state)
    /*


    // model check operands first for all states
        //TODO: Make model checker based on model
    /*auto b1 = mcCTMC->check(storm::modelchecker::CheckTask<>(exprTemp->asUntilFormula().getLeftSubformula(), true));.getBitSet();
    auto b2 = mcCTMC->check(storm::modelchecker::CheckTask<>(exprTemp->asUntilFormula().getRightSubformula(), true));.getBitSet();

    std::shared_ptr<storm::modelchecker::CheckResult> minStatesNeg = b1.get()->clone();
    minStatesNeg.andNot(b2); //TODO: I need to figure out how to do this in storm

    // lower bound is 0 if not specified
    // (i.e. if until is of form U<=t)
    auto timeExpr = &exprTemp->asBoundedUntilFormula().getLowerBound();
    if (timeExpr != NULL) {
        auto tempValuation = new storm::expressions::SimpleValuation(timeExpr->getManager().getSharedPointer());
        lTime = timeExpr->evaluateAsDouble(tempValuation);
        delete tempValuation;
        if (lTime < 0) {
            //throw new PrismException("Invalid lower bound " + lTime + " in time-bounded until formula");
            //need to add correct exception throwing here
        }
    } else {
        lTime = 0;
    }
    // upper bound is -1 if not specified
    // (i.e. if until is of form U>=t)
    timeExpr = &exprTemp->asBoundedUntilFormula().getUpperBound();
    if (timeExpr != NULL) {
        auto tempValuation = new storm::expressions::SimpleValuation(timeExpr->getManager().getSharedPointer());
        uTime = timeExpr->evaluateAsDouble(tempValuation);
        delete tempValuation;
        if (uTime < 0 || (uTime == 0 && exprTemp->asBoundedUntilFormula().isUpperBoundStrict())) {
            std::stringstream boundStream;
            boundStream << (exprTemp->asBoundedUntilFormula().isUpperBoundStrict() ? "<" : "<=") << uTime;
            std::string bound = boundStream.str();
            //throw new PrismException("Invalid upper bound " + bound + " in time-bounded until formula");
            //need to add correct exception throwing here
        }
        if (uTime < lTime) {
            //throw new PrismException("Upper bound must exceed lower bound in time-bounded until formula");
            //need to add correct exception throwing here
        }
    } else {
        uTime = -1;
    }
=======
    return result;
>>>>>>> 04f2468abbf4262826516b6a79b33d83c7c8a10b
                    // if(result->isExplicitQuantitativeCheckResult()) {
                    //     retu
                    // } else {
                    //     return null;
                    // }

                    //return quantRes;
                    // Eventually replace this^^ line with transient check (including absorbing state)
/*


                    // model check operands first for all states
                     //TODO: Make model checker based on model
                    /*auto b1 = mcCTMC->check(storm::modelchecker::CheckTask<>(exprTemp->asUntilFormula().getLeftSubformula(), true));.getBitSet();
                    auto b2 = mcCTMC->check(storm::modelchecker::CheckTask<>(exprTemp->asUntilFormula().getRightSubformula(), true));.getBitSet();

                    std::shared_ptr<storm::modelchecker::CheckResult> minStatesNeg = b1.get()->clone();
                    minStatesNeg.andNot(b2); //TODO: I need to figure out how to do this in storm

                    // lower bound is 0 if not specified
                    // (i.e. if until is of form U<=t)
                    auto timeExpr = &exprTemp->asBoundedUntilFormula().getLowerBound();
                    if (timeExpr != NULL) {
                        auto tempValuation = new storm::expressions::SimpleValuation(timeExpr->getManager().getSharedPointer());
                        lTime = timeExpr->evaluateAsDouble(tempValuation);
                        delete tempValuation;
                        if (lTime < 0) {
                            //throw new PrismException("Invalid lower bound " + lTime + " in time-bounded until formula");
                            //need to add correct exception throwing here
                        }
                    } else {
                        lTime = 0;
                    }
                    // upper bound is -1 if not specified
                    // (i.e. if until is of form U>=t)
                    timeExpr = &exprTemp->asBoundedUntilFormula().getUpperBound();
                    if (timeExpr != NULL) {
                        auto tempValuation = new storm::expressions::SimpleValuation(timeExpr->getManager().getSharedPointer());
                        uTime = timeExpr->evaluateAsDouble(tempValuation);
                        delete tempValuation;
                        if (uTime < 0 || (uTime == 0 && exprTemp->asBoundedUntilFormula().isUpperBoundStrict())) {
                            std::stringstream boundStream;
                            boundStream << (exprTemp->asBoundedUntilFormula().isUpperBoundStrict() ? "<" : "<=") << uTime;
                            std::string bound = boundStream.str();
                            //throw new PrismException("Invalid upper bound " + bound + " in time-bounded until formula");
                            //need to add correct exception throwing here
                        }
                        if (uTime < lTime) {
                            //throw new PrismException("Upper bound must exceed lower bound in time-bounded until formula");
                            //need to add correct exception throwing here
                        }
                    } else {
                        uTime = -1;
                    }
>>>>>>> parent of 092a837... Formatting (x2)

                    if(lTime>0.0);//need stuff here for exceptions //throw new stormException("Currently only supports [0,t] time bound.");

                    // verification step
                    std::cout << std::endl;
                    std::cout << "---------------------------------------------------------------------" << std::endl;
                    std::cout << std::endl;
                    std::cout << "Verifying " << propName << " ....." << std::endl;

                    auto timer = std::chrono::system_clock::now();

                    // run transient analysis
                    storm::Environment env;
                    //auto probsExpl = storm::modelchecker::helper::SparseCtmcCslHelper::computeAllTransientProbabilities(env, model->getTransitionMatrix(), model->getInitialStates(), b1.get()->getTruthValuesVector(), b2.get()->getTruthValuesVector(), model->getExitRateVector(), uTime);


                    double ans_min = 0.0;

                    for(int i=0; i<model.getNumberOfStates(); ++i) {

                       // if(!minStatesNeg.get()->) ans_min += (double) probsExpl[i]; //TODO: this line needs to be fixed

                    }

                    // TODO: need the index of absorbing state
                    //double ans_max =  ans_min + (double) probsExpl[0];

                    auto timeElapsed = std::chrono::system_clock::now() - timer;
                    timeElapsed /= 1000.0;
                    std::cout << "\nTime for model checking: " << (timeElapsed.count()) << " seconds." << std::endl;

                    // set results
                    res_min_max[0]->setResultAndExplanation(ans_min, "minimum bound");


                    // Print result to log
                    std::cout << "\nResult: " << res_min_max[0]->toString() << std::endl;

                    //res_min_max[1]->setResultAndExplanation(ans_max, "maximum bound");

                    // Print result to log
                    std::cout << "\nResult: " << res_min_max[1]->toString() << std::endl;



                // Reduce kappa for refinement
                reachTh /= StaminaOptions::getKappaReductionFactor();

                // increment refinement count
                ++numRefineIteration;

        //}

    //}
*/
}

/**
 * Prints all transition actions taken by the InfCTMCModelGenerator to a file.
 * @param modelGen The model generator we're using.
 * @param exportFileName The file we're exporting to.
 */
// TODO: Remove all autos and replace them with proper typing
// TODO: figure out how to dequeue and send to file.
void StaminaModelChecker::printTransitionActions(InfCTMCModelGenerator<double> * modelGen, std::string exportFileName) {
    std::map</* TODO: StateType */, /* TODO: ProbState */ > globalStateSet = modelGen->getGlobalStateSet(); // getGlobalStateSet() not yet implemented
    boost::property_tree::ptree sortedStates; // TODO: sort keys from globalStateSet

    if (modelGen->finalModelHasAbsorbing()) {
        sortedStates.add(modelGen->getAbsorbingState());
    }

    std::map</* TODO: StateType */, int> stateIndex;
    int spot = 0;
    for (auto sortState : sortedStates) {
        stateIndex.put(sortState, spot);
        spot++;
    }

    try {
        std::ofstream out(exportFileName, std::ofstream::trunc);
        for (auto exploredState : sortedStates) {
            modelGen->exploreState(exploredState);
            // Look at each outgoing choice from the state
            int nc = modelGen->getNumChoices();
            boost::property_tree::ptree sortedTrans; // TODO: sort the sortedStates
            // Sort the transitions
            for (int i = 0; i < nc; i++) {
                int nt = modelGen->getNumTransitions(i);
                for (int j = 0; j < nt; j++) {
                    auto stateNew = modelGen->computeTransitionTarget(i, j); // computeTransitionTarget is not implemented either
                    int indecies[2] = {i, j};
                    sortedTrans.put(stateNew, indecies);
                }
            }

            // While we still have transitions to export
            while (!sortedTrans.empty()) {
                auto mapping = sortedTrans.pop_front(); // Not sure if this is what's needed
                // auto stateNew = mapping.getKey()
            }

        }
        out.close();
    }
    catch (const std::exception& e) {
        std::cerr << "An error occured in creating the transition file: " << e.what() << '\n';
    }
    
}
/**
 * Gets the results string for a Result
 * 
 * @return Results string
 * */
std::string StaminaModelChecker::Result::getResultsString() {
    std::string result = self->toString();
    // TODO: not sure if we just want to use the toString Method?
    return result;
}