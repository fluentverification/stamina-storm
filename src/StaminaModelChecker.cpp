/**
 * Implementation for methods for StaminaModelChecker
 * 
 * Created by Josh Jeppson on 8/17/2021
 * */
#include "StaminaModelChecker.h"
#include "ANSIColors.h"

#include <sstream>
#include <stdio.h>
#include <fstream>
#include <chrono>

using namespace stamina;

StaminaModelChecker::StaminaModelChecker(        
    std::function<void(std::string)> err
    , std::function<void(std::string)> warn
    , std::function<void(std::string)> info
    , std::function<void(std::string)> good
    , Options * options
    , storm::prism::Program * modulesFile
    , std::vector<storm::jani::Property> * propertiesVector
) : err(err)
    , warn(warn)
    , info(info)
    , good(good)
    , options(options)
    , modulesFile(modulesFile)
    , propertiesVector(propertiesVector)
{
    // Explicitly invoke model build
    std::string iFilename = options->import_filename; 
    if (iFilename != "") {
        try {
            std::string transFile = iFilename + ".tra";
            std::string stateRewardsFile = iFilename + "srew";
            std::string transRewardsFile = iFilename + ".trew";
            std::string statesFile = iFilename + ".sta";
            std::string labelsFile = iFilename + ".lab";
            // TODO: load model from explicit files
            warn("Importing model from explicit files not implemented yet!");
        }
        catch (const std::exception& e) {
            std::stringstream ss;
            ss << "Cannot import file: " << iFilename << '\n';
            ss << BOLD("\tGot Error:") << e.what();
            err(ss.str());
        }  
    }
}

StaminaModelChecker::~StaminaModelChecker() {
    // Explicitly invoke model export
    std::string oFilename = options->export_trans; 
    if (oFilename != "") {
        try {
            std::string transFile = oFilename + ".tra";
            std::string stateRewardsFile = oFilename + "srew";
            std::string transRewardsFile = oFilename + ".trew";
            std::string statesFile = oFilename + ".sta";
            std::string labelsFile = oFilename + ".lab";
            // TODO: export model to explicit files
            warn("Exporting model to explicit files not implemented yet!");
        }
        catch (const std::exception& e) {
            std::stringstream ss;
            ss << "Cannot export file: " << oFilename << '\n';
            ss << BOLD("\tGot Error:") << e.what();
            err(ss.str());
        }  
    }
    // Clean up memory
    delete builder;
    
}

void
StaminaModelChecker::initialize(
    storm::prism::Program * modulesFile
    , std::vector<storm::jani::Property> * propertiesVector
) {
    // Don't pass in nullptr please
    if (!modulesFile) {
        err("Modules file cannot be null!");
        std::exit(1);
    }
    else if (!propertiesVector) {
        err("Properties vector cannot be null!");
        std::exit(1);
    }
    this->modulesFile = modulesFile;
    this->propertiesVector = propertiesVector;
    // Create PrismNextStateGenerator. May need to create a NextStateGeneratorOptions for it if default is not working
    auto generator = std::make_shared<storm::generator::PrismNextStateGenerator<double, uint32_t>>(*modulesFile);
    // Create StaminaModelBuilder
    builder = new StaminaModelBuilder<double>(
        options
        , err
        , warn
        , info
        , good
        , generator
    );
}

std::unique_ptr<storm::modelchecker::CheckResult> 
StaminaModelChecker::modelCheckProperty(
    storm::jani::Property prop
    , storm::prism::Program const& modulesFile
) {
    auto startTime = std::chrono::high_resolution_clock::now();
    // Instantiate lower and upper results
    min_results = new Result();
    max_results = new Result();

    // Create number of refined iterations and rechability threshold
    int numRefineIterations = 0;
    double reachThreshold = options->kappa;

    // Get the name of the property
    std::string propName = prop.getName().empty() ? "Prob" : prop.getName();

    // Lower and upper bound times
    double lTime, uTime;

    // Split property into 2 to find P_min and P_max
    storm::jani::Property * prop_min = new storm::jani::Property(propName + "_min", prop.getRawFormula(), prop.getUndefinedConstants()); // todo: modify true
    storm::jani::Property * prop_max = new storm::jani::Property(propName + "_max", prop.getRawFormula(), prop.getUndefinedConstants()); // todo: modify false

    auto propertyExpression = prop.getRawFormula();

    if (propertyExpression->isProbabilityPathFormula()) {
        warn("This property (" + propName + ") is a probability path formula.");
    }

    // Will we sitch to optimized CTMC analysis?
    bool switchToCombinedCTMC = false;
    // Check if we are using an until formula and a path formula
    std::shared_ptr<const storm::logic::Formula> formula = prop.getFilter().getFormula();
    if (formula->isPathFormula() && formula->isUntilFormula()) {
        // TODO: Set this property for our model generator
        // Update switchToCombinedCTMC
        switchToCombinedCTMC = switchToCombinedCTMC || !options->no_prop_refine;
    }
    switchToCombinedCTMC = switchToCombinedCTMC && !options->no_prop_refine;

    // While we should not terminate
    while (numRefineIterations == 0
        || (!terminateModelCheck() && numRefineIterations < options->max_approx_count)
    ) {
        // Print out our current refinement iteration
        info("Approximation [Refine Iterations: " + std::to_string(numRefineIterations) + ", kappa = " + std::to_string(reachThreshold) + "]");
        // Reset the reachability threshold
        reachThreshold = options->kappa;


        // If we don't switch to a combined CTMC, just perform the model checking
        if (!switchToCombinedCTMC) {
            info("Verifying lower bound for " + prop_min->getName() + ".");
            check(prop_min, min_results);
            info("Verifying upper bound for " + prop_max->getName() + ".");
            check(prop_max, max_results);
            // Write to output file
            // TODO: write to output file
            // Update here since we continue to the next loop iteration
            ++numRefineIterations;
            continue;
        }

        // Reduce kappa for refinement
        double percentOff = max_results->result - min_results->result;
        percentOff *= (double) 4.0 / options->prob_win;
        // max percent off at 100%
        if (percentOff > 1.0) {
            percentOff = 1.0;
        }
        options->approx_factor *= percentOff;

        // Increment the refinement count
        if (options->export_perimeter_states != "") {
            writePerimeterStates(numRefineIterations);    
        }

        // Increment number of refine iterations
        ++numRefineIterations;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> timeTaken = endTime - startTime;
    std::stringstream ss;
    ss << "Taken total time: " << timeTaken.count() << " s\n";
    info(ss.str());

    // Print results
    std::stringstream resultInfo;
    resultInfo << "Finished checking property: " << propName << std::endl;
    resultInfo << "\t" << BOLD(FMAG("Probability Minimum: ")) << *min_results << std::endl;
    resultInfo << "\t" << BOLD(FMAG("Probability Maximum: ")) << *max_results << std::endl;
    info(resultInfo.str());
    
    // Export transitions to file if desired
    if (options->export_trans != "") {
        info("Exporting transitions to file: " + options->export_trans);
        printTransitionActions(options->export_trans);
        good("Export Complete!");
    }

    // Clean up memory
    delete min_results;
    delete max_results;
    delete prop_min;
    delete prop_max;
    return nullptr;
}

void 
StaminaModelChecker::check(storm::jani::Property * property, StaminaModelChecker::Result * r) {
    warn("This method is not implemented yet!!");
    double result = 0.0;
    r->result = result;
    r->explanation = "Property check for " + property->getName();
}

bool 
StaminaModelChecker::terminateModelCheck() {
    // If our max result minus our min result is less than our maximum window
    return (max_results->result - min_results->result) <= options->prob_win;

}

void 
StaminaModelChecker::writePerimeterStates(int numRefineIteration) {
    try {
        // Refer to line 450 in StaminaModelChecker.java
        warn("writePerimeterStates(int numRefineIteration) has not been implemented yet!");
    }
    catch(const std::exception& e) {
        std::stringstream ss;
        ss << "Got error while trying to export perimeter states: " << e.what();
        err(ss.str());
    }
}

void 
StaminaModelChecker::printTransitionActions(std::string filename) {
    warn("This feature has not been implemented yet");
}

void
StaminaModelChecker::writeToOutput(std::string filename) {
    if (!min_results || !max_results) {
        err("A results pointer is a nullptr");
        return;
    }
    std::ofstream outfile;
    outfile.open(filename);
    if (!outfile) {
        err("Output file could not be opened");
        return;
    }
    outfile << min_results << "\r\n";
    outfile << max_results << "\r\n";
    outfile.close();
}