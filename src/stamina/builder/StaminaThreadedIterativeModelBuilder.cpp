#include "StaminaThreadedIterativeModelBuilder.h"

namespace stamina {
namespace builder {

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType>::StaminaThreadedIterativeModelBuilder(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, storm::prism::Program const& modulesFile
	, storm::generator::NextStateGeneratorOptions const & options
) : StaminaIterativeModelBuilder(
		generator
		, modulesFile
		, options
	)

{
	// Intentionally left empty
}

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType>::StaminaThreadedIterativeModelBuilder(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions
) : StaminaThreadedIterativeModelBuilder(
		program
		, generatorOptions
	)
{
	// Intentionally left empty
}

template<typename ValueType, typename RewardModelType, typename StateType>
void
StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType>::buildMatrices(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
	, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
	, StateAndChoiceInformationBuilder& choiceInformationBuilder
	, boost::optional<storm::storage::BitVector>& markovianChoices
	, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder

) {
	// Get initial states
	fresh = false;
	numberTransitions = 0;
	// Builds model
	// Initialize building state valuations (if necessary)
	if (stateAndChoiceInformationBuilder.isBuildStateValuations()) {
		stateAndChoiceInformationBuilder.stateValuationsBuilder() = generator->initializeStateValuationsBuilder();
	}

	this->loadPropertyExpressionFromFormula();

	// Create a callback for the next-state generator to enable it to request the index of states.
	std::function<StateType (CompressedState const&)> stateToIdCallback = std::bind(
		&StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex
		, this
		, std::placeholders::_1
	);

	if (firstIteration) {
		// Create absorbing state
		this->setUpAbsorbingState(
			transitionMatrixBuilder
			, rewardModelBuilders
			, stateAndChoiceInformationBuilder
			, markovianChoices
			, stateValuationsBuilder
		);
		isInit = true;
		// Let the generator create all initial states.
		this->stateStorage.initialStateIndices = generator->getInitialStates(stateToIdCallback);
		if (this->stateStorage.initialStateIndices.empty()) {
			StaminaMessages::errorAndExit("Initial states are empty!");
		}
		currentRowGroup = 1;
		currentRow = 1;
		firstIteration = false;
	}
	else {
		// Flush the previously early-terminated states into statesToExplore FIRST
		flushStatesTerminated();
	}
	numberOfExploredStatesSinceLastMessage = 0;

	auto timeOfStart = std::chrono::high_resolution_clock::now();
	auto timeOfLastMessage = std::chrono::high_resolution_clock::now();

	StateType currentIndex;
	CompressedState currentState;

	isInit = false;
	// Perform a search through the model.
	while (!statesToExplore.empty() && numberTerminal < Options::threads) {
		auto currentProbabilityStatePair = statesToExplore.front();
		currentProbabilityState = statesToExplore.front().first;
		currentState = statesToExplore.front().second;
		statesToExplore.pop_front();
		// Get the first state in the queue.
		currentIndex = currentProbabilityState->index;
		if (currentIndex == 0) {
			StaminaMessages::errorAndExit("Dequeued artificial absorbing state!");
		}

		// std::cout << "Dequeued state " << currentIndex << std::endl;
		// Set our state variable in the class

		if (currentIndex % MSG_FREQUENCY == 0) {
			StaminaMessages::info("Exploring state with id " + std::to_string(currentIndex) + ".");
		}

		if (stateAndChoiceInformationBuilder.isBuildStateValuations()) {
			generator->addStateValuation(currentIndex, stateAndChoiceInformationBuilder.stateValuationsBuilder());
		}

		// Load state for us to use
		generator->load(currentState);

		if (propertyExpression != nullptr) {
			storm::expressions::SimpleValuation valuation = generator->currentStateToSimpleValuation();
			bool evaluationAtCurrentState = propertyExpression->evaluateAsBool(&valuation);
			// If the property does not hold at the current state, make it absorbing in the
			// state graph and do not explore its successors
			if (!evaluationAtCurrentState) {
				transitionMatrixBuilder.addNextValue(currentRow, currentIndex, 1.0);
				// We treat this state as terminal even though it is also absorbing and does not
				// go to our artificial absorbing state
				currentProbabilityState->terminal = true;
				numberTerminal++;
				// Do NOT place this in the deque of states we should start with next iteration
				continue;
			}
		}

		// Add the state rewards to the corresponding reward models.
		// Do not explore if state is terminal and its reachability probability is less than kappa
		if (currentProbabilityState->isTerminal() && currentProbabilityState->getPi() < localKappa) {
			// Do not connect to absorbing yet
			// Place this in statesTerminatedLastIteration
			if ( !currentProbabilityState->wasPutInTerminalQueue ) {
				statesTerminatedLastIteration.emplace_back(currentProbabilityStatePair);
				currentProbabilityState->wasPutInTerminalQueue = true;
				++currentRow;
				++currentRowGroup;
			}
			continue;
		}

		// We assume that if we make it here, our state is either nonterminal, or its reachability probability
		// is greater than kappa
		// Expand (explore next states)
		storm::generator::StateBehavior<ValueType, StateType> behavior = generator->expand(stateToIdCallback);

		auto stateRewardIt = behavior.getStateRewards().begin();
		for (auto& rewardModelBuilder : rewardModelBuilders) {
			if (rewardModelBuilder.hasStateRewards()) {
				rewardModelBuilder.addStateReward(*stateRewardIt);
			}
			++stateRewardIt;
		}
		// If there is no behavior, we have an error.
		if (behavior.empty()) {
			// Make absorbing
			transitionMatrixBuilder.addNextValue(currentRow, currentIndex, 1.0);
			continue;
			// StaminaMessages::warn("Behavior for state " + std::to_string(currentIndex) + " was empty!");
		}

		bool shouldEnqueueAll = currentProbabilityState->getPi() == 0.0;
		// Now add all choices.
		bool firstChoiceOfState = true;
		for (auto const& choice : behavior) {
			if (!firstChoiceOfState) {
				StaminaMessages::errorAndExit("Model was not deterministic!");
			}
			// add the generated choice information
			if (stateAndChoiceInformationBuilder.isBuildChoiceLabels() && choice.hasLabels()) {
				for (auto const& label : choice.getLabels()) {
					stateAndChoiceInformationBuilder.addChoiceLabel(label, currentRow);

				}
			}
			if (stateAndChoiceInformationBuilder.isBuildChoiceOrigins() && choice.hasOriginData()) {
				stateAndChoiceInformationBuilder.addChoiceOriginData(choice.getOriginData(), currentRow);
			}
			if (stateAndChoiceInformationBuilder.isBuildStatePlayerIndications() && choice.hasPlayerIndex()) {
				if (firstChoiceOfState) {
					stateAndChoiceInformationBuilder.addStatePlayerIndication(choice.getPlayerIndex(), currentRowGroup);
				}
			}

			double totalRate = 0.0;
			if (!shouldEnqueueAll && isCtmc) {
				for (auto const & stateProbabilityPair : choice) {
					if (stateProbabilityPair.first == 0) {
						StaminaMessages::warning("Transition to absorbing state from API!!!");
						continue;
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
				double probability = isCtmc ? stateProbabilityPair.second / totalRate : stateProbabilityPair.second;
				// Enqueue S is handled in stateToIdCallback
				// Update transition probability only if we should enqueue all
				// These are next states where the previous state has a reachability
				// greater than zero

				auto nextProbabilityState = stateMap.get(sPrime);
				if (nextProbabilityState != nullptr) {
					if (!shouldEnqueueAll) {
						nextProbabilityState->addToPi(currentProbabilityState->getPi() * probability);
					}

					if (currentProbabilityState->isNew) {
						this->createTransition(currentIndex, sPrime, stateProbabilityPair.second);
						numberTransitions++;
					}
				}
			}

			++currentRow;
			firstChoiceOfState = false;
		}

		currentProbabilityState->isNew = false;

		if (currentProbabilityState->isTerminal() && numberTerminal > 0) {
			numberTerminal--;
		}
		currentProbabilityState->setTerminal(false);
		currentProbabilityState->setPi(0.0);

		++currentRowGroup;

		if (generator->getOptions().isShowProgressSet()) {
			++numberOfExploredStatesSinceLastMessage;

			auto now = std::chrono::high_resolution_clock::now();
			auto durationSinceLastMessage = std::chrono::duration_cast<std::chrono::seconds>(now - timeOfLastMessage).count();
			if (static_cast<uint64_t>(durationSinceLastMessage) >= generator->getOptions().getShowProgressDelay()) {
				auto statesPerSecond = numberOfExploredStatesSinceLastMessage / durationSinceLastMessage;
				auto durationSinceStart = std::chrono::duration_cast<std::chrono::seconds>(now - timeOfStart).count();
				StaminaMessages::info(
					"Explored " + std::to_string(numberOfExploredStatesSinceLastMessage) + " states in " + std::to_string(durationSinceStart) + " seconds (currently " + std::to_string(statesPerSecond) + " states per second)."
				);
				timeOfLastMessage = std::chrono::high_resolution_clock::now();
				numberOfExploredStatesSinceLastMessage = 0;
			}
		}

	}
	iteration++;
	numberStates = stateStorage.stateToId.size(); // numberOfExploredStates;
	
	// Create threads and explore initial states
	// If numberTerminal is less than the number of threads, we are under-utilizing threads
	if (numberTerminal < Options::threads) {
		StaminaMessages::warning("The number of threads that will be utilized (" + std::to_string(numberTerminal) + ") is less than the number created! ( " + std::to_string(Options::threads) + ")");
	}
	
	/*
	 * The hold (auto set to true in thread constructor) make it so these threads don't just stop after starting.
	 * Therefore after everything is set up, we must turn off the hold. However:
	 *     + The threads must be STARTED in order to request cross exploration. TODO: do they?
	 *     + Threads won't die when hold is true.
	 */

	// Start control thread
	this->controlThread.start();

	// Start all of the worker threads
	for (auto explorationThread : this->explorationThreads) {
		explorationThread.start();
	}

	auto terminalStatesVector = this->getTerminalStates();
	uint8_t threadIndex = 1;
	for (auto terminalState : terminalStatesVector) {
		auto explorationThread = this->explorationThreads[i];
		// TODO: ask for cross exploration from thread at that index
		if (threadIndex == Options::threads) {
			threadIndex = 1;
		}
		else {
			threadIndex++;
		}
	}

	// Remove the hold on all of the worker threads
	for (auto explorationThread : this->explorationThreads) {
		explorationThread.setHold(false);
	}

	this->controlThread.setHold(false);

}

template class StaminaThreadedIterativeModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace builder
} // namespace stamina
