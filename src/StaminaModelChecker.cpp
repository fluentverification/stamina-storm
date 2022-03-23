/**
* Implementation for methods for StaminaModelChecker
*
* Created by Josh Jeppson on 8/17/2021
* */
#include "StaminaModelChecker.h"
#include "ANSIColors.h"
#include "StaminaMessages.h"

#include "storm/builder/BuilderOptions.h"

#include <sstream>
#include <stdio.h>
#include <fstream>
#include <chrono>

#define USE_STAMINA_TRUNCATION

#ifndef USE_STAMINA_TRUNCATION
	#include "ExplicitTruncatedModelBuilder.h"
#endif // USE_STAMINA_TRUNCATION

using namespace stamina;

StaminaModelChecker::StaminaModelChecker(
	storm::prism::Program * modulesFile
	, std::vector<storm::jani::Property> * propertiesVector
) : modulesFile(modulesFile)
	, propertiesVector(propertiesVector)
{
	// Explicitly invoke model build
	std::string iFilename = Options::import_filename;
	if (iFilename != "") {
		StaminaMessages::info("Trying to import files named like " + iFilename);
		try {
			std::string transFile = iFilename + ".tra";
			std::string stateRewardsFile = iFilename + "srew";
			std::string transRewardsFile = iFilename + ".trew";
			std::string statesFile = iFilename + ".sta";
			std::string labelsFile = iFilename + ".lab";
			// TODO: load model from explicit files
			StaminaMessages::warning("Importing model from explicit files not implemented yet!");
		}
		catch (const std::exception& e) {
			std::stringstream ss;
			ss << "Cannot import file: " << iFilename << '\n';
			ss << BOLD("\tGot Error:") << e.what();
			StaminaMessages::error(ss.str());
		}
	}
}

StaminaModelChecker::~StaminaModelChecker() {
	// Explicitly invoke model export
	std::string oFilename = Options::export_filename;
	if (oFilename != "") {
		StaminaMessages::info("Attempting to export model to " + oFilename);
		try {
			std::string transFile = oFilename + ".tra";
			std::string stateRewardsFile = oFilename + "srew";
			std::string transRewardsFile = oFilename + ".trew";
			std::string statesFile = oFilename + ".sta";
			std::string labelsFile = oFilename + ".lab";
			// TODO: export model to explicit files
			StaminaMessages::warning("Exporting model to explicit files not implemented yet!");
		}
		catch (const std::exception& e) {
			std::stringstream ss;
			ss << "Cannot export file: " << oFilename << '\n';
			ss << BOLD("\tGot Error:") << e.what();
			StaminaMessages::error(ss.str());
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
		StaminaMessages::errorAndExit("Modules file cannot be null!");
	}
	else if (!propertiesVector) {
		StaminaMessages::errorAndExit("Properties vector cannot be null!");
	}
	this->modulesFile = modulesFile;
	this->propertiesVector = propertiesVector;
}

std::unique_ptr<storm::modelchecker::CheckResult>
StaminaModelChecker::modelCheckProperty(
	storm::jani::Property prop
	, storm::prism::Program const& modulesFile
) {
	// Create PrismNextStateGenerator. May need to create a NextStateGeneratorOptions for it if default is not working
	auto options = BuilderOptions(*prop.getFilter().getFormula());
	auto generator = std::make_shared<storm::generator::PrismNextStateGenerator<double, uint32_t>>(modulesFile, options);
	// Create StaminaModelBuilder
	builder = new StaminaModelBuilder<double>(generator);

	auto startTime = std::chrono::high_resolution_clock::now();
	// Instantiate lower and upper results
	min_results = new Result();
	max_results = new Result();

	// Get the formulae
	// auto formulae = storm::api::extractFormulasFromProperties(*propertiesVector);

	// Create number of refined iterations and rechability threshold
	int numRefineIterations = 0;
	double reachThreshold = Options::kappa;

	// Get the name of the property
	std::string propName = prop.getName().empty() ? "Prob" : prop.getName();

	// Lower and upper bound times
	double lTime, uTime;

	// Split property into 2 to find P_min and P_max
	storm::jani::Property * prop_min = new storm::jani::Property(propName + "_min", prop.getRawFormula(), prop.getUndefinedConstants());
	storm::jani::Property * prop_max = new storm::jani::Property(propName + "_max", prop.getRawFormula(), prop.getUndefinedConstants());
	// TODO: Get string representation of formula and add
	// For reachability just use the "absorbing" label for the upper bound and add the lower results--if only reachability formulas
	auto propertyExpression = prop.getRawFormula();

	std::cout << propertyExpression->toString() << std::endl;

	if (propertyExpression->isProbabilityPathFormula()) {
		StaminaMessages::warning("This property (" + propName + ") is a probability path formula.");
	}

	// Will we sitch to optimized CTMC analysis? (I.e., will we perform the state-space truncation)
	bool switchToCombinedCTMC = true;
	// Check if we are using an until formula and a path formula
	std::shared_ptr<const storm::logic::Formula> formula = prop.getFilter().getFormula();
	if (formula->isPathFormula() && formula->isUntilFormula()) {
		// TODO: Set this property for our model generator
		// Update switchToCombinedCTMC
		switchToCombinedCTMC = switchToCombinedCTMC || !Options::no_prop_refine;
	}
	switchToCombinedCTMC = switchToCombinedCTMC && !Options::no_prop_refine;

	// While we should not terminate
	while (numRefineIterations == 0
		|| (!terminateModelCheck() && numRefineIterations < Options::max_approx_count)
	) {
		// Print out our current refinement iteration
		StaminaMessages::info("Approximation [Refine Iterations: " + std::to_string(numRefineIterations) + ", kappa = " + std::to_string(reachThreshold) + "]");
		// Reset the reachability threshold
		reachThreshold = Options::kappa;


		// If we don't switch to a combined CTMC, just perform the model checking
		// Without state space truncation
		if (!switchToCombinedCTMC) {
			StaminaMessages::info("Verifying lower bound for " + prop_min->getName() + ".");
			check(prop_min, min_results);
			StaminaMessages::info("Verifying upper bound for " + prop_max->getName() + ".");
			check(prop_max, max_results);
			// Write to output file
			// TODO: write to output file
			// Update here since we continue to the next loop iteration
			++numRefineIterations;
			continue;
		}
		double piHat = 1.0;
		std::shared_ptr<CtmcModelChecker> checker = nullptr;
		std::shared_ptr<storm::models::sparse::Ctmc<double, storm::models::sparse::StandardRewardModel<double>>> model;
#ifdef USE_STAMINA_TRUNCATION
		while (piHat > Options::prob_win / Options::approx_factor) {
			builder->reset();
			model = builder->build()->template as<storm::models::sparse::Ctmc<double>>();
			// Rebuild the initial state labels
			auto labeling = model->getStateLabeling();
			labeling.addLabel("absorbing");
			labeling.addLabelToState("absorbing", 0);
#ifdef DEBUG_PRINTS
			labeling.printLabelingInformationToStream();
#endif
			checker = std::make_shared<CtmcModelChecker>(*model);
			// Accumulate probabilities
			piHat = builder->accumulateProbabilities();
			// NOTE: Kappa reduction taken care of in StaminaModelBuilder::buildMatrices

			generator = std::make_shared<storm::generator::PrismNextStateGenerator<double, uint32_t>>(modulesFile, options);
			builder->setGenerator(generator);
		}
#endif // USE_STAMINA_TRUNCATION
#ifndef USE_STAMINA_TRUNCATION
		StaminaMessages::info("Using test truncation");
		auto simpleBuilder = new ExplicitTruncatedModelBuilder<double>(generator);
		model = simpleBuilder->build()->template as<storm::models::sparse::Ctmc<double>>();
		auto labeling = model->getStateLabeling();
		labeling.addLabel("unknown");
		labeling.addLabelToState("unknown", 1361);
		checker = std::make_shared<CtmcModelChecker>(*model);
		delete simpleBuilder;
#endif
		// Instruct STORM to compute P_min and P_max
		// We will need to get info from the terminal states
		try {
			auto result_lower = checker->check(storm::modelchecker::CheckTask<>(*(prop_min->getRawFormula()), true));
			auto result_upper = checker->check(storm::modelchecker::CheckTask<>(*(prop_max->getRawFormula()), true));
		}
		catch (std::exception& e) {
			StaminaMessages::errorAndExit(e.what());
		}
		double percentOff = max_results->result - min_results->result;
		percentOff *= (double) 4.0 / Options::prob_win;
		// max percent off at 100%
		if (percentOff > 1.0) {
			percentOff = 1.0;
		}
		Options::approx_factor *= percentOff;

		// Increment the refinement count
		if (Options::export_perimeter_states != "") {
			writePerimeterStates(numRefineIterations);
		}

		// Increment number of refine iterations
		++numRefineIterations;
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> timeTaken = endTime - startTime;
	std::stringstream ss;
	ss << "Taken total time: " << timeTaken.count() << " s\n";
	StaminaMessages::info(ss.str());

	// Print results
	std::stringstream resultInfo;
	resultInfo << "Finished checking property: " << propName << std::endl;
	resultInfo << "\t" << BOLD(FMAG("Probability Minimum: ")) << *min_results << std::endl;
	resultInfo << "\t" << BOLD(FMAG("Probability Maximum: ")) << *max_results << std::endl;
	StaminaMessages::info(resultInfo.str());

	// Export transitions to file if desired
	if (Options::export_trans != "") {
		StaminaMessages::info("Exporting transitions to file: " + Options::export_trans);
		printTransitionActions(Options::export_trans);
		StaminaMessages::good("Export Complete!");
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
	StaminaMessages::warning("This method (StaminaModelChecker::check()) is not implemented yet!!");
	double result = 0.0;
	// auto model = builder->build()->as<storm::models::sparse::Ctmc<double>>();
	// auto checker = std::make_shared<CtmcModelChecker>(*model);
	// auto resultClass = checker->check(storm::modelchecker::CheckTask<>(*property, true));
	// result = resultClass->asExplicitQuantitativeCheckResult<double>();
	r->result = result;
	r->explanation = "Property check for " + property->getName();
}

bool
StaminaModelChecker::terminateModelCheck() {
	// If our max result minus our min result is less than our maximum window
	return (max_results->result - min_results->result) <= Options::prob_win;

}

void
StaminaModelChecker::writePerimeterStates(int numRefineIteration) {
	try {
		// Refer to line 450 in StaminaModelChecker.java
		StaminaMessages::warning("writePerimeterStates(int numRefineIteration) has not been implemented yet!");
	}
	catch(const std::exception& e) {
		std::stringstream ss;
		ss << "Got error while trying to export perimeter states: " << e.what();
		StaminaMessages::error(ss.str());
	}
}

void
StaminaModelChecker::printTransitionActions(std::string filename) {
	StaminaMessages::warning("This feature (StaminaModelChecker::printTransitionActions()) has not been implemented yet");
}

void
StaminaModelChecker::writeToOutput(std::string filename) {
	if (!min_results || !max_results) {
		StaminaMessages::error("A results pointer is a nullptr");
		return;
	}
	std::ofstream outfile;
	outfile.open(filename);
	if (!outfile) {
		StaminaMessages::error("Output file could not be opened");
		return;
	}
	outfile << min_results << "\r\n";
	outfile << max_results << "\r\n";
	outfile.close();
}

std::shared_ptr<storm::jani::Property>
StaminaModelChecker::createModifiedProperty(
	std::shared_ptr<storm::jani::Property> baseProperty
	, bool isMax
) {
	std::allocator<storm::jani::Property> allocator;
	std::default_delete<storm::jani::Property> del;
	// Get the name of the property
	std::string propName = baseProperty->getName().empty() ? "UNNAMED_PROPERTY" : baseProperty->getName();
	auto formula = baseProperty->getRawFormula();
	std::string formulaString = formula->toString();
	/*
	 * Minimum formula is equal to (phi) and not (absorbing)
	 */
	if (!isMax) {
// 		formulaString = // TODO
	}
	/*
	 * Maximum formula is equal to (phi) or (absorbing)
	 */
	else {
// 		formulaString = // TODO
	}
	auto prop = std::allocate_shared<storm::jani::Property> (
		allocator
		, storm::jani::Property(propName + "_min", formula, baseProperty->getUndefinedConstants())
	);
	return prop;
}
