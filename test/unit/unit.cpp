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
#include <stamina/util/StateIndexArray.h>
#include <stamina/util/StateMemoryPool.h>
#include <stamina/builder/ProbabilityState.h>

#include <storm-parsers/parser/FormulaParser.h>

namespace bt = boost::unit_test;
using namespace stamina;
using namespace stamina::util;
using namespace stamina::builder;

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
}

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

