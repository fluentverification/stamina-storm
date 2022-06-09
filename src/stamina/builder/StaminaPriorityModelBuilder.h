#ifndef STAMINA_PRIORITY_MODEL_BUILDER_H
#define STAMINA_PRIORITY_MODEL_BUILDER_H

/**
 * The model builder class which implements the STAMINA 3.0 algorithm which uses a priority queue
 * on the estimated reachability
 *
 * Created by Josh Jeppson on Jun 9, 2021
 * */

#include "StaminaModelBuilder.h"

namespace stamina {
	namespace builder {
		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		class StaminaPriorityModelBuilder : protected StaminaModelBuilder<ValueType, RewardModelType, StateType> {
		public:
			/**
			* Constructs a StaminaPriorityModelBuilder with a given storm::generator::PrismNextStateGenerator. Invokes super's constructor
			*
			* @param generator The generator we are going to use.
			* */
			StaminaPriorityModelBuilder(
				std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
				, storm::prism::Program const& modulesFile
				, storm::generator::NextStateGeneratorOptions const & options
			);
			/**
			* Constructs a StaminaPriorityModelBuilder with a PRISM program and generatorOptions. Invokes super's constructor
			*
			* @param program The PRISM program we are going to use to build the model with.
			* @param generatorOptions Options for the storm::generator::PrismNextStateGenerator we are going to use.
			* */
			StaminaPriorityModelBuilder(
				storm::prism::Program const& program
				, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
			);
		private:

		};
	}
}
#endif // STAMINA_PRIORITY_MODEL_BUILDER_H
