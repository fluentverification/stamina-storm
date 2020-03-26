//
// Created by Riley Layne Roberts on 2/19/20.
//

#ifndef STAMINA_STAMINACL_H
#define STAMINA_STAMINACL_H


#include "options.h"
#include "StaminaModelChecker.h"
#include "storm/storage/prism/Constant.h"

#include "storm/exceptions/FileIoException.h"
#include "storm/parser/CSVParser.h"
#include "storm/utility/cli.h"
#include "storm/modelchecker/results/CheckResult.h"

class StaminaCL {


typedef storm::exceptions::FileIoException FileNotFoundException;


private:
    static std::vector<std::reference_wrapper<storm::prism::Constant const>> undefinedMFConstants;


    static std::string modelFilename;
    static std::string propertiesFilename;

    static storm::prism::Program modulesFile;
    static std::vector<storm::jani::Property> propertiesVector;

    // info about which properties to model check
    static int numPropertiesToCheck = 0;
    static std::vector<storm::jani::Property> propertiesToCheck = NULL;
    static const std::string dotPrism = ".prism";
    static const std::string dotSm = ".sm";
    static const std::string dotCsl = ".csl";
//////////////////////// Command line options ///////////////////////

// argument to -const switch
    static std::string constSwitch;

//Probabilistic state search termination value : Defined by kappa in command line argument
    static double reachabilityThreshold = -1.0;

// Kappa reduction factor
    static double kappaReductionFactor = -1;

// max number of refinement count
    static int maxApproxCount = -1;

// termination Error window
    static double probErrorWindow = -1.0;

// Use property based refinement
    static bool noPropRefine = false;

// Use property to explore by highest rank transition
    static bool rankTransitions = false;


//////////////////////////////////// Command lines args to pass to prism ///////////////////
// Solutions method max iteration
    static int maxLinearSolnIter = -1;

// Solution method
    static std::string solutionMethod;

    static StaminaModelChecker* staminaMC;

public:
    static void errorAndExit(std::string s);
    static void exit();
    static void parseArguments(int argc, char* argv[]);
    static char* trim(char* input);
    static void printHelp();
    static bool endsWith(std::string const& original, std::string const& ending);
    static void run(int argv, char* argc[]);
    static void initializeSTAMINA();
    static void processOptions();
    static void doParsing(int argc, char* argv[]);
    static void parseModelProperties()


};

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
        storm::modelchecker::CheckResult results[numPropertiesToCheck];

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
                continue;
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

    } catch (stormException e) {
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
        if (reachabilityThreshold >= 0.0 )	Options::setReachabilityThreshold(reachabilityThreshold);
        if (kappaReductionFactor >= 0.0 )	Options::setKappaReductionFactor(kappaReductionFactor);
        if (maxApproxCount >= 0) Options::setMaxRefinementCount(maxApproxCount);
        if (probErrorWindow >= 0.0) Options::setProbErrorWindow(probErrorWindow);
        Options::setRankTransitions(rankTransitions);

        Options::setNoPropRefine(noPropRefine);

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

        constSwitch = "";

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

                    if (i < argc - 1) {
                        // store argument for later use (append if already partially specified)
                        std::string temp = "";
                        if (temp.compare(constSwitch) == 0)
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
            modulesFile = staminaMC->parseModelFile(modelFilename);


            // Parse and load a properties model for the model
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

#endif //STAMINA_STAMINACL_H
