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

#include "ThreadsafePrismNextStateGenerator.h"

#include "core/StaminaMessages.h"

namespace stamina {
namespace threadsafe {
namespace generator {

template<typename ValueType, typename StateType = uint32_t>
ThreadsafePrismNextStateGenerator<ValueType, StateType>::ThreadsafePrismNextStateGenerator(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& options = NextStateGeneratorOptions()
	, std::shared_ptr<ActionMask<ValueType, StateType>> const& = nullptr
) : PrismNextStateGenerator<ValueType, StateType>(program, options)
	, varInfoWasSetUp(false)
{
	// intentionally left empty
}

template<typename ValueType, typename StateType = uint32_t>
storm::storage::sparse::StateValuations
ThreadsafePrismNextStateGenerator<ValueType, StateType>::makeObservationValuation() const {
	if (varInfoWasSetUp) {
		StaminaMessages::warning("Cannot make observation valuation");
	}
}

template<typename ValueType, typename StateType = uint32_t>
void
ThreadsafePrismNextStateGenerator<ValueType, StateType>::addStateValuation(
	storm::storage::sparse::state_type const& currentStateIndex
	, storm::storage::sparse::StateValuationsBuilder& valuationsBuilder
) const {

}

template class ThreadsafePrismNextStateGenerator<double>;

} // namespace generator
} // namespace threadsafe
} // namespace stamina


