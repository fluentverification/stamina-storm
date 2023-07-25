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

/**
 * Threadsafe drop-in replacement to storm::generator::PrismNextStateGenerator
 * Overrides the following methods:
 *   storm::storage::sparse::StateValuations NextStateGenerator<ValueType, StateType>::makeObservationValuation() const
 *   void NextStateGenerator<ValueType, StateType>::addStateValuation(storm::storage::sparse::state_type const& currentStateIndex, storm::storage::sparse::StateValuationsBuilder& valuationsBuilder) const
 * */
#ifndef STAMINA_THREADSAFE_GENERATOR_THREADSAFEPRISMNEXTSTATEGENERATOR_H
#define STAMINA_THREADSAFE_GENERATOR_THREADSAFEPRISMNEXTSTATEGENERATOR_H

#include <mutex>

#include "storm/generator/PrismNextStateGenerator.h"

namespace stamina {
	namespace threadsafe {
		namespace generator {

			template<typename ValueType, typename StateType = uint32_t>
			class ThreadsafePrismNextStateGenerator : public storm::generator::PrismNextStateGenerator<ValueType, StateType> {
			public:
				ThreadsafePrismNextStateGenerator(
					storm::prism::Program const& program
					, storm::generator::NextStateGeneratorOptions const& options = NextStateGeneratorOptions()
					, std::shared_ptr<ActionMask<ValueType, StateType>> const& = nullptr
				);
				storm::storage::sparse::StateValuations makeObservationValuation() const override;
				void addStateValuation(
					storm::storage::sparse::state_type const& currentStateIndex
					, storm::storage::sparse::StateValuationsBuilder& valuationsBuilder
				) const override;
			private:
				std::mutex variableInformationMutex;
				// Variable information should be set up *before* calls to expand()
				bool varInfoWasSetUp;
			};
		} // namespace generator
	} // namespace threadsafe
} // namespace stamina

#endif // STAMINA_THREADSAFE_GENERATOR_THREADSAFEPRISMNEXTSTATEGENERATOR_H

