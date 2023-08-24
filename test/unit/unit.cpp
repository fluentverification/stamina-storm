/**
 * STAMINA - the [ST]ochasic [A]pproximate [M]odel-checker for [IN]finite-state [A]nalysis
 * Copyright (C) 2023 Fluent Verification, Utah State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 *
 **/

/********************************************************************
 * Unit tests for STAMINA/Storm
 * Licensed under the GPLv3 license -- provided with no warranty or liability.
 * File created by Josh Jeppson on May 15, 2023
 ********************************************************************/

#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <string>
#include <cstdint>

#include <stamina/util/ModelModify.h>

namespace bt = boost::unit_test;
using namespace stamina::util;

// =======================================================================================
// This unit test checks to see if the ModelModify creates the right modified properties
// and parses the property correctly.
// =======================================================================================
BOOST_AUTO_TEST_CASE( ModelModify_Basic ) {
	std::string modelFile = "../test/models/simple.prism";
	std::string propFile = "../test/models/simple.csl";
	ModelModify mod(modelFile, propFile);
	auto loadedModelFile = mod.readModel();
	// Model file should not be null
	BOOST_TEST( loadedModelFile != nullptr);
	auto propVector = mod.createPropertiesList(loadedModelFile);
	// There should only be one properties vector
	BOOST_TEST( propVector->size() == 1 );
	// Create modified prop min and prop max
	auto propOriginal = (*propVector)[0];
	auto propMin = mod.modifyProperty(propOriginal, true);
	auto propMax = mod.modifyProperty(propOriginal, false);
	// Test that the properties look correct (in prism syntax)
	// auto propMinString = propMin.asPrismSyntax();
	// // I wish there were some way to parse the property from the string and compare two
	// // instances of storm::jani::Property
	// BOOST_TEST(
	// 	propMinString == "\"1_min\": P=? [true U[0,1] ((!\"Absorbing\") & (First < 20) & (Second >= 20))]; // Added by STAMINA"
	// );
	// auto propMaxString = propMax.asPrismSyntax();
	// BOOST_TEST(
	// 	propMaxString == "\"1_max\": P=? [true U[0,1] (\"Absorbing\" | (First < 20) & (Second >= 20))]; // Added by STAMINA"
	// );
}
// =======================================================================================

