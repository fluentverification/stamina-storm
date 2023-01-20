/**
 * Implementation of State Priority methods
 *
 * Created by Josh Jeppson on 1/11/2023
 * */
#include "StatePriority.h"

namespace stamina {
namespace priority {

template <typename StateType>
void
StatePriority<StateType>::setupStatePriority() {
	// TODO
}

template <typename StateType>
void
StatePriority<StateType>::initialize(storm::jani::Property * property) {
	StaminaMessages::warning("initialize() does nothing in this instance!");
}

// Forward declare EventStatePriority class
template class StatePriority<uint32_t>;

} // namespace priority
} // namespace stamina
