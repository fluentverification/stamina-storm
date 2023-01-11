/**
 * Stamina State Priority Class
 * Created by Josh Jeppson on 1/11/2023
 *
 * Pure virtual abstract class that creates a priority metric on the state passed in
 * */
#ifndef STAMINA_PRIORITY_STATEPRIORITY_H
#define STAMINA_PRIORITY_STATEPRIORITY_H

#include <cstdint>

#include "core/Options.h"
#include "core/StaminaMessages.h"

#include "builder/ProbabilityState.h"
#include "builder/StateAndTransitions.h"

namespace stamina {
	namespace priority {
		template <typename StateType>
		class StatePriority {
		public:
			virtual static float priority(std::shared_ptr<ProbabilityStatePair<StateType>> state) = 0;
			virtual bool operatorValue(
				const std::shared_ptr<ProbabilityStatePair<StateType>> first
				, const std::shared_ptr<ProbabilityStatePair<StateType>> second
			) = 0;
			static void setupStatePriority();
			/* Data members */
			static StatePriority<StateType> statePriority;
		};
	} // namespace priority
} // namespace stamina

#endif // STAMINA_PRIORITY_STATE_PRIORITY_H
