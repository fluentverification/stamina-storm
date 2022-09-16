#include "StaminaThreadedIterativeModelBuilder.h"

#include "core/StaminaMessages.h"


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

	// Another callback function to keep track of terminal states
	std::function<StateType (CompressedState const&)> stateToIdCallbackWithTerminalTracking = std::bind(
		&StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndexAndTrackTerminal
		, this
		, std::placeholders::_1
	);

	// Create exploration threads
	if (!controlThreadsCreated) {
		uint8_t threadIndex = 1;
		while (explorationThreads.size() < Options::threads) {
			explorationThreads.push_back(
				new threads::IterativeExplorationThread<ValueType, RewardModelType, StateType>(
					this // parent
					, threadIndex // Thread index
					, this->controlThread // Control Thread
					, this->getGenerator()->getVariableInformation().getTotalBitOffset(true)// state size
					, & this->getStateMap()
					, this->getGenerator()
					, stateToIdCallback
				)
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
		this->stateStorage.initialStateIndices = this->generator->getInitialStates(stateToIdCallbackWithTerminalTracking);
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

	// Put all initial states in fastTerminalStates
	// If this doesn't work, change to a while loop that re-enqueues
	for (auto initProbabilityStatePair : this->statesToExplore) {
		fastTerminalStates.emplace_back(initProbabilityStatePair.second);
	}
	// Perform a search through the model.
	while (!this->statesToExplore.empty() && this->numberTerminal < Options::threads) {
		auto currentProbabilityStatePair = this->statesToExplore.front();
		this->currentProbabilityState = this->statesToExplore.front().first;
		currentState = this->statesToExplore.front().second;
		this->statesToExplore.pop_front();
		fastTerminalStates.pop_front();
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
			// Add to fastTerminal
			fastTerminalStates.emplace_back(currentState);
			continue;
		}

		// We assume that if we make it here, our state is either nonterminal, or its reachability probability
		// is greater than kappa
		// Expand (explore next states)
		storm::generator::StateBehavior<ValueType, StateType> behavior = this->generator->expand(stateToIdCallbackWithTerminalTracking);

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
		explorationThread->startThread();
	}

	uint8_t threadIndex = 1;
	for (auto & terminalState : this->fastTerminalStates) {
		auto & explorationThread = this->explorationThreads[threadIndex];
		// TODO: ask for cross exploration from thread at that index
		// auto state = this->stateMap.get(terminalState);
		STAMINA_DEBUG_MESSAGE("Requesting cross exploration of state to thread " << threadIndex);
		explorationThread->requestCrossExploration(terminalState, 0.0);
		if (threadIndex == Options::threads) {
			threadIndex = 1;
		}
		else {
			threadIndex++;
		}
	}

	// Remove the hold on all of the worker threads
	for (auto & explorationThread : this->explorationThreads) {
		explorationThread->setHold(false);
	}

	this->controlThread.setHold(false);

	for (auto explorationThread : this->explorationThreads) {
		explorationThread->join();
		STAMINA_DEBUG_MESSAGE("Exploration thread finished");
	}
	// Control thread must be the one to terminate the other threads, so it must be alive when they all
	// are joined
	this->controlThread.join();
	STAMINA_DEBUG_MESSAGE("Control thread finished");

}

template <typename ValueType, typename RewardModelType, typename StateType>
StateType
StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndexAndTrackTerminal(CompressedState const& state) {
	StateType actualIndex;
	StateType newIndex = static_cast<StateType>(this->stateStorage.getNumberOfStates());
	if (this->stateStorage.stateToId.contains(state)) {
		actualIndex = this->stateStorage.stateToId.getValue(state);
	}
	else {
		// Create new index just in case we need it
		actualIndex = newIndex;
	}

	auto nextState = this->stateMap.get(actualIndex);
	bool stateIsExisting = nextState != nullptr;

	this->stateStorage.stateToId.findOrAdd(state, actualIndex);
	// Handle conditional enqueuing
	if (this->isInit) {
		if (!stateIsExisting) {
			// Create a ProbabilityState for each individual state
			ProbabilityState<StateType> * initProbabilityState = this->memoryPool.allocate();
			*initProbabilityState = ProbabilityState<StateType>(
				actualIndex
				, 1.0
				, true
			);
			// Add to fast terminal states
			fastTerminalStates.emplace_back(state);
			this->numberTerminal++;
			this->stateMap.put(actualIndex, initProbabilityState);
			this->statesToExplore.push_back(std::make_pair(initProbabilityState, state));
			initProbabilityState->iterationLastSeen = this->iteration;
		}
		else {
			ProbabilityState<StateType> * initProbabilityState = nextState;
			this->stateMap.put(actualIndex, initProbabilityState);
			this->statesToExplore.push_back(std::make_pair(initProbabilityState, state));
			initProbabilityState->iterationLastSeen = this->iteration;
		}
		return actualIndex;
	}

	bool enqueued = false;

	// This bit handles the non-initial states
	// The previous state has reachability of 0
	if (this->currentProbabilityState->getPi() == 0) {
		if (stateIsExisting) {
			// Don't rehash if we've already called find()
			ProbabilityState<StateType> * nextProbabilityState = nextState;
			if (nextProbabilityState->iterationLastSeen != this->iteration) {
				nextProbabilityState->iterationLastSeen = this->iteration;
				// Enqueue
				this->statesToExplore.push_back(std::make_pair(nextProbabilityState, state));
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
			if (nextProbabilityState->iterationLastSeen != this->iteration) {
				nextProbabilityState->iterationLastSeen = this->iteration;
				// Enqueue
				this->statesToExplore.push_back(std::make_pair(nextProbabilityState, state));
				enqueued = true;
			}
		}
		else {
			// This state has not been seen so create a new ProbabilityState
			ProbabilityState<StateType> * nextProbabilityState = this->memoryPool.allocate();
			*nextProbabilityState = ProbabilityState<StateType>(
				actualIndex
				, 0.0
				, true
			);
			// Add to fast terminal states
			fastTerminalStates.emplace_back(state);
			this->stateMap.put(actualIndex, nextProbabilityState);
			nextProbabilityState->iterationLastSeen = this->iteration;
			// exploredStates.emplace(actualIndex);
			this->statesToExplore.push_back(std::make_pair(nextProbabilityState, state));
			enqueued = true;
			this->numberTerminal++;
		}
	}
	return actualIndex;
}
template <typename ValueType, typename RewardModelType, typename StateType>
std::vector<typename threads::ExplorationThread<ValueType, RewardModelType, StateType> *> const &
StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType>::getExplorationThreads() const {
	std::vector<typename threads::ExplorationThread<ValueType, RewardModelType, StateType> *> currentExplorationThreads;
	for (auto thread : explorationThreads) {
		currentExplorationThreads.push_back(thread);
	}
	return currentExplorationThreads;
}

template class StaminaThreadedIterativeModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace builder
} // namespace stamina
