#include "ExplorationThread.h"
#include "builder/StaminaModelBuilder.h"

#include "core/StaminaMessages.h"

#include <mutex>

namespace stamina {
namespace builder {
namespace threads {

template <typename ValueType, typename RewardModelType, typename StateType>
ExplorationThread<ValueType, RewardModelType, StateType>::ExplorationThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t threadIndex
	, ControlThread<ValueType, RewardModelType, StateType> & controlThread
	, uint32_t stateSize
	, util::StateIndexArray<StateType, ProbabilityState<StateType>> * stateMap
	, std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, std::function<StateType (CompressedState const&)> stateToIdCallback
) : BaseThread<ValueType, RewardModelType, StateType>(parent)
	, threadIndex(threadIndex)
	, controlThread(controlThread)
	, stateStorage(parent->getStateStorage())
	, stateMap(stateMap)
	, generator(generator)
	, stateToIdCallback(stateToIdCallback)
	, xLock(crossExplorationQueueMutex, std::defer_lock)
{
	// Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
uint8_t
ExplorationThread<ValueType, RewardModelType, StateType>::getIndex() {
	return threadIndex;
}

template <typename ValueType, typename RewardModelType, typename StateType>
uint32_t
ExplorationThread<ValueType, RewardModelType, StateType>::getNumberOfOwnedStates() {
	return numberOfOwnedStates;
}

template <typename ValueType, typename RewardModelType, typename StateType>
bool
ExplorationThread<ValueType, RewardModelType, StateType>::isIdling() {
	return idling;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ExplorationThread<ValueType, RewardModelType, StateType>::setIsCtmc(bool isCtmc) {
	this->isCtmc = isCtmc;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ExplorationThread<ValueType, RewardModelType, StateType>::requestCrossExploration(
	CompressedState & state
	, double deltaPi
) {
	// Lock the mutex since multiple threads will be calling this function
	// auto lock = std::unique_lock<std::shared_mutex>(crossExplorationQueueMutex);
	std::lock_guard<std::shared_mutex> guard(crossExplorationQueueMutex);
	// crossExplorationQueueMutex.lock();
	crossExplorationQueue.emplace_back(
		std::make_pair(state, deltaPi)
	);
	// crossExplorationQueueMutex.unlock();
	// lock.unlock();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
ExplorationThread<ValueType, RewardModelType, StateType>::mainLoop() {
	STAMINA_DEBUG_MESSAGE("Starting exploration thread: " << this->threadIndex);
	idling = false;
	while (!this->finished || this->hold) {
		// STAMINA_DEBUG_MESSAGE("Finished is " << this->finished << " and hold is " << this->hold);
		// Explore the states in the exploration queue
		exploreStates();
	}
}

template class ExplorationThread<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace threads
} // namespace builder
} // namespace stamina
#include "IterativeExplorationThread.h"

#include "builder/threads/ControlThread.h"
#include "core/StaminaMessages.h"
#include "core/StateSpaceInformation.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename ValueType, typename RewardModelType, typename StateType>
IterativeExplorationThread<ValueType, RewardModelType, StateType>::IterativeExplorationThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t threadIndex
	, ControlThread<ValueType, RewardModelType, StateType> & controlThread
	, uint32_t stateSize
	, util::StateIndexArray<StateType, ProbabilityState<StateType>> * stateMap
	, std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, std::function<StateType (CompressedState const&)> stateToIdCallback
) : ExplorationThread<ValueType, RewardModelType, StateType>(
	parent
	, threadIndex
	, controlThread
	, stateSize
	, stateMap
	, generator
	, stateToIdCallback
)
{
	// Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
IterativeExplorationThread<ValueType, RewardModelType, StateType>::enqueueSuccessors(CompressedState & state) {
	StateType actualIndex;
	// Request ownership of state
	// Check if we own sPrime and if we don't ask the thread who does to explore it
	uint8_t sPrimeOwner = this->controlThread.whoOwns(state);
	if (sPrimeOwner != this->threadIndex && sPrimeOwner != NO_THREAD) {
		actualIndex = this->controlThread.whatIsIndex(state);
		StateIndexAndThread sThreadIndex(state, actualIndex, sPrimeOwner);
		// Request cross exploration handled in other function
		this->statesToRequestCrossExploration.emplace_back(sThreadIndex);
		return; // TODO: another thread owns
	}
	else if (sPrimeOwner == NO_THREAD) {
		// Request ownership
		auto threadAndStateIndecies = this->controlThread.requestOwnership(state, this->threadIndex);
		bool failedRequest = threadAndStateIndecies.first != this->threadIndex;
		actualIndex = threadAndStateIndecies.second;
		if (failedRequest) {
			StateIndexAndThread sThreadIndex(state, actualIndex, sPrimeOwner);
			// request cross exploration
			this->statesToRequestCrossExploration.emplace_back(sThreadIndex);
			return; // TODO: another thread owns
		}
	}
	else {
		actualIndex = this->controlThread.whatIsIndex(state);
	}

	auto nextState = this->parent->getStateMap().get(actualIndex);
	bool stateIsExisting = nextState != nullptr;

	this->parent->getStateStorage().stateToId.findOrAdd(state, actualIndex);
	// Handle conditional enqueuing

	bool enqueued = false;

	// This bit handles the non-initial states
	// The previous state has reachability of 0
	if (currentStateHasZeroReachability) {
		if (stateIsExisting) {
			// Don't rehash if we've already called find()
			ProbabilityState<StateType> * nextProbabilityState = nextState;
			if (nextProbabilityState->iterationLastSeen != this->parent->getIteration()) {
				nextProbabilityState->iterationLastSeen = this->parent->getIteration();
				// Enqueue
				/* this->mainExplorationQueue.emplace_back(
					std::make_pair(nextProbabilityState, state)
				); */
				enqueued = true;
			}
		}
		else {
			// State does not exist yet in this iteration
			return;
		}
	}
	else {
		if (stateIsExisting) {
			// Don't rehash if we've already called find()
			ProbabilityState<StateType> * nextProbabilityState = nextState;
			// auto emplaced = exploredStates.emplace(actualIndex);
			if (nextProbabilityState->iterationLastSeen != this->parent->getIteration()) {
				nextProbabilityState->iterationLastSeen = this->parent->getIteration();
				// Enqueue
				/* this->mainExplorationQueue.emplace_back(std::make_pair(nextProbabilityState, state)); */
				enqueued = true;
			}
		}
		else {
			// This state has not been seen so create a new ProbabilityState
			ProbabilityState<StateType> * nextProbabilityState = this->parent->getMemoryPool().allocate();
			*nextProbabilityState = ProbabilityState<StateType>(
				actualIndex
				, 0.0
				, true
			);
			this->parent->getStateMap().put(actualIndex, nextProbabilityState);
			nextProbabilityState->iterationLastSeen = this->parent->getIteration();
			// exploredStates.emplace(actualIndex);
			/* this->mainExplorationQueue.emplace_back(std::make_pair(nextProbabilityState, state)); */
			enqueued = true;
			numberTerminal++;
		}
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
IterativeExplorationThread<ValueType, RewardModelType, StateType>::exploreStates() {
	if (!this->crossExplorationQueue.empty() && this->xLock.try_lock()) {
		STAMINA_DEBUG_MESSAGE("Exploring from the cross exploration queue");
		// std::lock_guard<decltype(this->xLock)> lockGuard(this->xLock);
		auto stateDeltaPiPair = this->crossExplorationQueue.front();
		this->crossExplorationQueue.pop_front();
		auto s = stateDeltaPiPair.first;
		double deltaPi = stateDeltaPiPair.second;
		StateType stateIndex = this->parent->getStateIndexOrAbsorbing(s);
		StateProbability stateProbability(
			s            // State values
			, stateIndex // State Index
			, deltaPi    // Delta Pi
		);

		// Update the estimated reachability of s
		// auto probabilityState = this->parent->getStateMap().get(stateIndex);
		// probabilityState->pi += deltaPi;
		exploreState(stateProbability);
		this->xLock.unlock();
	}
	else if (!this->mainExplorationQueue.empty()) {
		STAMINA_DEBUG_MESSAGE("Exploring from main exploration queue");
		// If we are dequeuing from the main exploration queue, then
		// the state we are enqueuing doesn't have a delta pi
		auto s = this->mainExplorationQueue.front();
		this->mainExplorationQueue.pop_front();
		StateProbability stateProbability(
			s.second
			, s.first->index
		);
		exploreState(stateProbability);
	}
	else if (!this->xLock.owns_lock()) {
		STAMINA_DEBUG_MESSAGE("Size of cross exploration queue: " << this->crossExplorationQueue.size());
		STAMINA_DEBUG_MESSAGE("Thread " << this->threadIndex << " is idling...");
		if (!this->crossExplorationQueue.empty() && !this->xLock.owns_lock()) {
			STAMINA_DEBUG_MESSAGE("Cross-exploration queue is not emply, but lock has not been achieved.");
		}
		else if (this->mainExplorationQueue.empty()) {
			// STAMINA_DEBUG_MESSAGE("Both main and cross exploration queue are empty");
		}
		this->idling = true;
	}
	else {
		this->idling = false;
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
IterativeExplorationThread<ValueType, RewardModelType, StateType>::exploreState(StateProbability & stateProbability) {
	auto currentProbabilityState = this->parent->getStateMap().get(stateProbability.index);

	StateType currentIndex = stateProbability.index;
	CompressedState & currentState = stateProbability.state;

	// Flush deltaPi
	currentProbabilityState->pi += stateProbability.deltaPi;

	// Load this state to use
	this->generator->load(currentState);

	/*
	 * Early termination based on property expression
	 * */
	if (this->parent->getPropertyExpression() != nullptr) {
		storm::expressions::SimpleValuation valuation = this->generator->currentStateToSimpleValuation();
		bool evaluationAtCurrentState = this->parent->getPropertyExpression()->evaluateAsBool(&valuation);
		// If the property does not hold at the current state, make it absorbing in the
		// state graph and do not explore its successors
		if (!evaluationAtCurrentState) {
			this->controlThread.requestInsertTransition(
				this->threadIndex
				, currentIndex
				, 0
				, 1.0
			);
			// We treat this state as terminal even though it is also absorbing and does not
			// go to our artificial absorbing state
			currentProbabilityState->terminal = true;
			numberTerminal++;
			// Do NOT place this in the deque of states we should start with next iteration
			return;
		}
	}

	// Do not explore if state is terminal and its reachability probability is less than kappa
	if (currentProbabilityState->isTerminal() && currentProbabilityState->getPi() < this->parent->getLocalKappa()) {
		// Do not connect to absorbing yet
		// Place this in statesTerminatedLastIteration
		if ( !currentProbabilityState->wasPutInTerminalQueue ) {
			this->statesTerminatedLastIteration.emplace_back(stateProbability);
			currentProbabilityState->wasPutInTerminalQueue = true;
		}
		return;
	}

	currentStateHasZeroReachability = currentProbabilityState->getPi() == 0;

	// We assume that if we make it here, our state is either nonterminal, or its reachability probability
	// is greater than kappa
	// Expand this state
	storm::generator::StateBehavior<ValueType, StateType> behavior = this->generator->expand(this->stateToIdCallback);

	if (behavior.empty()) {
		// This state needs to be made absorbing
		this->controlThread.requestInsertTransition(
			this->threadIndex
			, currentIndex
			, currentIndex
			, 1.0
		);
	}

	bool shouldEnqueueAll = currentProbabilityState->getPi() == 0.0;
	// Now add all choices.
	bool firstChoiceOfState = true;
	for (auto const& choice : behavior) {
		if (!firstChoiceOfState) {
			StaminaMessages::errorAndExit("Model was not deterministic!");
		}
		// add the generated choice information
		if (choice.hasLabels()) { // stateAndChoiceInformationBuilder.isBuildChoiceLabels() &&
			for (auto const& label : choice.getLabels()) {
				StaminaMessages::warning("stamina::builder::threads::IterativeExplorationThread does not support choice labels!");

			}
		}
		if (choice.hasOriginData()) { // stateAndChoiceInformationBuilder.isBuildChoiceOrigins() &&
			StaminaMessages::warning("stamina::builder::threads::IterativeExplorationThread does not support origin data!");
		}
		if (choice.hasPlayerIndex()) { // stateAndChoiceInformationBuilder.isBuildStatePlayerIndications() &&
			if (firstChoiceOfState) {
				StaminaMessages::warning("stamina::builder::threads::IterativeExplorationThread does not player indecies!");
			}
		}

		double totalRate = 0.0;
		if (!shouldEnqueueAll && isCtmc) {
			for (auto const & stateProbabilityPair : choice) {
				if (stateProbabilityPair.first == 0) {
					StaminaMessages::warning("Transition to absorbing state from API!!!");
					return;
				}
				totalRate += stateProbabilityPair.second;
			}
		}
		// Add the probabilistic behavior to the matrix.
		for (auto const& stateProbabilityPair : choice) {
			StateType sPrime = stateProbabilityPair.first;
			double probability = isCtmc ? stateProbabilityPair.second / totalRate : stateProbabilityPair.second;

			if (sPrime == 0) {
				continue;
			}
			else if (sPrime == this->statesToRequestCrossExploration.front().index) {
				// Request cross exploration
				auto stateIndexAndThread = this->statesToRequestCrossExploration.front();
				this->statesToRequestCrossExploration.pop_front();
				this->controlThread.requestCrossExplorationFromThread(
					StateProbability(
						stateIndexAndThread.state // Compressed State
						, currentProbabilityState->getPi() * probability // deltaPi
						, stateIndexAndThread.index
					)
					, stateIndexAndThread.threadIndex
				);

			}

			// At this point we assume that we own sPrime

			// Enqueue S is handled in stateToIdCallback
			// Update transition probability only if we should enqueue all
			// These are next states where the previous state has a reachability
			// greater than zero

			auto nextProbabilityState = this->stateMap->get(sPrime);
			if (nextProbabilityState != nullptr) {
				if (!shouldEnqueueAll) {
					nextProbabilityState->addToPi(currentProbabilityState->getPi() * probability);
				}

				if (currentProbabilityState->isNew) {
					this->controlThread.requestInsertTransition(this->threadIndex, currentIndex, sPrime, stateProbabilityPair.second);
				}
			}
		}

		firstChoiceOfState = false;
	}

	currentProbabilityState->isNew = false;

	if (currentProbabilityState->isTerminal() && numberTerminal > 0) {
		numberTerminal--;
	}
	currentProbabilityState->setTerminal(false);
	currentProbabilityState->setPi(0.0);

}

template class IterativeExplorationThread<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace threads
} // namespace builder
} // namespace stamina
