#include "StaminaIterativeModelBuilder.h"

namespace stamina {
namespace builder {

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaIterativeModelBuilder<ValueType, StateType, RewardModelType>::StaminaIterativeModelBuilder(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, storm::prism::Program const& modulesFile
	, storm::generator::NextStateGeneratorOptions const & options
) // Invoke super constructor
	: StaminaModelBuilder<ValueType, RewardModelType, StateType>(
		generator
		, modulesFile
		, options
	)
{
	// Intentionally left empty
}

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaIterativeModelBuilder<ValueType, StateType, RewardModelType>::StaminaIterativeModelBuilder(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
) // Invoke super constructor
	: StaminaModelBuilder<ValueType, RewardModelType, StateType>(
		program
		, generatorOptions
	)
{
	// Intentionally left empty
}

} // namespace builder
} // namespace stamina
