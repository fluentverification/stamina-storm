#include "StaminaNextStateGenerator.h"

using namespace stamina;

template<typename ValueType, typename StateType>
StateBehavior<ValueType, StateType>
StaminaNextStateGenerator<ValueType, StateType>::expand(StateToIdCallback const& stateToIdCallback) {

}

template<typename ValueType, typename StateType>
std::vector<Choice<ValueType>>
StaminaNextStateGenerator<ValueType, StateType>::getAsynchronousChoices(
	CompressedState const& state
	, StateToIdCallback stateToIdCallback
	, CommandFilter const& commandFilter = CommandFilter::All
) {

}

template<typename ValueType, typename StateType>
void
StaminaNextStateGenerator<ValueType, StateType>::addSynchronousChoices(
	std::vector<Choice<ValueType>>& choices
	, CompressedState const& state
	, StateToIdCallback stateToIdCallback
	, CommandFilter const& commandFilter = CommandFilter::All
) {

}
