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

} // namespace builder
} // namespace stamina
