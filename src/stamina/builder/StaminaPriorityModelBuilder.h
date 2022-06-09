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

		private:

		};
	}
}
#endif // STAMINA_PRIORITY_MODEL_BUILDER_H
