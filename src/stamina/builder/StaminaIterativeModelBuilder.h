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

		private:

		};
	}
}
#endif // STAMINA_ITERATIVE_MODEL_BUILDER_H
