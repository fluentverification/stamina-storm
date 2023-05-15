/********************************************************************
 * Unit tests for STAMINA/Storm
 * Licensed under the MIT license -- provided with no warranty or liability.
 * File created by Josh Jeppson on May 15, 2023
 ********************************************************************/

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <string>
#include <cstdint>

namespace bt = boost::unit_test;

// =======================================================================================
// This unit test checks to see if the ModelModify creates the right modified properties
// and parses the property correctly.
// =======================================================================================
BOOST_AUTO_TEST_CASE( ModelModify_Basic ) {
	std::string modelFile = "../models/simple.prism";
	std::string propFile = "../models/simple.csl";
	ModelModify mod(modelFile, propFile);
	auto modelFile = mod.readModel();
	auto propVector = mod.createPropertiesList(modelFile);
	// There should only be one properties vector
	BOOST_TEST( propVector.size() == 1 );
	// Create modified prop min and prop max
	auto propOriginal = propVector[0];
	auto propMin = mod.modifyProperty(propOriginal, true);
	auto propMax = mod.modifyProperty(propOriginal, false);
	// Test that the properties look correct (in prism syntax)
	auto propMinString = propMin.asPrismSyntax();
	// I wish there were some way to parse the property from the string and compare two
	// instances of storm::jani::Property
	BOOST_TEST(
		propMinString == "P=? [true U[0,1] ((!\"Absorbing\") & (First < 20) & (Second >= 20))]; // Added by STAMINA"
	);
	auto propMaxString = propMax.asPrismSyntax();
	BOOST_TEST(
		propMaxString == "P=? [true U[0,1] (\"Absorbing\" | (First < 20) & (Second >= 20))]; // Added by STAMINA"
	);
}
// =======================================================================================
