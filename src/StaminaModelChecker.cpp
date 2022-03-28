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
#include <utility>
#include <unordered_set>

#define USE_STAMINA_TRUNCATION

#ifndef USE_STAMINA_TRUNCATION
	#include "ExplicitTruncatedModelBuilder.h"
#endif // USE_STAMINA_TRUNCATION

using namespace stamina;

StaminaModelChecker::StaminaModelChecker(
	std::shared_ptr<storm::prism::Program> modulesFile
	, std::shared_ptr<std::vector<storm::jani::Property>> propertiesVector
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
}

void
StaminaModelChecker::initialize(
	std::shared_ptr<storm::prism::Program> modulesFile
	, std::shared_ptr<std::vector<storm::jani::Property>> propertiesVector
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
	// Create allocators for shared pointers
	std::allocator<Result> allocatorResult;
	std::allocator<StaminaModelBuilder<double>> allocatorBuilder;
	// Create PrismNextStateGenerator. May need to create a NextStateGeneratorOptions for it if default is not working
	auto options = BuilderOptions(*prop.getFilter().getFormula());
	auto generator = std::make_shared<storm::generator::PrismNextStateGenerator<double, uint32_t>>(modulesFile, options);
	// Create StaminaModelBuilder
	builder = std::allocate_shared<StaminaModelBuilder<double>> (allocatorBuilder, generator);

	auto startTime = std::chrono::high_resolution_clock::now();
	// Instantiate lower and upper results
	min_results = std::allocate_shared<Result>(allocatorResult);
	max_results = std::allocate_shared<Result>(allocatorResult);

	// Create number of refined iterations and rechability threshold
	int numRefineIterations = 0;
	double reachThreshold = Options::kappa;

	// Get the name of the property
	std::string propName = prop.getName().empty() ? "Prob" : prop.getName();

	// Lower and upper bound times
	double lTime, uTime;

	// Split property into 2 to find P_min and P_max
	auto prop_min = createModifiedProperty(prop, false);
	auto prop_max = createModifiedProperty(prop, true);
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
			StaminaMessages::debugPrint("The following is the labeling information for the built model:");
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
		/* Naive truncation using JUST a breadth first search rather than truncating paths
		based on reachability probability */
		StaminaMessages::info("Using test truncation");
		auto simpleBuilder = new ExplicitTruncatedModelBuilder<double>(generator);
		model = simpleBuilder->build()->template as<storm::models::sparse::Ctmc<double>>();
		auto labeling = model->getStateLabeling();
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

	return nullptr;
}

void
StaminaModelChecker::check(std::shared_ptr<storm::jani::Property> property, std::shared_ptr<StaminaModelChecker::Result> r) {
	StaminaMessages::warning("This method (StaminaModelChecker::check()) is not implemented yet! This method DOES NOT perform truncated model checking, rather, it builds the entire model.");
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
		// StaminaMessages::warning("writePerimeterStates(int numRefineIteration) has not been implemented yet!");
		// Open the outfile
		std::ofstream out;
		out.open(Options::export_perimeter_states.c_str());
		// Write to it:
		out << numRefineIteration << " Refinement Iterations: ";
		auto perimeterStates = builder->getPerimeterStates();
		for (auto state : perimeterStates) {
			out << state << " ";
		}
		out << "\n";
		out.close();
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
	storm::jani::Property & baseProperty
	, bool isMax
) {
	std::allocator<storm::jani::Property> allocator;
	std::default_delete<storm::jani::Property> del;
	// Get the name of the property
	std::string propName = baseProperty.getName().empty() ? "UNNAMED_PROPERTY" : baseProperty.getName();
	auto formula = baseProperty.getRawFormula();
	std::string formulaString = formula->toString();
	auto phi = formula->toExpression(expressionManager);
	storm::expressions::BinaryRelationExpression absorbing(
		expressionManager // The expression manager
		, phi->getType() // The expression type
		, // The first operand. TODO: should be state index
		, absorbingStateIndex  	// The second operand (here, is 0 since that's the absorbing state index)
		, BinaryRelationExpression::RelationType::Equal // The relation between the two operands
	); // create expression with having the "absorbing" label (aka an index of 0)
	std::string suffix;
	auto newExpression = phi;
	/*
	 * Minimum formula is equal to (phi) and not (absorbing)
	 */
	if (!isMax) {
		newExpression = (phi) && !(absorbing);
		suffix = "_min";
	}
	/*
	 * Maximum formula is equal to (phi) or (absorbing)
	 */
	else {
		newExpression = (phi) || (absorbing);
		suffix = "_max";
	}
	// TODO: create new formula from modified expression
	// std::function<storm::expressions::Expression(storm::expressions::Expression const&)> remapping;
	auto remapping = [](
		storm::expressions::Expression const& inputExpression
		, storm::expressions::Expression const & newExpression = newExpression
	) {
		return newExpression;
	};
	auto newFormula = formula->substitute(
		/*
			This is the method I think we need to use. There are several different types of
			parameters that it takes and I'm not sure which one is best.

			1. Takes a std::map<storm::expression::Variable, storm::expressions::Expression>
			which substitutes all variables within that map with the mapped expressions

			2. Takes a std::function<storm::expressions::Expression(storm::expressions::Expression const&)>
			(aka a function binding which takes an expression and returns a new expression)

			3. Takes a std::map<std::string, storm::expressions::Expression> (substitutes labels for
			expressions)

			4. Takes a std::map<std::string, std::string> (substitutes labels for other labels)
		*/
		remapping;
	);
	auto prop = std::allocate_shared<storm::jani::Property> (
		allocator
		, storm::jani::Property(propName + suffix, newFormula, baseProperty.getUndefinedConstants())
	);
	return prop;
}
