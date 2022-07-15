/**
 * There are some benefits if only using one thread to using just the StaminaModelBuilder
 * class or StaminaIterativeModelBuilder class. This class is used if and only if the threadcount
 * is greater than 1.
 *
 * Simply including threading makes it so that there is some nontrivial overhead in exploration,
 * and we want to provide the users the option without having them have to have a control thread
 * idling and waiting for the exploration thread.
 *
 * Created by Josh Jeppson on Jul 15, 2022
 * */
#ifndef STAMINA_BUILDER_STAMINATHREADEDITERATIVEMODELBUILDER_H
#define STAMINA_BUILDER_STAMINATHREADEDITERATIVEMODELBUILDER_H

#include "StaminaModelBuilder.h"

namespace stamina {
	namespace builder {
		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		class StaminaThreadedIterativeModelBuilder : public StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType> {
		public:

		private:
		};
		// "Custom" deleter (which actually is not custom) to allow for polymorphic shared pointers
		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		void __delete_stamina_iterative_model_builder(StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType> * t) { delete t; }
	}
}

#endif // STAMINA_BUILDER_STAMINATHREADEDITERATIVEMODELBUILDER_H
