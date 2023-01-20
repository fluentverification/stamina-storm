/**
 * Stamina State Priority Class
 * Created by Josh Jeppson on 1/11/2023
 *
 * Pure virtual abstract class that creates a priority metric on the state passed in
 * */
#ifndef STAMINA_PRIORITY_STATEPRIORITY_H
#define STAMINA_PRIORITY_STATEPRIORITY_H

#include <cstdint>

#include "builder/__storm_needed_for_builder.h"

#include "core/Options.h"
#include "core/StaminaMessages.h"

#include "builder/ProbabilityState.h"
#include "builder/StateAndTransitions.h"

#include <storm/storage/jani/Property.h>

namespace stamina {
	namespace priority {
		typedef builder::CompressedState CompressedState;

		template <typename StateType>
		class StatePriority {
		public:
			virtual float priority(std::shared_ptr<builder::ProbabilityStatePair<StateType>> state) = 0;
			virtual bool operatorValue(
				const std::shared_ptr<builder::ProbabilityStatePair<StateType>> first
				, const std::shared_ptr<builder::ProbabilityStatePair<StateType>> second
			) = 0;
			static void setupStatePriority();
			virtual void initialize(storm::jani::Property * property);
			/* Data members */
			// static StatePriority<StateType> statePriority;
			static constexpr storm::jani::Property * property = nullptr;
		};
	} // namespace priority
} // namespace stamina

#endif // STAMINA_PRIORITY_STATE_PRIORITY_H
