// Since we must work with the filesystem, we need the CWD
#ifdef WINDOWS
	#include <direct.h>
	#define getcwd _getcwd
	#define SLASH '\\'
#else
	#include <unistd.h>
	#define SLASH '/'
#endif // WINDOWS

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <regex>

#include <boost/algorithm/string/trim.hpp>
#include "storm/storage/expressions/RationalLiteralExpression.h"

#include <stdio.h> // For remove()

#include "ModelModify.h"

#include "core/StaminaMessages.h"

using namespace stamina;
using namespace stamina::util;

ModelModify::ModelModify(
	std::string model
	, std::string properties
) : model(model)
	, properties(properties)
{
	// Ensure we are actually given a file path
	if (model == "") {
		StaminaMessages::error("Model file path was \"\"!");
	}
	if (properties == "") {
		StaminaMessages::error("Properties file path was \"\"!");
	}
}

ModelModify::~ModelModify() {
	// intentionally left empty
}

std::shared_ptr<storm::prism::Program>
ModelModify::readModel() {
	this->modelFile = std::make_shared<storm::prism::Program>(storm::parser::PrismParser::parse(model, true));
	return this->modelFile;
}

std::shared_ptr<std::vector<storm::jani::Property>>
ModelModify::createPropertiesList(
	std::shared_ptr<storm::prism::Program> modelFile
) {
	auto propertiesVector = std::make_shared<std::vector<storm::jani::Property>>(storm::api::parsePropertiesForPrismProgram(properties, *modelFile));
	return propertiesVector;
}


storm::jani::Property
ModelModify::modifyProperty(
	storm::jani::Property prop
	, bool isMin
) {
	try {
		std::shared_ptr<const storm::logic::Formula> formulaAbs = prop.getRawFormula();
		assert(formulaAbs->isProbabilityOperatorFormula());
		std::shared_ptr<const storm::logic::ProbabilityOperatorFormula> formula
			= std::static_pointer_cast<const storm::logic::ProbabilityOperatorFormula>(formulaAbs);
		if (!formula) {
			StaminaMessages::errorAndExit("Could not convert formula to ProbabilityOperatorFormula!");
		}
		// This is a path formula
		const storm::logic::BoundedUntilFormula & pathFormula
			= static_cast<const storm::logic::BoundedUntilFormula &>(formula->getSubformula());
		const storm::logic::StateFormula & stateFormula
			= static_cast<const storm::logic::StateFormula &>(pathFormula.getRightSubformula());
		// Assert that the path formula is an until formula
		if (!pathFormula.isUntilFormula()) {
			StaminaMessages::warning("Formula \"" + pathFormula.toString() + "\" was not an until formula");
		}
		// At this point, formula should be a state formula
		assert(stateFormula.isStateFormula());
		// If isMin, chose And, otherwise, choose or
		auto opType = isMin ?
			storm::logic::BinaryBooleanOperatorType::And
			: storm::logic::BinaryBooleanOperatorType::Or;
		std::shared_ptr<storm::logic::AtomicLabelFormula> absorbing(
				new storm::logic::AtomicLabelFormula("Absorbing"));
		// use std::make_shared ?
		std::shared_ptr<storm::logic::StateFormula> stateFormulaPointer
			= std::dynamic_pointer_cast<storm::logic::StateFormula>(stateFormula.clone());
		std::shared_ptr<storm::logic::Formula const> pathFormulaPtr(nullptr);
		// Create lower and upper bounds from the known information
		auto upper = pathFormula.getUpperBound();
		if (!pathFormula.hasUpperBound()) {
			StaminaMessages::errorAndExit("Needs upper bound!");
		}
		// auto lower = pathFormula.getLowerBound();
		std::shared_ptr<storm::expressions::BaseExpression> zeroBase(new storm::expressions::RationalLiteralExpression(upper.getManager(), 0.0));
		storm::expressions::Expression zeroLiteral(zeroBase);
		storm::expressions::Expression & zero = zeroLiteral;
		storm::expressions::Expression & lower = zero;
		if (pathFormula.hasLowerBound()) {
			lower = pathFormula.getLowerBound();
		}
		storm::logic::TimeBound lowerBound(true, lower);
		storm::logic::TimeBound upperBound(true, upper);
		// TODO: add for Steps if DTMC
		storm::logic::TimeBoundReference timeBoundReference(storm::logic::TimeBoundType::Time);
		if (isMin) {
			std::shared_ptr<storm::logic::UnaryBooleanStateFormula> nt(
				new storm::logic::UnaryBooleanStateFormula(
					storm::logic::UnaryBooleanOperatorType::Not
					, absorbing
				)
			);
			storm::logic::BinaryBooleanStateFormula newFormula(
				opType
				// Left formula
				, nt
				// Right Formula
				, stateFormulaPointer
			);
			pathFormulaPtr = storm::logic::BoundedUntilFormula(
				// Left subformula from the existing path formula
				pathFormula.getLeftSubformula().clone()
				// New right-hand formula from above
				, newFormula.clone()
				// The lower bound from pathFormula
				, lowerBound
				// The upper bound from the pathFormula
				, upperBound
				// The time bound reference (basically whether it's steps, time, or reward)
				, timeBoundReference
			).clone();
		}
		else {
			storm::logic::BinaryBooleanStateFormula newFormula(
				opType
				// Left formula
				, absorbing
				// Right Formula
				, stateFormulaPointer
			);
			pathFormulaPtr = storm::logic::BoundedUntilFormula(
				// Left subformula from the existing path formula
				pathFormula.getLeftSubformula().clone()
				// New right-hand formula from above
				, newFormula.clone()
				// The lower bound from pathFormula
				, lowerBound
				// The upper bound from the pathFormula
				, upperBound
				// The time bound reference (basically whether it's steps, time, or reward)
				, timeBoundReference
			).clone();
		}
		// Get operator information from formula and set it to newFormula
		std::shared_ptr<storm::logic::Formula> newFormula(
			new storm::logic::ProbabilityOperatorFormula(pathFormulaPtr, formula->getOperatorInformation())
		);
		// formula->setSubFormula(pathFormula);
		std::string name = isMin ? "_min" : "_max";
		storm::jani::Property newProp(prop.getName() + name, newFormula, prop.getUndefinedConstants(), "Added by STAMINA");
		return newProp;
	}
	catch (std::exception e) {
		StaminaMessages::errorAndExit("Caught Error while trying to modify property!");
	}
}


void
ModelModify::setModelAndProperties(
	std::string model
	, std::string properties
) {
	this->model = model;
	this->properties = properties;
}

std::string
ModelModify::getModel() {
	return this->model;
}

std::string
ModelModify::getProperties() {
	return this->properties;
}
