#include "IterativeExplorationThread.h"

#include "core/StaminaMessages.h"
#include "util/StateSpaceInformation.h"

namespace stamina {
namespace builder {
namespace threads {

template <typename StateType, typename RewardModelType, typename ValueType>
IterativeExplorationThread<StateType, RewardModelType, ValueType>::IterativeExplorationThread(
	StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
	, uint8_t threadIndex
	, ControlThread<StateType, RewardModelType, ValueType> & controlThread
	, uint32_t stateSize
	, util::StateIndexArray<StateType, ProbabilityState<StateType>> * stateMap
	, std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, std::function<StateType (CompressedState const&)> stateToIdCallback
) : ExplorationThread<StateType, RewardModelType, ValueType>(
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

template <typename StateType, typename RewardModelType, typename ValueType>
void
IterativeExplorationThread<StateType, RewardModelType, ValueType>::enqueueSuccessors(CompressedState & state) {
	StateType actualIndex;
	// Request ownership of state
	// Check if we own sPrime and if we don't ask the thread who does to explore it
	uint8_t sPrimeOwner = this->controlThread.whoOwns(state);
	if (sPrimeOwner != this->threadIndex && sPrimeOwner != NO_THREAD) {
		actualIndex = controlThread.whatIsIndex(state);
		// Request cross exploration handled in other function
		this->statesToRequestCrossExploration.emplace_back({state, actualIndex, sPrimeOwner});
		return actualIndex; // TODO: another thread owns
	}
	else if (sPrimeOwner == NO_THREAD) {
		// Request ownership
		auto threadAndStateIndecies = controlThread.requestOwnership(this->threadIndex, state);
		bool failedRequest = threadAndStateIndecies.first != this->threadIndex;
		actualIndex = threadAndStateIndecies.second;
		if (failedRequest) {
			// request cross exploration
			this->statesToRequestCrossExploration.emplace_back({state, actualIndex, sPrimeOwner});
			return actualIndex; // TODO: another thread owns
		}
	}
	else {
		actualIndex = controlThread.whatIsIndex(state);
	}

	auto nextState = stateMap.get(actualIndex);
	bool stateIsExisting = nextState != nullptr;

	stateStorage.stateToId.findOrAdd(state, actualIndex);
	// Handle conditional enqueuing
	if (isInit) {
		if (!stateIsExisting) {
			// Create a ProbabilityState for each individual state
			ProbabilityState<StateType> * initProbabilityState = memoryPool.allocate();
			*initProbabilityState = ProbabilityState<StateType>(
				actualIndex
				, 1.0
				, true
			);
			numberTerminal++;
			stateMap.put(actualIndex, initProbabilityState);
			statesToExplore.push_back(std::make_pair(initProbabilityState, state));
			initProbabilityState->iterationLastSeen = iteration;
		}
		else {
			ProbabilityState<StateType> * initProbabilityState = nextState;
			stateMap.put(actualIndex, initProbabilityState);
			statesToExplore.push_back(std::make_pair(initProbabilityState, state));
			initProbabilityState->iterationLastSeen = iteration;
		}
		return actualIndex;
	}

	bool enqueued = false;

	// This bit handles the non-initial states
	// The previous state has reachability of 0
	if (currentProbabilityState->getPi() == 0) {
		if (stateIsExisting) {
			// Don't rehash if we've already called find()
			ProbabilityState<StateType> * nextProbabilityState = nextState;
			if (nextProbabilityState->iterationLastSeen != iteration) {
				nextProbabilityState->iterationLastSeen = iteration;
				// Enqueue
				statesToExplore.push_back(std::make_pair(nextProbabilityState, state));
				enqueued = true;
			}
		}
		else {
			// State does not exist yet in this iteration
			return 0;
		}
	}
	else {
		if (stateIsExisting) {
			// Don't rehash if we've already called find()
			ProbabilityState<StateType> * nextProbabilityState = nextState;
			// auto emplaced = exploredStates.emplace(actualIndex);
			if (nextProbabilityState->iterationLastSeen != iteration) {
				nextProbabilityState->iterationLastSeen = iteration;
				// Enqueue
				statesToExplore.push_back(std::make_pair(nextProbabilityState, state));
				enqueued = true;
			}
		}
		else {
			// This state has not been seen so create a new ProbabilityState
			ProbabilityState<StateType> * nextProbabilityState = memoryPool.allocate();
			*nextProbabilityState = ProbabilityState<StateType>(
				actualIndex
				, 0.0
				, true
			);
			stateMap.put(actualIndex, nextProbabilityState);
			nextProbabilityState->iterationLastSeen = iteration;
			// exploredStates.emplace(actualIndex);
			statesToExplore.push_back(std::make_pair(nextProbabilityState, state));
			enqueued = true;
			numberTerminal++;
		}
	}
	return actualIndex;
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
IterativeExplorationThread<StateType, RewardModelType, ValueType>::exploreStates() {
	if (!this->crossExplorationQueue.empty() && !this->crossExplorationQueueMutex.locked()) {
		std::lock_guard<std::shared_mutex> lockGuard(this->crossExplorationQueueMutex);
		auto stateDeltaPiPair = this->crossExplorationQueue.top();
		this->crossExplorationQueue.pop();
		StateType s = stateDeltaPiPair.first;
		double deltaPi = stateDeltaPiPair.second;
		// Update the estimated reachability of s
		auto probabilityState = this->parent->getStateMap().get(s);
		probabilityState.pi += deltaPi;
		exploreState(s);
	}
	else if (!this->mainExplorationQueue.empty()) {
		auto s = this->mainExplorationQueue.top();
		this->mainExplorationQueue.pop();
		exploreState(s);
	}
	else {
		this->idling = true;
	}
}

template <typename StateType, typename RewardModelType, typename ValueType>
void
IterativeExplorationThread<StateType, RewardModelType, ValueType>::exploreState(StateAndProbability & stateProbability) {
	auto currentProbabilityState = this->stateMap.get(stateProbability.index);

	StateType currentIndex = stateProbability.index;
	CompressedState & currentState = stateProbability.state;

	// Flush deltaPi
	currentProbabilityState.pi += stateProbability.deltaPi;

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
				StaminaMessages::warning("stamina::builder::threads::IterativeExplorationThread does not support choice labels! The following label will not be added: " + std::to_string(label));

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
				StateIndexAndThread stateIndexAndThread = this->statesToRequestCrossExploration.front();
				this->statesToRequestCrossExploration.pop_front();
				controlThread.requestCrossExplorationFromThread(
					StateAndProbability(
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

			auto nextProbabilityState = this->stateMap.get(sPrime);
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


} // namespace threads
} // namespace builder
} // namespace stamina
