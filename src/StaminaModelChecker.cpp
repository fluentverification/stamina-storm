/**
 * Implementation for methods for StaminaModelChecker
 * 
 * Created by Josh Jeppson on 8/17/2021
 * */
#include "StaminaModelChecker.h"
#include "ANSIColors.h"

#include <sstream>
#include <stdio.h>

using namespace stamina;

StaminaModelChecker::StaminaModelChecker(        
    std::function<void(std::string)> err
    , std::function<void(std::string)> warn
    , std::function<void(std::string)> info
    , std::function<void(std::string)> good
    , Options * options
) : err(err)
    , warn(warn)
    , info(info)
    , good(good)
    , options(options)
{
    // Intentionally left empty
}

std::unique_ptr<storm::modelchecker::CheckResult> 
StaminaModelChecker::modelCheckProperty(
    storm::jani::Property prop
    , storm::prism::Program const& modulesFile
) {
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

    // Will we sitch to optimized CTMC analysis?
    bool switchToCombinedCTMC = false;

    auto propertyExpression = prop.getRawFormula();

    if (propertyExpression->isProbabilityPathFormula()) {
        warn("This property (" + propName + ") is a probability path formula.");
    }

    // While we should not terminate
    while (numRefineIterations == 0
        || (!terminateModelCheck() && numRefineIterations < options->max_approx_count)
    ) {
        // Print out our current refinement iteration
        info("Approximation [Refine Iterations: " + std::to_string(numRefineIterations) + ", kappa = " + std::to_string(reachThreshold) + "]");
        // Reset the reachability threshold
        reachThreshold = options->kappa;

        // Check if we are using an until formula and a path formula
        std::shared_ptr<storm::logic::Formula> formula = prop.getFilter().getFormula();
        if (formula.isPathFormula() && formula.isUntilFormula()) {
            // TODO: Set this property for our model generator
            // Update switchToCombinedCTMC
            switchToCombinedCTMC = switchToCombinedCTMC || !options->no_prop_refine;
        }
        switchToCombinedCTMC = switchToCombinedCTMC && !options->no_prop_refine;

        // If we don't switch to a combined CTMC, just perform the model checking
        if (!switchToCombinedCTMC) {
            info("Verifying lower bound for " + prop_min->getName() + ".");
            check(prop_min, min_results);
            info("Verifying upper bound for " + prop_max->getName() + ".");
            check(prop_max, max_results);
            // Update here since we continue to the next loop iteration
            ++numRefineIterations;
            continue;
        }

        // Reduce kappa for refinement
        double percentOff = max_results->result - min_results->result;
        percentOff *= (double) 4.0 / options->prob_win;
        if (percentOff > 1.0) {
            // info("percentOff is equal to: " + std::to_string(percentOff));
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



    // Print results
    std::stringstream resultInfo;
    resultInfo << "Finished checking property: " << propName << std::endl;
    resultInfo << "\t" << BOLD(FMAG("Probability Minimum: ")) << *min_results << std::endl;
    resultInfo << "\t" << BOLD(FMAG("Probability Maximum: ")) << *max_results << std::endl;
    info(resultInfo.str());
    
    // Export transitions to file if desired
    if (options->export_trans != "") {
        info("Exporting transitions to file: " + options->export_trans);
        // TODO: Export 
        warn("This feature has not been implemented yet");
        good("Export Complete!");
    }

    // Clean up memory
    delete max_results;
    delete prop_min;
    delete prop_max;
    // min_results is returned so stamina::Stamina should delete this when done.
    return min_results;
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