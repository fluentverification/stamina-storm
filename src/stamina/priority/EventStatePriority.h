/**
 * Event State Priority Class
 * Created by Josh Jeppson on 1/11/2023
 *
 * Creates a metric based on whether we are looking for a rare event or a commen event based on
 * the state distance to the threshold.
 * */
#ifndef STAMINA_PRIORITY_EVENTSTATEPRIORITY_H
#define STAMINA_PRIORITY_EVENTSTATEPRIORITY_H

#include "StatePriority.h"

namespace stamina {
	namespace priority {
		template <typename StateType>
		class EventStatePriority : public StatePriority<StateType> {
		public:
			virtual static float priority(std::shared_ptr<ProbabilityStatePair<StateType>> state);
			virtual bool operatorValue(
				const std::shared_ptr<ProbabilityStatePair<StateType>> first
				, const std::shared_ptr<ProbabilityStatePair<StateType>> second
			);
		private:
			bool rareEvent;
		};
	}
}
#endif // STAMINA_PRIORITY_EVENTSTATEPRIORITY_H
