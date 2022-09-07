#include "StaminaThreadedIterativeModelBuilder.h"

namespace stamina {
namespace builder {

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType>::StaminaThreadedIterativeModelBuilder(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, storm::prism::Program const& modulesFile
	, storm::generator::NextStateGeneratorOptions const & options
) : StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>(
		generator
		, modulesFile
		, options
	)
	, controlThread(this, Options::threads)
	, controlThreadsCreated(false)

{
	// Intentionally left empty
}

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType>::StaminaThreadedIterativeModelBuilder(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions
) : StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>(
		program
		, generatorOptions
	)
	, controlThread(this, Options::threads)
	, controlThreadsCreated(false)
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
	this->fresh = false;
	this->numberTransitions = 0;
	// Builds model
	// Initialize building state valuations (if necessary)
	if (choiceInformationBuilder.isBuildStateValuations()) {
		choiceInformationBuilder.stateValuationsBuilder() = this->generator->initializeStateValuationsBuilder();
	}

	this->loadPropertyExpressionFromFormula();

	// Create a callback for the next-state generator to enable it to request the index of states.
	std::function<StateType (CompressedState const&)> stateToIdCallback = std::bind(
		&StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex
		, this
		, std::placeholders::_1
	);

	// Create exploration threads
	if (!controlThreadsCreated) {
		uint8_t threadIndex = 1;
		while (explorationThreads.size() < Options::threads) {
			explorationThreads.push_back(
				std::ref(threads::IterativeExplorationThread<ValueType, RewardModelType, StateType>(
					this // parent
					, threadIndex // Thread index
					, this->controlThread // Control Thread
					, this->getGenerator()->getVariableInformation().getTotalBitOffset(true)// state size
					, & this->getStateMap()
					, this->getGenerator()
					, stateToIdCallback
				))
			);
			threadIndex++;
		}
		controlThreadsCreated = true;
	}

	if (this->firstIteration) {
		// Create absorbing state
		this->setUpAbsorbingState(
			transitionMatrixBuilder
			, rewardModelBuilders
			, choiceInformationBuilder
			, markovianChoices
			, stateValuationsBuilder
		);
		this->isInit = true;
		// Let the generator create all initial states.
		this->stateStorage.initialStateIndices = this->generator->getInitialStates(stateToIdCallback);
		if (this->stateStorage.initialStateIndices.empty()) {
			StaminaMessages::errorAndExit("Initial states are empty!");
		}
		this->currentRowGroup = 1;
		this->currentRow = 1;
		this->firstIteration = false;
	}
	else {
		// Flush the previously early-terminated states into statesToExplore FIRST
		this->flushStatesTerminated();
	}
	this->numberOfExploredStatesSinceLastMessage = 0;

	auto timeOfStart = std::chrono::high_resolution_clock::now();
	auto timeOfLastMessage = std::chrono::high_resolution_clock::now();

	StateType currentIndex;
	CompressedState currentState;

	this->isInit = false;
	// Perform a search through the model.
	while (!this->statesToExplore.empty() && this->numberTerminal < Options::threads) {
		auto currentProbabilityStatePair = this->statesToExplore.front();
		this->currentProbabilityState = this->statesToExplore.front().first;
		currentState = this->statesToExplore.front().second;
		this->statesToExplore.pop_front();
		// Get the first state in the queue.
		currentIndex = this->currentProbabilityState->index;
		if (currentIndex == 0) {
			StaminaMessages::errorAndExit("Dequeued artificial absorbing state!");
		}

		// std::cout << "Dequeued state " << currentIndex << std::endl;
		// Set our state variable in the class

		if (currentIndex % MSG_FREQUENCY == 0) {
			StaminaMessages::info("Exploring state with id " + std::to_string(currentIndex) + ".");
		}

		if (choiceInformationBuilder.isBuildStateValuations()) {
			this->generator->addStateValuation(currentIndex, choiceInformationBuilder.stateValuationsBuilder());
		}

		// Load state for us to use
		this->generator->load(currentState);

		if (this->propertyExpression != nullptr) {
			storm::expressions::SimpleValuation valuation = this->generator->currentStateToSimpleValuation();
			bool evaluationAtCurrentState = this->propertyExpression->evaluateAsBool(&valuation);
			// If the property does not hold at the current state, make it absorbing in the
			// state graph and do not explore its successors
			if (!evaluationAtCurrentState) {
				transitionMatrixBuilder.addNextValue(this->currentRow, currentIndex, 1.0);
				// We treat this state as terminal even though it is also absorbing and does not
				// go to our artificial absorbing state
				this->currentProbabilityState->terminal = true;
				this->numberTerminal++;
				// Do NOT place this in the deque of states we should start with next iteration
				continue;
			}
		}

		// Add the state rewards to the corresponding reward models.
		// Do not explore if state is terminal and its reachability probability is less than kappa
		if (this->currentProbabilityState->isTerminal() && this->currentProbabilityState->getPi() < this->localKappa) {
			// Do not connect to absorbing yet
			// Place this in statesTerminatedLastIteration
			if ( !this->currentProbabilityState->wasPutInTerminalQueue ) {
				this->statesTerminatedLastIteration.emplace_back(currentProbabilityStatePair);
				this->currentProbabilityState->wasPutInTerminalQueue = true;
				++this->currentRow;
				++this->currentRowGroup;
			}
			continue;
		}

		// We assume that if we make it here, our state is either nonterminal, or its reachability probability
		// is greater than kappa
		// Expand (explore next states)
		storm::generator::StateBehavior<ValueType, StateType> behavior = this->generator->expand(stateToIdCallback);

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
			transitionMatrixBuilder.addNextValue(this->currentRow, currentIndex, 1.0);
			continue;
			// StaminaMessages::warn("Behavior for state " + std::to_string(currentIndex) + " was empty!");
		}

		bool shouldEnqueueAll = this->currentProbabilityState->getPi() == 0.0;
		// Now add all choices.
		bool firstChoiceOfState = true;
		for (auto const& choice : behavior) {
			if (!firstChoiceOfState) {
				StaminaMessages::errorAndExit("Model was not deterministic!");
			}
			// add the generated choice information
			if (choiceInformationBuilder.isBuildChoiceLabels() && choice.hasLabels()) {
				for (auto const& label : choice.getLabels()) {
					choiceInformationBuilder.addChoiceLabel(label, this->currentRow);

				}
			}
			if (choiceInformationBuilder.isBuildChoiceOrigins() && choice.hasOriginData()) {
				choiceInformationBuilder.addChoiceOriginData(choice.getOriginData(), this->currentRow);
			}
			if (choiceInformationBuilder.isBuildStatePlayerIndications() && choice.hasPlayerIndex()) {
				if (firstChoiceOfState) {
					choiceInformationBuilder.addStatePlayerIndication(choice.getPlayerIndex(), this->currentRowGroup);
				}
			}

			double totalRate = 0.0;
			if (!shouldEnqueueAll && this->isCtmc) {
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
				double probability = this->isCtmc ? stateProbabilityPair.second / totalRate : stateProbabilityPair.second;
				// Enqueue S is handled in stateToIdCallback
				// Update transition probability only if we should enqueue all
				// These are next states where the previous state has a reachability
				// greater than zero

				auto nextProbabilityState = this->stateMap.get(sPrime);
				if (nextProbabilityState != nullptr) {
					if (!shouldEnqueueAll) {
						nextProbabilityState->addToPi(this->currentProbabilityState->getPi() * probability);
					}

					if (this->currentProbabilityState->isNew) {
						this->createTransition(currentIndex, sPrime, stateProbabilityPair.second);
						this->numberTransitions++;
					}
				}
			}

			++this->currentRow;
			firstChoiceOfState = false;
		}

		this->currentProbabilityState->isNew = false;

		if (this->currentProbabilityState->isTerminal() && this->numberTerminal > 0) {
			this->numberTerminal--;
		}
		this->currentProbabilityState->setTerminal(false);
		this->currentProbabilityState->setPi(0.0);

		++this->currentRowGroup;

		if (this->generator->getOptions().isShowProgressSet()) {
			++this->numberOfExploredStatesSinceLastMessage;

			auto now = std::chrono::high_resolution_clock::now();
			auto durationSinceLastMessage = std::chrono::duration_cast<std::chrono::seconds>(now - timeOfLastMessage).count();
			if (static_cast<uint64_t>(durationSinceLastMessage) >= this->generator->getOptions().getShowProgressDelay()) {
				auto statesPerSecond = this->numberOfExploredStatesSinceLastMessage / durationSinceLastMessage;
				auto durationSinceStart = std::chrono::duration_cast<std::chrono::seconds>(now - timeOfStart).count();
				StaminaMessages::info(
					"Explored " + std::to_string(this->numberOfExploredStatesSinceLastMessage) + " states in " + std::to_string(durationSinceStart) + " seconds (currently " + std::to_string(statesPerSecond) + " states per second)."
				);
				timeOfLastMessage = std::chrono::high_resolution_clock::now();
				this->numberOfExploredStatesSinceLastMessage = 0;
			}
		}

	}
	this->iteration++;
	this->numberStates = this->stateStorage.stateToId.size(); // numberOfExploredStates;

	// Create threads and explore initial states
	// If numberTerminal is less than the number of threads, we are under-utilizing threads
	if (this->numberTerminal < Options::threads) {
		StaminaMessages::warning("The number of threads that will be utilized (" + std::to_string(this->numberTerminal) + ") is less than the number created! ( " + std::to_string(Options::threads) + ")");
	}
	else {
		STAMINA_DEBUG_MESSAGE("All threads in use");
	}

	/*
	 * The hold (auto set to true in thread constructor) make it so these threads don't just stop after starting.
	 * Therefore after everything is set up, we must turn off the hold. However:
	 *     + The threads must be STARTED in order to request cross exploration. TODO: do they?
	 *     + Threads won't die when hold is true.
	 */

	// Start control thread
	this->controlThread.startThread();

	STAMINA_DEBUG_MESSAGE("The size of exploration threads is " << this->explorationThreads.size());
	// Start all of the worker threads
	for (auto & explorationThread : this->explorationThreads) {
		explorationThread.startThread();
	}

	auto terminalStatesVector = this->getPerimeterStates(); // TODO: should this be all T states
	uint8_t threadIndex = 1;
	for (auto & terminalState : terminalStatesVector) {
		auto & explorationThread = this->explorationThreads[threadIndex];
		// TODO: ask for cross exploration from thread at that index
		if (threadIndex == Options::threads) {
			threadIndex = 1;
		}
		else {
			threadIndex++;
		}
	}

	// Remove the hold on all of the worker threads
	for (auto & explorationThread : this->explorationThreads) {
		explorationThread.setHold(false);
	}

	this->controlThread.setHold(false);

}

template class StaminaThreadedIterativeModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace builder
} // namespace stamina
