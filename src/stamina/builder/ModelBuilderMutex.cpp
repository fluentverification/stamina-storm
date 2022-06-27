namespace stamina {
namespace builder {

template <typename StateType>
ModelBuilderMutex<StateType>::ModelBuilderMutex()
{
	// Intentionally left empty
}

template <typename StateType>
bool
ModelBuilderMutex<StateType>::stateIsInUse(StateType stateId) {
	std::shared_lock<std::shared_mutex> lock(write);
	bool inUse = inUseStates.find(stateId) != inUseStates.end();
	return inUse;
}

template <typename StateType>
void
ModelBuilderMutex<StateType>::setStateInUse(StateType stateId, bool inUse) {
	std::lock_guard<std::shared_mutex> lock(write);
	if (inUse) {
		inUseStates.insert(stateId);
	}
	else {
		inUseStates.remove(stateId);
	}
}

} // namespace builder
} // namespace stamina

