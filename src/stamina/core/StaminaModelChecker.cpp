/**
* Implementation for methods for StaminaModelChecker
*
* Created by Josh Jeppson on 8/17/2021
* */
#include "StaminaModelChecker.h"
#include "ANSIColors.h"
#include "StaminaMessages.h"

#include "core/StateSpaceInformation.h"

#include "storm/environment/Environment.h"
#include "storm/builder/BuilderOptions.h"
#include "storm/storage/expressions/BinaryRelationExpression.h"

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

namespace stamina {
namespace core {

using namespace stamina::builder;

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
	storm::jani::Property propMin
	, storm::jani::Property propMax
	, storm::prism::Program const& modulesFile
) {
	// Create allocators for shared pointers
	std::allocator<Result> allocatorResult;
	// auto options = BuilderOptions(*propMin.getFilter().getFormula());
	auto options = BuilderOptions(*(storm::api::parsePropertiesForPrismProgram("P=? [ true U[0,1000] (YFP_protein <= 30) ]", *(this->modulesFile))[0].getRawFormula()));
	// Create PrismNextStateGenerator. May need to create a NextStateGeneratorOptions for it if default is not working
	auto generator = std::make_shared<storm::generator::PrismNextStateGenerator<double, uint32_t>>(modulesFile, options);
	StateSpaceInformation::setVariableInformation(generator->getVariableInformation());
	if (Options::method == STAMINA_METHODS::ITERATIVE_METHOD) {
		// The reason that this splits into two separate classes is that when calling STAMINA
		// as a single-threaded application there is less overhead to use just StaminaIterativeModelBuilder
		// rather than the threaded version
		if (Options::threads == 1) {
			// Create StaminaModelBuilder
			auto builderPointer = std::make_shared<StaminaIterativeModelBuilder<double>> (generator, modulesFile, options);
			builder = std::static_pointer_cast<StaminaModelBuilder<double>>(builderPointer);
		}
		else {
			StaminaMessages::info("Using thread-count: " + std::to_string(Options::threads));
			auto builderPointer = std::make_shared<StaminaThreadedIterativeModelBuilder<double>> (generator, modulesFile, options);
			builder = std::static_pointer_cast<StaminaModelBuilder<double>>(builderPointer);
			std::vector<std::shared_ptr<storm::generator::PrismNextStateGenerator<double, uint32_t>>> generators;
			for (int i = 0; i < Options::threads; i++) {
				generators.push_back(std::make_shared<storm::generator::PrismNextStateGenerator<double, uint32_t>>(modulesFile, options));
			}
			// Give to model builder.
			//
			// This must be builderPointer because when we pointer-cast to a
			// std::shared_ptr<StaminaModelBuilder> we lose the knowledge that this is a
			// StaminaThreadedIterativeModelBuilder, which has this method.
			builderPointer->setGeneratorsVector(generators);
		}
	}
	else if (Options::method == STAMINA_METHODS::PRIORITY_METHOD) {
		StaminaMessages::warning("Not fully implemented yet!");
		// Create StaminaModelBuilder
		auto builderPointer = std::make_shared<StaminaPriorityModelBuilder<double>> (generator, modulesFile, options);
		builderPointer->initializeEventStatePriority(&propMin);
		builder = std::static_pointer_cast<StaminaModelBuilder<double>>(builderPointer);
	}
	else if (Options::method == STAMINA_METHODS::RE_EXPLORING_METHOD) {
		if (Options::threads != 1) {
			StaminaMessages::error("The re-exploring method (STAMINA 2.0) does not support multithreading!");
		}
		auto builderPointer = std::make_shared<StaminaReExploringModelBuilder<double>> (generator, modulesFile, options);
		builder = std::static_pointer_cast<StaminaModelBuilder<double>>(builderPointer);
	}
	else {
		StaminaMessages::errorAndExit("Truncation method is invalid!");
	}

	auto startTime = std::chrono::high_resolution_clock::now();
	auto modelTime = startTime;
	// Instantiate lower and upper results
	min_results = std::allocate_shared<Result>(allocatorResult);
	max_results = std::allocate_shared<Result>(allocatorResult);

	// Create number of refined iterations and reachability threshold
	int numRefineIterations = 0;
	double reachThreshold = Options::kappa;

	// Property refinement optimization
	if (!Options::no_prop_refine) {
		// Get the expression for the current property
		auto propertyFormula = propMin.getRawFormula();
		StaminaMessages::info("Attempting to convert formula to expression:\n\t" + propertyFormula->toString());
		if ((!propertyFormula->isPathFormula())
			&& (
				propertyFormula->isAtomicExpressionFormula()
				|| propertyFormula->isBinaryBooleanStateFormula()
				|| propertyFormula->isBooleanLiteralFormula()
				|| propertyFormula->isUnaryBooleanStateFormula()
			)
		) {
			builder->setPropertyFormula(propertyFormula, modulesFile);
		}
	}

	// While we should not terminate
	// All versions of the STAMINA algorithm (except for the heuristic version use refinement iterations)
	while (numRefineIterations == 0
		|| (!terminateModelCheck() && numRefineIterations < Options::max_approx_count)
	) {
		// Print out our current refinement iteration
		StaminaMessages::info("Approximation [Refine Iterations: " + std::to_string(numRefineIterations) + ", kappa = " + std::to_string(reachThreshold) + "]");
		// Reset the reachability threshold
		reachThreshold = Options::kappa;

		std::shared_ptr<CtmcModelChecker> checker = nullptr;
		std::shared_ptr<storm::models::sparse::Ctmc<double, storm::models::sparse::StandardRewardModel<double>>> model;
		model = builder->build()->template as<storm::models::sparse::Ctmc<double>>();


		// Rebuild the initial state labels
		labeling = &( model->getStateLabeling());

		std::cout << "Labeling:\n" << model->getStateLabeling() << std::endl;

		checker = std::make_shared<CtmcModelChecker>(*model);

		builder->setLocalKappaToGlobal();
		modelTime = std::chrono::high_resolution_clock::now();
		// Instruct STORM to compute P_min and P_max
		// We will need to get info from the terminal states
		try {
			// storm::Environment env;
			// env.solver().native().setPrecision(storm::utility::convertNumber<storm::RationalNumber>(1e-9));
			auto result_lower = checker->check(
				// env,
				storm::modelchecker::CheckTask<>(*(propMin.getRawFormula()), true)
			);
			min_results->result = result_lower->asExplicitQuantitativeCheckResult<double>()[*model->getInitialStates().begin()];
			auto result_upper = checker->check(
				// env,
				storm::modelchecker::CheckTask<>(*(propMax.getRawFormula()), true)
			);
			max_results->result = result_upper->asExplicitQuantitativeCheckResult<double>()[*model->getInitialStates().begin()];
			builder->printStateSpaceInformation();
			StaminaMessages::info(std::string("At this refine iteration, the following result values are found:\n") +
				"\tMinimum Results: " + std::to_string(min_results->result) + "\n" +
				"\tMaximum Results: " + std::to_string(max_results->result) + "\n"  +
				"This gives us a window of " + std::to_string(max_results->result - min_results->result)
			);
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

	// Export transitions to file if desired
	if (Options::export_trans != "") {
		StaminaMessages::info("Exporting transitions to file: " + Options::export_trans);
		builder->printTransitionActions();
		StaminaMessages::good("Export Complete!");
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> timeTaken = endTime - startTime;
	std::chrono::duration<double> timeTakenModel = modelTime - startTime;
	std::chrono::duration<double> timeTakenCheck = endTime - modelTime;
	std::stringstream ss;
	ss.setf( std::ios::floatfield );
	ss << std::fixed << std::setprecision(12);
	ss << "The following summary shows the time for each step:" << std::endl;
	ss << "\tTime taken for model building: " << timeTakenModel.count() << " s\n";
	ss << "\tTime taken for model checking: " << timeTakenCheck.count() << " s\n";
	ss << "\tTaken total time: " << timeTaken.count() << " s\n";
	StaminaMessages::info(ss.str());

	// Print results
	std::stringstream resultInfo;
	resultInfo.setf( std::ios::floatfield );
	resultInfo << std::fixed << std::setprecision(12);
	resultInfo << "Finished checking property: " << propMin.getName() << std::endl;
	resultInfo << "\t" << BOLD(FMAG("Probability Minimum: ")) << min_results->result << std::endl;
	resultInfo << "\t" << BOLD(FMAG("Probability Maximum: ")) << max_results->result << std::endl;
	StaminaMessages::info(resultInfo.str());


	return nullptr;
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
StaminaModelChecker::printTransitionActions(std::string const & filename) {
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


void
StaminaModelChecker::modifyState(bool isMin) {
	StaminaMessages::warning("Currently STAMINA only supports \"true U\" formulas. Other formulas may return inaccurate results");
	for (std::string label : labeling->getLabels() ) {
		if (label == "absorbing" || label == "deadlock" || label == "init") { continue; }
		//else if (label == /* TODO: label is "a" (the label before the until */) {
		//	continue;
		//}
		else if (isMin && labeling->getStateHasLabel(label, absorbingStateIndex)) {
			// Remove all labels except for, "deadlock", and "absorbing" from
			// the absorbing state
			labeling->removeLabelFromState(label, absorbingStateIndex);
		}
		else if (!isMin) {

			labeling->addLabelToState(label, absorbingStateIndex);
		}
	}
}

} // namespace core
} // namespace stamina
