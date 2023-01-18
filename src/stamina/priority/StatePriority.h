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

#include <storm/storage/jani/Property.h>

namespace stamina {
	namespace priority {
		template <typename StateType>
		class StatePriority {
		public:
			virtual float priority(std::shared_ptr<builder::ProbabilityStatePair<StateType>> state) = 0;
			virtual bool operatorValue(
				const std::shared_ptr<builder::ProbabilityStatePair<StateType>> first
				, const std::shared_ptr<builder::ProbabilityStatePair<StateType>> second
			);
			static void setupStatePriority();
			/* Data members */
			static StatePriority<StateType> statePriority;
			static storm::jani::Property * property = nullptr;
		};
	} // namespace priority
} // namespace stamina

#endif // STAMINA_PRIORITY_STATE_PRIORITY_H
