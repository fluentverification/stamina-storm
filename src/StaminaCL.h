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
    static int numPropertiesToCheck;
    static std::vector<storm::jani::Property> propertiesToCheck;
    static const std::string dotPrism;
    static const std::string dotSm;

    static const std::string dotCsl;

//////////////////////// Command line options ///////////////////////

// argument to -const switch
    static std::string constSwitch;

//Probabilistic state search termination value : Defined by kappa in command line argument
    static double reachabilityThreshold;


// Kappa reduction factor
    static double kappaReductionFactor;


// max number of refinement count
    static int maxApproxCount;


// termination Error window
    static double probErrorWindow;


// Use property based refinement
    static bool noPropRefine;


// Use property to explore by highest rank transition
    static bool rankTransitions;



//////////////////////////////////// Command lines args to pass to prism ///////////////////
// Solutions method max iteration
    static int maxLinearSolnIter;


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
    static void parseModelProperties();


};


#endif //STAMINA_STAMINACL_H
