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
#include <iostream>

#include <cstring> // For memcmp
#include <cstdint>

#include <stamina/util/ModelModify.h>
#include <stamina/util/StateIndexArray.h>
#include <stamina/util/StateMemoryPool.h>
#include <stamina/builder/ProbabilityState.h>
#include <stamina/core/Options.h>
#include <stamina/Stamina.h>

#include <storm-parsers/parser/FormulaParser.h>

#include "helper.h"

namespace bt = boost::unit_test;
using namespace stamina;
using namespace stamina::util;
using namespace stamina::builder;

using namespace stamina_test;

// =======================================================================================
// A quick test to ensure that helper works correctly
// =======================================================================================

BOOST_AUTO_TEST_CASE( Helper_Tests ) {
#ifndef WINDOWS
	setup_signal_handling();
#endif
	core::Options::quiet = true;
	int foo = 10;
	int * bar = new int;
	*bar = 10;
	BOOST_TEST( pointer_is_valid<int>(&foo) );
	BOOST_TEST( pointer_is_valid<int>(bar) );
	delete bar;
}

// =======================================================================================
// This unit test checks to see if the ModelModify creates the right modified properties
// and parses the property correctly.
// =======================================================================================

BOOST_AUTO_TEST_CASE( ModelModify_Basic ) {
	core::Options::quiet = true;
	std::string modelFile = "../test/models/simple.prism";
	std::string propFile = "../test/models/simple.csl";
	ModelModify mod(modelFile, propFile);
	auto loadedModelFile = mod.readModel();
	// Model file should not be null
	BOOST_TEST( loadedModelFile != nullptr );
	auto propVector = mod.createPropertiesList(loadedModelFile);
	// There should only be one properties vector
	BOOST_TEST( propVector->size() == 1 );
	// Create modified prop min and prop max
	auto propOriginal = (*propVector)[0];
	auto propMin = mod.modifyProperty(propOriginal, true);
	auto propMax = mod.modifyProperty(propOriginal, false);
	storm::parser::FormulaParser parser(*loadedModelFile);

	// we better get probability operator formulas
	BOOST_TEST( propMin.getRawFormula()->isProbabilityOperatorFormula() );
	BOOST_TEST( propMax.getRawFormula()->isProbabilityOperatorFormula() );

	std::shared_ptr<const storm::logic::ProbabilityOperatorFormula> formulaMin
		= std::static_pointer_cast<const storm::logic::ProbabilityOperatorFormula>(propMin.getRawFormula());
	std::shared_ptr<const storm::logic::ProbabilityOperatorFormula> formulaMax
		= std::static_pointer_cast<const storm::logic::ProbabilityOperatorFormula>(propMax.getRawFormula());

	BOOST_TEST( formulaMin != nullptr );
	BOOST_TEST( formulaMax != nullptr );

	// Test expected results
	auto propMinExpected = parser.parseFromString("P=? [true U[0,1] ((!\"Absorbing\") & (First < 20) & (Second >= 20))];")[0];
	auto propMaxExpected = parser.parseFromString("P=? [true U[0,1] (\"Absorbing\" | (First < 20) & (Second >= 20))];")[0];
	// BOOST_TEST(*propMin.getRawFormula() == *propMinExpected.getRawFormula());
	// BOOST_TEST(*propMax.getRawFormula() == *propMaxExpected.getRawFormula());

	// TODO: Actually check expressions
}

// =======================================================================================
// Tests to ensure that the state index array should work as they should.
// =======================================================================================

BOOST_AUTO_TEST_CASE( StateIndexArray_Basic ) {
	StateIndexArray<uint32_t, ProbabilityState<uint32_t>> sia;

	// Don't try to dereference any of these
	ProbabilityState<uint32_t> * one = (ProbabilityState<uint32_t> *) 1;
	ProbabilityState<uint32_t> * two = (ProbabilityState<uint32_t> *) 2;
	ProbabilityState<uint32_t> * three = (ProbabilityState<uint32_t> *) 3;
	ProbabilityState<uint32_t> * mrNullPointer = nullptr;
	sia.put(1, one);
	sia.put(2, two);
	sia.put(3, three);
	sia.put(0, mrNullPointer);

	BOOST_TEST( sia.get(1) == one );
	BOOST_TEST( sia.get(2) == two );
	BOOST_TEST( sia.get(3) == three );

	BOOST_TEST( sia.get(0) == nullptr );
}

// =======================================================================================

BOOST_AUTO_TEST_CASE( StateIndexArray_Advanced ) {
	StateIndexArray<uint32_t, ProbabilityState<uint32_t>> sia;
	StateMemoryPool<ProbabilityState<uint32_t>> memPool;

	// Don't try to dereference any of these
	ProbabilityState<uint32_t> * one = memPool.allocate();
	ProbabilityState<uint32_t> * two = memPool.allocate();
	ProbabilityState<uint32_t> * three = memPool.allocate();
	ProbabilityState<uint32_t> * mrNullPointer = nullptr;
	sia.put(1, one);
	sia.put(2, two);
	sia.put(3, three);
	sia.put(0, mrNullPointer);

	BOOST_TEST( sia.get(1) == one );
	BOOST_TEST( sia.get(2) == two );
	BOOST_TEST( sia.get(3) == three );

	BOOST_TEST( sia.get(0) == nullptr );
}

// =======================================================================================
// Tests to ensure that the StateMemoryPool always returns valid memory
// =======================================================================================

BOOST_AUTO_TEST_CASE( StateMemoryPool_Basic ) {
	StateMemoryPool<ProbabilityState<uint32_t>> memPool;
	const int NUM_TEST = 1000;
	for (int i = 0; i < NUM_TEST; i++) {
		auto pState = memPool.allocate();
		BOOST_TEST( pState != nullptr );
	}
}

// =======================================================================================

BOOST_AUTO_TEST_CASE( StateMemoryPool_Advanced ) {
	StateMemoryPool<ProbabilityState<uint32_t>> memPool;
	const int NUM_TEST = 1000;
	for (int i = 0; i < NUM_TEST; i++) {
		auto pState = memPool.allocate();
		BOOST_TEST( pointer_is_valid<ProbabilityState<uint32_t>>(pState) );
	}
}

// =======================================================================================
// Tests that check the ProbabilityState class
// =======================================================================================

BOOST_AUTO_TEST_CASE( ProbabilityState_Basic ) {
	ProbabilityStateComparison<uint16_t> cmp;
	ProbabilityState<uint16_t> p1(1, 0.5, false, 2);
	ProbabilityState<uint16_t> p2(2, 0.2, false, 3);
	BOOST_TEST( memcmp(&p1, &p2, sizeof(ProbabilityState<uint16_t>)) != 0 );
	BOOST_TEST( !cmp(&p1, &p2) );
	BOOST_TEST( cmp(&p2, &p1) );
	// Now test copy constructor
	p2 = p1;
	BOOST_TEST( !cmp(&p1, &p2) );
	BOOST_TEST( memcmp(&p1, &p2, sizeof(ProbabilityState<uint16_t>)) == 0 );

}

// =======================================================================================

BOOST_AUTO_TEST_CASE( Results_Basic ) {
	set_default_values();
	std::string modelFile = "../test/models/simple.prism";
	std::string propFile = "../test/models/simple.csl";
	core::Options::model_file = modelFile;
	core::Options::properties_file = propFile;
	Stamina s;
	s.run();
	std::cerr << "[WARNING (Unit Tests)] This test not implemented." << std::endl;
	BOOST_TEST( true );
}
