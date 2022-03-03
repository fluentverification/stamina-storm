#include <storm/api/storm.h>
#include <storm-parsers/api/storm-parsers.h>
#include <storm-parsers/parser/PrismParser.h>
#include <storm/storage/prism/Program.h>
#include <storm/storage/jani/Property.h>
#include <storm/modelchecker/results/CheckResult.h>
#include <storm/modelchecker/results/ExplicitQuantitativeCheckResult.h>
#include <storm/generator/PrismNextStateGenerator.h>

#include <storm/utility/initialize.h>

#include "../../src/StaminaModelBuilder.h"

#include <cstdint>

typedef storm::models::sparse::Ctmc<double> Ctmc;
typedef storm::modelchecker::SparseCtmcCslModelChecker<Ctmc> CtmcModelChecker;

bool check(std::string const& path_to_model, std::string const& property_string) {
    // Assumes that the model is in the prism program language format and parses the program.
    auto program = storm::parser::PrismParser::parse(path_to_model);
    // Then parse the properties, passing the program to give context to some potential variables.
    auto properties = storm::api::parsePropertiesForPrismProgram(property_string, program);
    // Translate properties into the more low-level formulae.
    auto formulae = storm::api::extractFormulasFromProperties(properties);

    // Now translate the prism program into a DTMC in the sparse format.
    // Use the formulae to add the correct labelling.
	auto generator = std::make_shared<storm::generator::PrismNextStateGenerator<double,uint32_t>>(program);
    //stamina::StaminaModelBuilder<uint32_t> modelBuilder(generator);
	stamina::StaminaModelBuilder<double> * modelBuilder = new stamina::StaminaModelBuilder<double>(generator);
	auto model = modelBuilder->build()->as<storm::models::sparse::Ctmc<double>>();
	// storm::api::buildSparseModel<double>(program, formulae)->template as<Ctmc>();

    // Create a model checker on top of the sparse engine.
    auto checker = std::make_shared<CtmcModelChecker>(*model);
    // Create a check task with the formula. Run this task with the model checker.
    auto result = checker->check(storm::modelchecker::CheckTask<>(*(formulae[0]), true));
    assert(result->isExplicitQuantitativeCheckResult());
    // Use that we know that the model checker produces an explicit quantitative result
    auto quantRes = result->asExplicitQuantitativeCheckResult<double>();
    // Now compare the result at the first initial state of the model with 0.5.
    return quantRes[*model->getInitialStates().begin()] > 0.5;
}


int main (int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Needs exactly 2 arguments: model file and property" << std::endl;
        return 1;
    }

    // Init loggers
    storm::utility::setUp();
    // Set some settings objects.
    storm::settings::initializeAll("storm-starter-project", "storm-starter-project");

    // Call function
    auto result = check(argv[1], argv[2]);
    // And print result
    std::cout << "Result > 0.5? " << (result ? "yes" : "no") << std::endl;
}
