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
	// Intentionally left empty
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
	// TODO: Add in the modified properties
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
		const storm::logic::BinaryPathFormula & pathFormula = static_cast<const storm::logic::BinaryPathFormula &>(formula->getSubformula());
		const storm::logic::StateFormula & stateFormula = static_cast<const storm::logic::StateFormula &>(pathFormula.getRightSubformula());
		// At this point, formula should be a state formula
		assert(stateFormula.isStateFormula());
		// If isMin, chose And, otherwise, choose or
		auto opType = isMin ?
			storm::logic::BinaryBooleanOperatorType::And
			: storm::logic::BinaryBooleanOperatorType::Or;
		std::shared_ptr<storm::logic::AtomicLabelFormula> absorbing(
				new storm::logic::AtomicLabelFormula("Absorbing"));
		// use std::make_shared ?
		std::shared_ptr<storm::logic::StateFormula> stateFormulaPointer(
			new storm::logic::StateFormula(stateFormula)
		);
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
			pathFormula.setRightSubformula(newFormula);
		}
		else {
			storm::logic::BinaryBooleanStateFormula newFormula(
				opType
				// Left formula
				, absorbing
				// Right Formula
				, stateFormulaPointer
			);
			pathFormula.setRightSubformula(newFormula);
		}
		auto pathFormulaPtr = std::make_shared<storm::logic::Formula const>(pathFormula);
		// TODO: Get operator information from formula and set it to newFormula
		std::shared_ptr<storm::logic::Formula> newFormula(
			new storm::logic::ProbabilityOperatorFormula(pathFormulaPtr)
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
