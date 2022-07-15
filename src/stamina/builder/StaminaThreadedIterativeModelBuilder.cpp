#include "StaminaThreadedIterativeModelBuilder.h"

namespace stamina {
namespace builder {

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType>::StaminaThreadedIterativeModelBuilder(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, storm::prism::Program const& modulesFile
	, storm::generator::NextStateGeneratorOptions const & options
) : StaminaIterativeModelBuilder(
		generator
		, modulesFile
		, options
	)

{
	// Intentionally left empty
}

StaminaThreadedIterativeModelBuilder(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
) : StaminaThreadedIterativeModelBuilder(
		program
		, generatorOptions
	)
{
	// Intentionally left empty
}

} // namespace builder
} // namespace stamina
