#include "EventStatePriority.h"

namespace stamina {
namespace priority {

template <typename StateType>
float
EventStatePriority<StateType>::priority(std::shared_ptr<ProbabilityStatePair<StateType>> state) {

}

template <typename StateType>
bool
EventStatePriority<StateType>::operatorValue(
	const std::shared_ptr<ProbabilityStatePair<StateType>> first
	, const std::shared_ptr<ProbabilityStatePair<StateType>> second
) {

}

} // namespace priority
} // namespace stamina
