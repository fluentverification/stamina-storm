#ifndef STAMINA_ITERATIVE_MODEL_BUILDER_H
#define STAMINA_ITERATIVE_MODEL_BUILDER_H

/**
 * The model builder class which implements the STAMINA 2.5 algorithm (STAMINA 2.0/2.1 with dynamic programming)
 *
 * Created by Josh Jeppson on Jun 9, 2021
 * */

#include "StaminaModelBuilder.h"

namespace stamina {
	namespace builder {
		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		class StaminaIterativeModelBuilder : protected StaminaModelBuilder<ValueType, RewardModelType, StateType> {
		public:
			/**
			* Constructs a StaminaIterativeModelBuilder with a given storm::generator::PrismNextStateGenerator. Invokes super's constructor
			*
			* @param generator The generator we are going to use.
			* */
			StaminaIterativeModelBuilder(
				std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
				, storm::prism::Program const& modulesFile
				, storm::generator::NextStateGeneratorOptions const & options
			);
			/**
			* Constructs a StaminaIterativeModelBuilder with a PRISM program and generatorOptions. Invokes super's constructor
			*
			* @param program The PRISM program we are going to use to build the model with.
			* @param generatorOptions Options for the storm::generator::PrismNextStateGenerator we are going to use.
			* */
			StaminaIterativeModelBuilder(
				storm::prism::Program const& program
				, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
			);
		private:

		};
	}
}
#endif // STAMINA_ITERATIVE_MODEL_BUILDER_H
