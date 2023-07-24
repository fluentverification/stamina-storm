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

#include "StaminaThreadedPriorityModelBuilder.h"

namespace stamina {
namespace builder {

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaThreadedPriorityModelBuilder<ValueType, RewardModelType, StateType>::StaminaThreadedPriorityModelBuilder(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, storm::prism::Program const& modulesFile
	, storm::generator::NextStateGeneratorOptions const & options
) : StaminaPriorityModelBuilder(
		generator
		, modulesFile
		, options
	)

{
	// Intentionally left empty
}

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaThreadedPriorityModelBuilder(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
) : StaminaThreadedPriorityModelBuilder(
		program
		, generatorOptions
	)
{
	// Intentionally left empty
}

template class StaminaThreadedPriorityModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace builder
} // namespace stamina
