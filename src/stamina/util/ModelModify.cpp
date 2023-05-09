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

}


storm::jani::Property
ModelModify::modifyProperty(
	storm::jani::Property prop
	, bool isMin
) {
	storm::logic::Formula formula = prop.getRawFormula();
	try {
		assert(formula.isProbabilityOperatorFormula());
		// This is a path formula
		auto pathFormula = formula.getSubformula();
		auto stateFormula = pathFormula.getRightSubformula();
		// At this point, formula should be a state formula
		assert(stateFormula.isStateFormula());
		auto opType = storm::logic::BinaryBooleanOperatorType::And
			? isMin : storm::logic::BinaryBooleanOperatorType::Or;
		storm::logic::AtomicLabelFormula absorbing("Absorbing");
		if (isMin) {
			storm::logic::UnaryBooleanStateFormula nt(
				storm::logic::UnaryBooleanOperatorType::Not
				, absorbing
			);
			storm::logic::BinaryBooleanStateFormula newFormula(
				opType
				// Left formula
				, nt
				// Right Formula
				, stateFormula
			);
			pathFormula.setRightSubformula(newFormula);
		}
		else {
			storm::logic::BinaryBooleanStateFormula newFormula(
				opType
				// Left formula
				, absorbing
				// Right Formula
				, stateFormula
			);
			pathFormula.setRightSubformula(newFormula);
		}
		formula.setSubFormula(pathFormula);
		prop.setFormula(formula);
		return prop;
	}
	catch (std::exception e) {
		// TODO: handle
	}
}
