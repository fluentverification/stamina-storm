//
// Created by Riley Layne Roberts on 2/28/20.
//
// Test of git control
// #include <MacTypes.h>
#include "StaminaModelChecker.h"

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

    // Get max and min properties
    storm::jani::Property propMin(
        propName + "_min"
        , modifyExpression(
            prop.getFilter().getFormula()->toExpression().getBaseExpression()
            , true
            , modulesFile
        )
        , prop.getUndefinedConstants()
        , prop.getComment()
    );

    storm::jani::Property propMax(
        propName + "_min"
        , modifyExpression(
            prop.getFilter().getFormula()->toExpression().getBaseExpression()
            , false
            , modulesFile
        )
        , prop.getUndefinedConstants()
        , prop.getComment()   
    );

    // timer
    long timer = 0;

    // iteration count
    int numRefineIteration = 0;

    // flag to switch optimized CTMC analysis
    bool switchToCombinedCTMC = false;

    // I don't like the "auto" keyword. >:(
    std::shared_ptr<storm::logic::Formula const> exprProp = prop.getRawFormula();
    //if(exprProp->isProbabilityPathFormula()) {


    /* while(numRefineIteration==0 || 
        ((!terminateModelCheck(res_min_max[0]->getResult(), res_min_max[1]->getResult(), StaminaOptions::getProbErrorWindow())) 
        && (numRefineIteration < StaminaOptions::getMaxApproxCount()))) { */

    // std::shared_ptr<storm::logic::Formula const> expr = exprProp;
    // std::shared_ptr<storm::logic::Formula const> exprTemp = expr;

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

                if (StaminaOptions::getNoPropRefine()) {
                    switchToCombinedCTMC = false;
                }

                /* ============ Approximation Step ============ */
                std::cout << std::endl;
                std::cout << "========================================================================" << std::endl;
                std::cout << "Approximation<" << (numRefineIteration + 1) << "> : kappa = " << reachTh << std::endl;
                std::cout << "========================================================================" << std::endl;
                infModelGen->setReachabilityThreshold(reachTh);
                if (switchToCombinedCTMC) {

                    // Invoke model build explicitly
                    if (StaminaOptions::getImportModel()) {
                        // Base filename
                        std::string filename = StaminaOptions::getImportFileName();
                        // States file
                        FILE * sf = nullptr;
                        std::string statesFile = filename + ".sta";
                        // Labels file
                        FILE * lf = nullptr;
                        std::string labelsFile = filename + ".lab";
                        // State rewards file
                        FILE * srf = nullptr;
                        std::string stateRewardsFile = filename + ".srew";
                        // Transitions file
                        FILE * mf = nullptr;
                        std::string transFile = filename + ".tra";
                        try {
                            // TODO: loadModelFromExplicitFiles(). Currently the C++ version doesn't have a superclass
                        }
                        catch(const std::exception& e) {
                            std::cerr << e.what() << std::endl;
                        }
                    }
                    // TODO: buildModel(). The Java version invoked this from the superclass.

                    // Export if necessary
                    if (StaminaOptions::getExportModel()) {
                        // Base filename
                        std::string filename = StaminaOptions::getExportFileName();
                        // States file
                        FILE * sf = nullptr;
                        std::string statesFile = filename + ".sta";
                        // Labels file
                        FILE * lf = nullptr;
                        std::string labelsFile = filename + ".lab";
                        // State rewards file
                        FILE * srf = nullptr;
                        std::string stateRewardsFile = filename + ".srew";
                        // Transitions file
                        FILE * mf = nullptr;
                        std::string transFile = filename + ".tra";
                        try {
                            // TODO: Export everything. Currently the C++ version doesn't have a superclass, which the Java version invoked
                            // to do this part.
                        }
                        catch(const std::exception& e) {
                            std::cerr << e.what() << std::endl;
                        }
                    }

                    // Model check operands first for all states.
                    // TODO: get mcCTMC. Also, find out what size the bitset should be.
                    std::bitset<8> b2 = // mcCTMC::checkExpression;

                    // Lower bound is set to 0 unless specified (U<=t)
                    // TODO: get timeExpr. storm::jani::formula has no getLowerBound() method.
                    auto timeExpr = nullptr; // Also, change this from auto to whatever the storm equivalent of Expression is
                    
                    if (timeExpr != nullptr) {
                        // TODO: get lower bound from exprTemp and mcCTMC
                        // Throw an error if < 0
                        STORM_LOG_THROW(timeExpr < 0, storm::exceptions::OutOfRangeException, "Lower bound must be greater than 0");
                    }
                    else {
                        lTime = 0;
                    }

                    // Default upper bound is -1
                    // TODO: get upper bound from exprTemp
                    if (timeExpr != nullptr) {
                        // TODO: get upper bound from exprTemp and mcCTMC
                        // Throw error if uTime < 0 or if it is zero and the upper bound is strict
                        STORM_LOG_THROW(
                            uTime < 0 // || (uTime == 0 && exprTemp.upperBoundIsStrict())
                            , storm::exceptions::OutOfRangeException
                            , "Invalid upper bound in time bounded until formula"
                        );
                        STORM_LOG_THROW(uTime < lTime, storm::exceptions::OutOfRangeException, "Upper bound must exceed lower bound");
                    }
                    else {
                        uTime = -1;
                    }

                    STORM_LOG_THROW(lTime > 0.0, storm::exceptions::OutOfRangeException, "This version of STAMINA only supports causal [0,t] time bounds");
                    
                    // Verification
                    std::cout << "\n\n---------------------------------------------------------------------\n\n";
                    std::cout << "Verifying " << propName << " ....." << std::endl;

                    timer = std::chrono::system_clock::now();

                    // TODO: run transient analysis

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

                    std::cout <<"\nResult:" << res_min_max[0]->getResultsString() << std::endl; // TODO: no implementation exists for Results::getResultsString
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
                        std::ofstream outf(StaminaOptions::getExportPerimeterFilename(), , std::ofstream::trunc);
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

    if(lTime>0.0);//need stuff here for exceptions //throw new stormException("Currently only supports [0,t] time bound.");

    // verification step
    std::cout << std::endl;
    std::cout << "---------------------------------------------------------------------" << std::endl;
    std::cout << std::endl;
    std::cout << "Verifying " << propName << " ....." << std::endl;

    auto timer = std::chrono::system_clock::now();

    // run transient analysis
    storm::Environment env;
    //auto probsExpl = storm::modelchecker::helper::SparseCtmcCslHelper::computeAllTransientProbabilities(
        env, model->getTransitionMatrix()
        , model->getInitialStates()
        , b1.get()->getTruthValuesVector()
        , b2.get()->getTruthValuesVector()
        , model->getExitRateVector()
        , uTime
    );


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
