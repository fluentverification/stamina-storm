#include "ControlThread.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename ValueType, typename RewardModelType, typename StateType>
ControlThread<ValueType, RewardModelType, StateType>::ControlThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t numberExplorationThreads
) : BaseThread<ValueType, RewardModelType, StateType>(parent)
	, numberExplorationThreads(numberExplorationThreads)
{
	// Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
std::pair<uint8_t, StateType>
ControlThread<ValueType, RewardModelType, StateType>::requestOwnership(CompressedState & state, uint8_t threadIndex, StateType requestedId) {
	// Test to see if a thread already owns this state.
	// TODO: should this pre-lock even be in here?
	if (stateThreadMap.stateToId.contains(state)) {
		return std::make_pair(
			stateThreadMap.stateToId.getValue(state)
			, this->parent->getStateStorage().stateToId.getValue(state)
		);
	}
	// Lock the ownership mutex
	std::lock_guard<std::shared_mutex> lock(ownershipMutex);
	if (stateThreadMap.stateToId.contains(state)) {
		return std::make_pair(
			stateThreadMap.stateToId.getValue(state)
			, this->parent->getStateStorage().stateToId.getValue(state)
		);
	}
	else {
		// Claim ownership of state
		stateThreadMap.stateToId.findOrAdd(
			state
			, threadIndex
		);
	}
	// Get the state index
	StateType stateIndex = static_cast<StateType>(this->parent->getStateStorage().getNumberOfStates());
	return std::make_pair(threadIndex, stateIndex);
}

template <typename ValueType, typename RewardModelType, typename StateType>
uint8_t
ControlThread<ValueType, RewardModelType, StateType>::whoOwns(CompressedState & state) {
	if (stateThreadMap.stateToId.contains(state)) {
		return stateThreadMap.stateToId.getValue(state);
	}
	// Index 0 (the same index as the absorbing state) indicates that no thread owns this state.
	return 0;
}

template <typename ValueType, typename RewardModelType, typename StateType>
StateType
ControlThread<ValueType, RewardModelType, StateType>::whatIsIndex(CompressedState & state) {
	// Don't need to lock it in this function
	if (this->parent->getStateStorage().stateToId.contains(state)) {
		return this->parent->getStateStorage().stateToId.getValue(state);
	}
	return 0;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ControlThread<ValueType, RewardModelType, StateType>::requestInsertTransition(
	uint8_t thread
	, StateType from
	, StateType to
	, double rate
) {
	LockableDeque tQueue = transitionQueues[thread - 1];
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ControlThread<ValueType, RewardModelType, StateType>::requestCrossExplorationFromThread(
	StateProbability stateAndProbability
	, double threadIndex
) {
	// TODO: implement
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ControlThread<ValueType, RewardModelType, StateType>::mainLoop() {
	uint8_t numberFinishedThreads;
	while (!this->finished || this->hold) { // allow for this thread to be killed outside of its main loop
		bool exitThisIteration = false;
		numberFinishedThreads = 0;
		for (auto explorationThread : this->parent->getExplorationThreads()) {
			if (explorationThread.isIdling()) {
				++numberFinishedThreads;
			}
		}
		if (numberFinishedThreads == this->parent->getExplorationThreads().size()) {
			// We have finished.
			exitThisIteration = true;
		}
		// Make sure that we flush the queues AFTER we determine whether to exit. This prevents a
		// thread from requesting a transition to be added
		for (auto q : transitionQueues) {
			q.lockThread();
			while (!q.empty()) {
				// Request that the parent class
				this->parent->createTransition(q.top());
				q.pop();
			}
			q.unlockThread();
		}
		if (exitThisIteration) { return; }
		// TODO: de-fragmentation
	}
}

template class ControlThread<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace threads
} // namespace builder
} // namespace stamina
