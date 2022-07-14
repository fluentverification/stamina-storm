#include "IterativeExplorationThread.h"

#include "core/StaminaMessages.h"

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
	auto currentProbabilityState = stateMap.get(stateProbability.index);

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
		bool evaluationAtCurrentState = propertyExpression->evaluateAsBool(&valuation);
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
	if (currentProbabilityState->isTerminal() && currentProbabilityState->getPi() < localKappa) {
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
// 		if (stateAndChoiceInformationBuilder.isBuildChoiceLabels() && choice.hasLabels()) {
// 			for (auto const& label : choice.getLabels()) {
// 				stateAndChoiceInformationBuilder.addChoiceLabel(label, currentRow);
//
// 			}
// 		}
// 		if (stateAndChoiceInformationBuilder.isBuildChoiceOrigins() && choice.hasOriginData()) {
// 			stateAndChoiceInformationBuilder.addChoiceOriginData(choice.getOriginData(), currentRow);
// 		}
// 		if (stateAndChoiceInformationBuilder.isBuildStatePlayerIndications() && choice.hasPlayerIndex()) {
// 			if (firstChoiceOfState) {
// 				stateAndChoiceInformationBuilder.addStatePlayerIndication(choice.getPlayerIndex(), currentRowGroup);
// 			}
// 		}

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
			if (sPrime == 0) {
				continue;
			}
			// Check if we own sPrime and if we don't ask the thread who does to explore it
			uint8_t sPrimeOwner = this->controlThread.whoOwns(sPrime);
			if (sPrimeOwner != this->threadIndex && sPrimeOwner != NO_THREAD) {
				// Request cross exploration
				this->controlThread.requestCrossExplorationFromThread(sPrimeOwner, StateAndProbability());
				continue;
			}
			else if (sPrimeOwner == NO_THREAD) {
				// Request ownership

				bool failedRequest;
				if (failedRequest) {
					// request cross exploration

					continue;
				}
			}

			// At this point we assume that we own sPrime

			double probability = isCtmc ? stateProbabilityPair.second / totalRate : stateProbabilityPair.second;
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
					this->controlThread.requestInsertTransition(threadIndex, currentIndex, sPrime, stateProbabilityPair.second);
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
