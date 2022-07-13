#include "ControlThread.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename RewardModelType, typename ValueType>
ControlThread<StateType, RewardModelType, ValueType>::ControlThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t numberExplorationThreads
) : BaseThread<StateType, RewardModelType, ValueType>(parent)
	, numberExplorationThreads(numberExplorationThreads)
{
	// Intentionally left empty
}

template <typename StateType, typename RewardModelType, typename ValueType>
uint8_t
ControlThread<StateType, RewardModelType, ValueType>::requestOwnership(CompressedState & state, uint8_t threadIndex, StateType requestedId) {
	// Test to see if a thread already owns this state.
	// TODO: should this pre-lock even be in here?
	if (stateThreadMap.stateToId.contains(state)) {
		return stateThreadMap.stateToId.find(state);
	}
	// Lock the ownership mutex
	std::lock_guard<std::shared_mutex> lock(ownershipMutex);
	if (stateThreadMap.stateToId.contains(state)) {
		return stateThreadMap.stateToId.find(state);
	}
	else {
		// Claim ownership of state
		stateThreadMap.stateToId.findOrAdd(
			state
			threadIndex)
		);
	}
	return threadIndex;
}

template <typename StateType, typename RewardModelType, typename ValueType>
uint8_t
ControlThread<StateType, RewardModelType, ValueType>::whoOwns(CompressedState & state) {
	if (stateThreadMap.stateToId.contains(state)) {
		return stateThreadMap.stateToId.find(state);
	}
	// Index 0 (the same index as the absorbing state) indicates that no thread owns this state.
	return 0;
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
ControlThread<StateType, RewardModelType, ValueType>::requestInsertTransition(
	uint8_t thread
	, StateType from
	, StateType to
	, double rate
) {
	LockableDeque tQueue = transitionQueues[thread - 1];
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
ControlThread<StateType, RewardModelType, ValueType>::mainLoop() {
	uint8_t numberFinishedThreads;
	while (!finished) { // allow for this thread to be killed outside of its main loop
		bool exitThisIteration = false;
		numberFinishedThreads = 0;
		for (auto explorationThread : parent->getExplorationThreads()) {
			if (explorationThread.isIdling()) {
				++numberFinishedThreads;
			}
		}
		if (numberFinishedThreads == parent->getExplorationThreads().size()) {
			// We have finished.
			exitThisIteration = true;
		}
		// Make sure that we flush the queues AFTER we determine whether to exit. This prevents a
		// thread from requesting a transition to be added
		for (auto q : transitionQueues) {
			q.lockThread();
			while (!q.empty()) {
				// Request that the parent class
				parent->createTransition(q.top());
				q.pop();
			}
			q.unlockThread();
		}
		if (exitThisIteration) { return; }
		// TODO: de-fragmentation
	}
}

} // namespace threads
} // namespace builder
} // namespace stamina
