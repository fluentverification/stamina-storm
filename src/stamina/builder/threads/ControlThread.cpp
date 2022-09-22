#include "ControlThread.h"
#include "builder/__storm_needed_for_builder.h"

#include "core/StaminaMessages.h"

#include "builder/threads/ExplorationThread.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename ValueType, typename RewardModelType, typename StateType>
ControlThread<ValueType, RewardModelType, StateType>::ControlThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t numberExplorationThreads
) : BaseThread<ValueType, RewardModelType, StateType>(parent)
	, numberExplorationThreads(numberExplorationThreads)
	, stateThreadMap(*(new storm::storage::BitVectorHashMap<uint8_t, storm::storage::Murmur3BitVectorHash<StateType>>(parent->getGenerator()->getStateSize())))
{
	// Create transition queues
	for (int i = 0; i < Options::threads; i++) {
		transitionQueues.push_back(LockableDeque());
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
std::pair<uint8_t, StateType>
ControlThread<ValueType, RewardModelType, StateType>::requestOwnership(CompressedState & state, uint8_t threadIndex, StateType requestedId) {
	// Test to see if a thread already owns this state.
	// TODO: should this pre-lock even be in here?
	if (stateThreadMap.contains(state)) {
		return std::make_pair(
			stateThreadMap.getValue(state)
			, this->parent->getStateStorage().stateToId.getValue(state)
		);
	}
	// Lock the ownership mutex
	std::lock_guard<std::shared_mutex> lock(ownershipMutex);
	if (stateThreadMap.contains(state)) {
		return std::make_pair(
			stateThreadMap.getValue(state)
			, this->parent->getStateStorage().stateToId.getValue(state)
		);
	}
	else {
		// Claim ownership of state
		stateThreadMap.findOrAdd(
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
ControlThread<ValueType, RewardModelType, StateType>::whoOwns(CompressedState & state) const {
	if (stateThreadMap.contains(state)) {
		return stateThreadMap.getValue(state);
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
	LockableDeque & tQueue = transitionQueues[thread - 1];
	tQueue.lockThread();
	tQueue.emplace_back(from, to, rate);
	tQueue.unlockThread();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ControlThread<ValueType, RewardModelType, StateType>::requestCrossExplorationFromThread(
	StateProbability stateAndProbability
	, double threadIndex
) {
	// TODO: implement
	// Pointer black magic because you can't have a reference or variable of an abstract class
	auto explorationThread = &(this->explorationThreads[threadIndex - 1]);
	explorationThread->requestCrossExploration(
		stateAndProbability.state
		, stateAndProbability.deltaPi
	);
	// explorationThread(stateAndProbability.state, stateAndProbability.probability);
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ControlThread<ValueType, RewardModelType, StateType>::mainLoop() {
	STAMINA_DEBUG_MESSAGE("Starting control thread.");
	uint8_t numberFinishedThreads;
	auto explorationThreads = this->parent->getExplorationThreads();
	while (!this->finished || this->hold) { // allow for this thread to be killed outside of its main loop
		bool exitThisIteration = false;
		numberFinishedThreads = 0;

		for (auto explorationThread : explorationThreads) {
			if (!explorationThread) {
				StaminaMessages::warning("Somehow this exploration thread is null!");
			}
			else if (explorationThread->isIdling()) {
				++numberFinishedThreads;
			}
		}
		if (numberFinishedThreads == this->parent->getExplorationThreads().size() && !this->hold) {
			// We have finished.
			exitThisIteration = true;
		}
		registerTransitions();
		if (exitThisIteration) {
			STAMINA_DEBUG_MESSAGE("Exiting control thread main loop because all threads are finished");
			STAMINA_DEBUG_MESSAGE("Sanity check: there are the following number of exploration threads: " << this->parent->getExplorationThreads().size());
			for (auto explorationThread : this->parent->getExplorationThreads()) {
				STAMINA_DEBUG_MESSAGE("Killing exploration thread");
				explorationThread->terminate();
				explorationThread->join();
			}
			this->finished = true;
			this->hold = false;
			// this->terminate();
			STAMINA_DEBUG_MESSAGE("Ending this iteration now!");
			registerTransitions();
			return;
		}
		// TODO: de-fragmentation
		// TODO: LRU Cache
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ControlThread<ValueType, RewardModelType, StateType>::registerTransitions() {
		// Make sure that we flush the queues AFTER we determine whether to exit. This prevents a
		// thread from requesting a transition to be added
		STAMINA_DEBUG_MESSAGE("Size of transition queues: " << transitionQueues.size());
		for (auto q : transitionQueues) {
			q.lockThread();
			STAMINA_DEBUG_MESSAGE("Queue size: " << q.size());
			while (!q.empty()) {
				// Request that the parent class
				this->parent->createTransition(q.top());
				q.pop();
				STAMINA_DEBUG_MESSAGE("Creating a transition for a state");
			}
			q.unlockThread();
		}

}
// Lockable Deque methods

template <typename ValueType, typename RewardModelType, typename StateType>
int
ControlThread<ValueType, RewardModelType, StateType>::LockableDeque::size() const {
	return queue.size();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ControlThread<ValueType, RewardModelType, StateType>::LockableDeque::emplace_back(
	StateType from
	, StateType to
	, double rate
) {
	Transition t(from, to, rate);
	queue.emplace_back(t);
}

template <typename ValueType, typename RewardModelType, typename StateType>
bool
ControlThread<ValueType, RewardModelType, StateType>::LockableDeque::empty() const {
	return queue.empty();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ControlThread<ValueType, RewardModelType, StateType>::LockableDeque::lockThread() {
	lock.lock();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void ControlThread<ValueType, RewardModelType, StateType>::LockableDeque::unlockThread() {
	lock.unlock();
}

template <typename ValueType, typename RewardModelType, typename StateType>
typename ControlThread<ValueType, RewardModelType, StateType>::Transition
ControlThread<ValueType, RewardModelType, StateType>::LockableDeque::top() const {
	return queue.front();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ControlThread<ValueType, RewardModelType, StateType>::LockableDeque::pop() {
	queue.pop_front();
}

template class ControlThread<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace threads
} // namespace builder
} // namespace stamina
