//
// Created by Riley Layne Roberts on 3/26/20.
//

#include "StaminaCL.h"

    const std::string StaminaCL::dotPrism = ".prism";
    const std::string StaminaCL::dotSm = ".sm";
    const std::string StaminaCL::dotCsl = ".csl";
    double StaminaCL::reachabilityThreshold = -1.0;
    double StaminaCL::kappaReductionFactor = -1;
    int StaminaCL::numPropertiesToCheck = 0;
    int StaminaCL::maxApproxCount = -1;
    double StaminaCL::probErrorWindow = -1.0;
    bool StaminaCL::noPropRefine = false;
    bool StaminaCL::rankTransitions = false;
    int StaminaCL::maxLinearSolnIter = -1;
    std::string StaminaCL::constSwitch = "";
    std::string StaminaCL::modelFilename = "";
    std::string StaminaCL::propertiesFilename = "";
    std::string StaminaCL::solutionMethod = "";
    StaminaModelChecker* StaminaCL::staminaMC = nullptr;
    storm::prism::Program StaminaCL::modulesFile = storm::prism::Program();
    std::vector<storm::jani::Property> StaminaCL::propertiesVector = std::vector<storm::jani::Property>();


void StaminaCL::run(int argv, char* argc[]) {

    //Need Result type possibly Result res;
    //mainLog = new PrismFileLog("stdout");

    //Initialize
    initializeSTAMINA();

    // Parse options
    doParsing(argv, argc);

    // Process options
    processOptions();

    try {
        // process info about undefined constant




        /*std::set<storm::expressions::Variable> undefinedConstants[numPropertiesToCheck];
         for (int i = 0; i < numPropertiesToCheck; i++) {
             undefinedConstants[i] = propertiesVector[i].getUndefinedConstants();
         }*/
        std::map<storm::expressions::Variable, storm::expressions::Expression> constMap = storm::utility::cli::parseConstantDefinitionString(modulesFile.getManager(),constSwitch);

        modulesFile.defineUndefinedConstants(constMap);
        for (int i = 0; i < numPropertiesToCheck; i++) {
            propertiesVector[i].substitute(constMap); //Not sure if this works, need to check
        }

        // initialise storage for results
        storm::modelchecker::CheckResult* results[numPropertiesToCheck];

        //still need to figure this part out
        /*for (int i = 0; i < numPropertiesToCheck; i++) {
            results[i] = new ResultsCollection(undefinedConstants[i], propertiesToCheck.get(i).getExpression().getResultName());
        }*/

        // iterate through as many models as necessary
        //ask zhen about this part, what is happening.
        /*for (int i = 0; i < undefinedMFConstants.getNumModelIterations(); i++) {

            // set values for ModulesFile constants
            //Dont think we need this as already defined the constants
            /*try {
                definedMFConstants = undefinedMFConstants.getMFConstantValues();
                staminaMC.setPRISMModelConstants(definedMFConstants);
            } catch (PrismException e) {
                // in case of error, report it, store as result for any properties, and go on to the next model
                // (might happen for example if overflow or another numerical problem is detected at this stage)
                mainLog.println("\nError: " + e.getMessage() + ".");
                for (int j = 0; j < numPropertiesToCheck; j++) {
                    results[j].setMultipleErrors(definedMFConstants, null, e);
                }
                // iterate to next model
                undefinedMFConstants.iterateModel();
                for (int j = 0; j < numPropertiesToCheck; j++) {
                    undefinedConstants[j].iterateModel();
            }

            // Work through list of properties to be checked
            for (int j = 0; j < numPropertiesToCheck; j++) {


                for (int k = 0; k < undefinedConstants[j].getNumPropertyIterations(); k++) {

                    try {
                        // Set values for PropertiesFile constants
                        if (propertiesVector != NULL) {
                            definedPFConstants = undefinedConstants[j].getPFConstantValues();
                            propertiesVector.setSomeUndefinedConstants(definedPFConstants);
                        }

                        res = staminaMC->modelCheckStamina(propertiesFile, propertiesToCheck.get(j));



                    } catch (PrismException e) {
                        mainLog.println("\nError: " + e.getMessage() + ".");
                        res = new Result(e);
                    }

                    // store result of model checking
                    results[j].setResult(definedMFConstants, definedPFConstants, res.getResult());
                    //results[j+1].setResult(definedMFConstants, definedPFConstants, res[1].getResult());

                    // iterate to next property
                    undefinedConstants[j].iterateProperty();

                }
            }

            // iterate to next model
            undefinedMFConstants.iterateModel();
            for (int j = 0; j < numPropertiesToCheck; j++) {
                undefinedConstants[j].iterateModel();
            }

        }*/
        for(auto property: propertiesVector) {
            auto result = staminaMC->modelCheckStamina(propertiesVector, property, modulesFile);
        }

    } catch (stormException e) {
        std::cout << "Error: " << e.what() << std::endl;
        errorAndExit(e.what());
    }
}


void StaminaCL::initializeSTAMINA() {

    //init prism
    try {
        // Create a log for PRISM output (hidden or stdout)
        //mainLog = new PrismDevNullLog();
        //mainLog = new PrismFileLog("stdout");

        // Initialise PRISM engine
        staminaMC = new StaminaModelChecker();
        staminaMC->initialize();
        //staminaMC->setEngine(Prism.EXPLICIT); Don't think we need this for Storm intergace


    } catch (stormException e) {
        std::cout << "Error: " << e.what() << std::endl;
        std::exit(1);
    }
}



void StaminaCL::processOptions() {

    try {

        // Configure options
        if (reachabilityThreshold >= 0.0 )	StaminaOptions::setReachabilityThreshold(reachabilityThreshold);
        if (kappaReductionFactor >= 0.0 )	StaminaOptions::setKappaReductionFactor(kappaReductionFactor);
        if (maxApproxCount >= 0) StaminaOptions::setMaxRefinementCount(maxApproxCount);
        if (probErrorWindow >= 0.0) StaminaOptions::setProbErrorWindow(probErrorWindow);
        StaminaOptions::setRankTransitions(rankTransitions);

        StaminaOptions::setNoPropRefine(noPropRefine);

        if (maxLinearSolnIter >= 0) staminaMC->setMaxIters(maxLinearSolnIter);


        //Need to discuss what solution method means in the context of Storm

        /*if (solutionMethod != NULL) {

            if (solutionMethod.compare("power") == 0) {
                staminaMC->setEngine(Prism.POWER);
            }
            else if (solutionMethod.compare("jacobi") == 0) {
                staminaMC->setEngine(Prism.JACOBI);
            }
            else if (solutionMethod.compare("gaussseidel") == 0) {
                staminaMC->setEngine(Prism.GAUSSSEIDEL);
            }
            else if (solutionMethod.compare("bgaussseidel") == 0) {
                staminaMC->setEngine(Prism.BGAUSSSEIDEL);
            }
        }*/

        staminaMC->loadPRISMModel(modulesFile);



    } catch (stormException const& e) {
        std::cout << "Error: " << e.what() << std::endl;
        std::exit(1);
    }
}


void StaminaCL::doParsing(int argc, char* argv[]) {

    parseArguments(argc, argv);
    parseModelProperties();

}

void StaminaCL::parseArguments(int argc, char* argv[]) {

    std::string sw;



    for (int i=1; i<argc; i++) {

        // if is a switch...
        if (argv[i][0] == '-') {

            // Remove "-"
            sw = &argv[i][1];

            if (sw.length() == 0) {
                errorAndExit("Invalid empty switch");
            }

            if (sw.compare("kappa") == 0) {

                if (i < argc - 1) {
                    reachabilityThreshold = std::stod(argv[++i]);
                }
                else {
                    std::cout << "reachabilityThreshold(kappa) not defined." << std::endl;
                }

            }
            else if (sw.compare("reducekappa") == 0) {

                if (i < argc - 1) {
                    kappaReductionFactor = std::stod(argv[++i]);
                }
                else {
                    std::cout << "kappaReductionFactor not defined." << std::endl;
                }

            }
            else if (sw.compare("pbwin") == 0) {

                if (i < argc - 1) {
                    probErrorWindow = std::stod(argv[++i]);
                }
                else {
                    std::cout << "Probability error window not given." << std::endl;
                }

            }
            else if (sw.compare("noproprefine") == 0) {

                noPropRefine = true;

            }
            else if (sw.compare("maxapproxcount") == 0) {

                maxApproxCount = std::stoi(argv[++i]);

            }
            else if (sw.compare("maxiters") == 0) {

                maxLinearSolnIter = std::stoi(argv[++i]);

            }
            else if (sw.compare("power") == 0 || sw.compare("jacobi") == 0 || sw.compare("gaussseidel") == 0 || sw.compare("bgaussseidel") ==0 ) {

                solutionMethod = argv[++i];

            }
            else if (sw.compare("const") == 0) {
                std::string temp;
                if (i < argc - 1) {
                    // store argument for later use (append if already partially specified)

                    if (constSwitch.empty() == 0)
                        constSwitch = trim(argv[++i]);
                    else
                        temp = ",";
                    constSwitch += temp + trim(argv[++i]);
                }
            }
            else if (sw.compare("rankTransitions") == 0)
            {
                rankTransitions = true;
            }
            else {
                printHelp();
                exit();
            }

        }
            // otherwise argument must be a filename
        else if (((modelFilename.empty()) && endsWith(argv[i], dotPrism)) || endsWith(argv[i], dotSm)) {
            modelFilename = argv[i];
        }
        else if ((propertiesFilename.empty() && endsWith(argv[i], dotCsl))) {
            propertiesFilename = argv[i];
        }
            // anything else - must be something wrong with command line syntax
        else {
            errorAndExit("Invalid argument syntax");
        }

    }
}

void StaminaCL::parseModelProperties(){

    std::vector<storm::jani::Property> propertiesToCheck;

    try {
        // Parse and load a PRISM model from a file
       // delete &modulesFile;
        modulesFile = staminaMC->parseModelFile(modelFilename);

        // Parse and load a properties model for the model
        //delete &propertiesVector;
        propertiesVector = staminaMC->parsePropertiesFile(modulesFile, propertiesFilename);


        numPropertiesToCheck = (int) propertiesVector.size();


    } catch (FileNotFoundException const& e) {
        std::cout << "Error: " << e.what() << std::endl;
        std::exit(1);
    } catch (stormException const& e) {
        std::cout << "Error: " << e.what() << std::endl;
        std::exit(1);
    }

}

bool StaminaCL::endsWith(std::string const& original, std::string const& ending) {
    if (original.length() >= ending.length()) {
        return (original.compare(original.length() - ending.length(), ending.length(), ending) == 0);
    }
    else {
        return false;
    }
}

char* StaminaCL::trim(char* input) {

    for(int i = 0; input[i]== ' '; input++);

    for(int i = 0; input[i]!= '\0'; i++) {
        if (input[i] == ' ') {
            char *check = &input[i];
            while(*check == ' ') {
                check++;
            }
            if(*check == '\0') {
                input[i] = '\0';
                return input;
            }
        }
    }
    return input;
}

void StaminaCL::printHelp() {
    std::cout << "Usage: stamina <model-file> <properties-file> [options]" << std::endl;
    std::cout << std::endl ;
    std::cout << "<model-file> .................... Prism model file. Extensions: .prism, .sm" << std::endl ;
    std::cout << "<properties-file> ............... Property file. Extensions: .csl" << std::endl ;
    std::cout << std::endl ;
    std::cout << "Options:" << std::endl ;
    std::cout << "========" << std::endl ;
    std::cout << std::endl ;
    std::cout << "-kappa <k>.......................... ReachabilityThreshold [default: 1.0e-6]" << std::endl ;
    std::cout << "-reducekappa <f>.................... Reduction factor for ReachabilityThreshold(kappa << std::endl  for refinement step.  [default: 1000.0]" << std::endl ;
    std::cout << "-pbwin <e>.......................... Probability window between lower and upperbound for termination. [default: 1.0e-3]" << std::endl ;
    std::cout << "-maxapproxcount <n>................. Maximum number of approximation iteration. [default: 10]" << std::endl ;
    std::cout << "-noproprefine ...................... Do not use property based refinement. If given, model exploration method will reduce the kappa and do the property independent refinement. [default: off]" << std::endl ;
    std::cout << "-const <vals> ...................... Comma separated values for constants" << std::endl ;
    std::cout << "\tExamples:" << std::endl ;
    std::cout << "\t-const a=1,b=5.6,c=true" << std::endl ;
    std::cout << std::endl ;
    std::cout << "Other Options:" << std::endl ;
    std::cout << "========" << std::endl ;
    std::cout << std::endl ;
    std::cout << "-rankTransitions ................... Rank transitions before expanding. [default: false]" << std::endl ;
    std::cout << "-maxiters <n> ...................... Maximum iteration for solution. [default: 10000]" << std::endl ;
    std::cout << "-power ............................. Power method" << std::endl ;
    std::cout << "-jacobi ............................ Jacobi method" << std::endl ;
    std::cout << "-gaussseidel ....................... Gauss-Seidel method" << std::endl ;
    std::cout << "-bgaussseidel ...................... Backward Gauss-Seidel method" << std::endl ;
    std::cout << std::endl ;
}

void StaminaCL::exit()
{
    /*staminaMC.closeDown();
    mainLog.flush();
    System.exit(1);*/
    std::cout << "Exited" << std::endl;
    std::exit(0);
}

/**
 * Report a (fatal) error and exit cleanly (with exit code 1).
 */
void StaminaCL::errorAndExit(std::string s)
{
    /*staminaMC.closeDown();
    std::cout << "\nError: " + s + ".");
    mainLog.flush();*/
    std::cout << "Error and Exited" << std::endl;
    std::exit(1);
}
