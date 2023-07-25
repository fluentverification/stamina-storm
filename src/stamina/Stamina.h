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

#ifndef STAMINA_STAMINA
#define STAMINA_STAMINA

#include <iostream>
#include <ostream>
#include <functional>

#include "StaminaArgParse.h"
#include "core/StaminaModelChecker.h"
#include "core/Options.h"

#include <storm/api/storm.h>
#include <storm-parsers/api/storm-parsers.h>
#include <storm-parsers/parser/PrismParser.h>
#include <storm/storage/prism/Program.h>
#include <storm/storage/jani/Property.h>
#include <storm/modelchecker/results/CheckResult.h>
#include <storm/modelchecker/results/ExplicitQuantitativeCheckResult.h>

#include "util/ModelModify.h"

// #include <storm/utility/initialize.h>

namespace stamina {

	/* MAIN STAMINA CLASS */
	class Stamina {
	public:
		/**
		* Main construtor: creates an instance of the STAMINA class
		*
		* @param arguments Arguments struct from StaminaArgParse
		* */
		Stamina(struct arguments * arguments);
		Stamina();
		/**
		* Destructor. Cleans up memory
		* */
		~Stamina();
		/**
		* Runs stamina
		* */
		void run(bool rebuild=false);
		/* Data Members */
		/**
		* Initializes Stamina
		* */
		void initialize();
		void reInitialize();
		std::vector<core::StaminaModelChecker::ResultTableRow> & getResultTable() { return this->modelChecker->getResultTable(); }
		uint64_t getStateCount() { return modelChecker->getStateCount(); }
		std::shared_ptr<storm::prism::Program> getModelFile() { return modelFile; }
		uint64_t getTransitionCount() { return modelChecker->getTransitionCount(); }
		std::shared_ptr<std::vector<std::pair<std::string, uint64_t>>> getLabelsAndCount() {
			return modelChecker->getLabelsAndCount();
		}
		/* Data Members */
		std::shared_ptr<core::StaminaModelChecker> modelChecker;
		std::shared_ptr<storm::prism::Program> modelFile;
		std::shared_ptr<std::vector<storm::jani::Property>> propertiesVector;
		std::shared_ptr<util::ModelModify> modelModify;
		bool wasInitialized;
	};
}

#endif // STAMINA_STAMINA
