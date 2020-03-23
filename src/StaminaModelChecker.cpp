//
// Created by Riley Layne Roberts on 2/28/20.
//
//Test of git control
//#include <MacTypes.h>
#include "StaminaModelChecker.h"

template<typename BaseClass, typename CurrentClass>
inline bool instanceof(const CurrentClass*) {
    return std::is_base_of<BaseClass, CurrentClass>::value;
}

storm::prism::Program StaminaModelChecker::parseModelFile(std::string const& fileName) {
    return storm::parser::PrismParser::parse(fileName);
};

std::vector<storm::jani::Property> StaminaModelChecker::parsePropertiesFile(storm::prism::Program const& modulesFile, std::string const& propertiesFileName) {
    return storm::api::parsePropertiesForPrismProgram(propertiesFileName, modulesFile);
};

void StaminaModelChecker::initialize() {
    // Init loggers
    storm::utility::setUp();
    // Set some settings objects.
    storm::settings::initializeAll("Stamina", "Stamina");
};

//This is only used for non-combined

/*void StaminaModelChecker::modifyExpression(const storm::expressions::BaseExpression* expr, bool isMin, storm::prism::Program const& modulesFile) throws PrismLangException {

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

bool StaminaModelChecker::terminateModelCheck(double minProb, double maxProb, double termParam) {
    if(minProb == NULL || maxProb == NULL) {
        return false;
    }
    else if((( maxProb) - ( minProb)) <= termParam) {

        return true;
    }
    else {
        return false;
    }


}

storm::modelchecker::CheckResult StaminaModelChecker::modelCheckStamina(std::vector<storm::jani::Property> propertiesVector, storm::jani::Property prop, storm::prism::Program const& modulesFile) /*throws PrismException*/ {

        Result* res_min_max[2] = {new Result(), new Result()};

        double reachTh = Options::getReachabilityThreshold());

        // Instantiate and load model generator
        infModelGen = new InfCTMCModelGenerator(modulesFile, this);
        super.loadModelGenerator(infModelGen);

        // Time bounds
        double lTime, uTime;

        // Split property into 2 to find P_min and P_max

        std::string propName = prop.getName()==NULL ? "Prob" : prop.getName();

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

    auto exprProp = prop.getRawFormula();
    if(exprProp->isProbabilityPathFormula()) {


        while(numRefineIteration==0 || ((!terminateModelCheck(res_min_max[0]->getResult(), res_min_max[1]->getResult(), Options::getProbErrorWindow())) && (numRefineIteration < Options.getMaxApproxCount()))) {


                auto expr = exprProp;
                auto exprTemp = expr;

                if(exprTemp->isPathFormula() && (exprTemp->isUntilFormula()) && (!Options::getNoPropRefine())) {
                    infModelGen.setPropertyExpression(exprTemp);
                }

                if(exprTemp->isPathFormula() && (exprTemp->isUntilFormula())) {
                    switchToCombinedCTMC = true;
                }

                if(switchToCombinedCTMC) {

                    //////////////////////////Approximation Step///////////////////////////
                    std::cout << std::endl;
                    std::cout << "========================================================================" << std::endl;
                    std::cout << "Approximation<" << (numRefineIteration+1) << "> : kappa = " << reachTh << std::endl;
                    std::cout << "========================================================================" << std::endl;
                    infModelGen.setReachabilityThreshold(reachTh);


                    auto formulae = storm::api::extractFormulasFromProperties(propertiesVector);

                    // Now translate the prism program into a DTMC in the sparse format.
                    // Use the formulae to add the correct labelling.
                    auto model = storm::api::buildSparseModel<double>(modulesFile, formulae)->template as<Ctmc>();

                    // model check operands first for all states
                    auto mcCTMC = std::make_shared<CtmcModelChecker>(*model);
                    auto b1 = mcCTMC->check(storm::modelchecker::CheckTask<>(exprTemp->asUntilFormula().getLeftSubformula(), true));//.getBitSet();
                    auto b2 = mcCTMC->check(storm::modelchecker::CheckTask<>(exprTemp->asUntilFormula().getRightSubformula(), true));//.getBitSet();

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
                        auto tempValuation = new storm::expressions::SimpleValuation(timeExpr->getManager().getSharedPointer())
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
                    std::cout << "---------------------------------------------------------------------" <<std::end;
                    std::cout << std::endl;
                    std::cout << "Verifying " << propName << " ....." << std::endl;

                    auto timer = std::chrono::system_clock::now();

                    // run transient analysis
                    storm::Environment env;
                    auto probsExpl = storm::modelchecker::helper::SparseCtmcCslHelper::computeAllTransientProbabilities(env, model->getTransitionMatrix(), model->getInitialStates(), b1.get()->getTruthValuesVector(), b2.get()->getTruthValuesVector(), model->getExitRateVector(), uTime);


                    double ans_min = 0.0;

                    for(int i=0; i<model->getNumberOfStates(); ++i) {

                        if(!minStatesNeg.get()->) ans_min += (double) probsExpl[i]; //TODO: this line needs to be fixed

                    }

                    // TODO: need the index of absorbing state
                    double ans_max =  ans_min + (double) probsExpl[0];

                    std::chrono::duration timeElapsed = std::chrono::system_clock::now() - timer;
                    timeElapsed /= 1000.0;
                    std::cout << "\nTime for model checking: " << (timeElapsed.count()) << " seconds." << std::endl;

                    // set results
                    res_min_max[0]->setResultAndExplanation(ans_min, "minimum bound");


                    // Print result to log
                    std::cout << "\nResult: " << res_min_max[0]->toString() << std::endl);

                    res_min_max[1]->setResultAndExplanation(ans_max, "maximum bound");

                    // Print result to log
                    std::cout << "\nResult: " << res_min_max[1]->toString() << std::endl;



                }

                //Not using this for now
                /*else {

                    //////////////////////////Approximation Step///////////////////////////
                    mainLog.println();
                    mainLog.println("========================================================================");
                    mainLog.println("Approximation<" + (numRefineIteration+1) + "> : kappa = " + reachTh);
                    mainLog.println("========================================================================");
                    infModelGen.setReachabilityThreshold(reachTh);

                    // Explicitely invoke model build
                    super.buildModel();


                    mainLog.println();
                    mainLog.println("---------------------------------------------------------------------");
                    mainLog.println();
                    mainLog.println("Verifying Lower Bound for " + prop_min.getName() + " .....");
                    res_min_max[0] = super.modelCheck(propertiesFile, prop_min);

                    mainLog.println();
                    mainLog.println("---------------------------------------------------------------------");
                    mainLog.println();
                    mainLog.println("Verifying Upper Bound for " + prop_max.getName() + " .....");
                    res_min_max[1] = super.modelCheck(propertiesFile, prop_max);
                }*/


                // Reduce kappa for refinement
                reachTh /= Options::getKappaReductionFactor();

                // increment refinement count
                ++numRefineIteration;

        }

    }

}


