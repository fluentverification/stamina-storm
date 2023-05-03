#include "StaminaPriorityModelBuilder.h"

#include "core/StateSpaceInformation.h"
#include "priority/EventStatePriority.h"

#include <functional>
#include <sstream>
#include <cmath>

// Pre-load factor for hash-map
#define PRE_LOAD 1000000

namespace stamina {
namespace builder {

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::StaminaPriorityModelBuilder(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, storm::prism::Program const& modulesFile
	, storm::generator::NextStateGeneratorOptions const & options
) // Invoke super constructor
	: StaminaModelBuilder<ValueType, RewardModelType, StateType>(
		generator
		, modulesFile
		, options
	)
	, preTerminatedStates(PRE_LOAD * (int) Options::preterminate) // pre-size our hashmap. The "*Options::preterminate" prevents resizing if no pretermination occurs
{
	setupStatePriority(modulesFile.getManager());
}

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::StaminaPriorityModelBuilder(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions
) // Invoke super constructor
	: StaminaModelBuilder<ValueType, RewardModelType, StateType>(
		program
		, generatorOptions
	)
	, preTerminatedStates(PRE_LOAD * (int) Options::preterminate) // pre-size our hashmap. The "*Options::preterminate" prevents resizing if no pretermination occurs
{
	setupStatePriority(program.getManager());
}

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::~StaminaPriorityModelBuilder() {
	if (this->statePriority) {
		delete this->statePriority;
	}
}

template<typename ValueType, typename RewardModelType, typename StateType>
void
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::initializeEventStatePriority(storm::jani::Property * property) {
	if (Options::event == EVENTS::UNDEFINED) { return; }
	// This method assumes you're using an event state priority
	statePriority->initialize(property);
}

template<typename ValueType, typename RewardModelType, typename StateType>
StateType
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex(CompressedState const& state) {
	StateType actualIndex;
	StateType newIndex = static_cast<StateType>(stateStorage.getNumberOfStates());
	if (stateStorage.stateToId.contains(state)) {
		actualIndex = stateStorage.stateToId.getValue(state);
	}
	else {
		// Create new index just in case we need it
		actualIndex = newIndex;
	}

	auto nextState = stateMap.get(actualIndex);
	bool stateIsExisting = nextState != nullptr;

	stateStorage.stateToId.findOrAdd(state, actualIndex);
	// Handle conditional enqueuing
	if (isInit) {
		if (!stateIsExisting) {
			// Create a ProbabilityState for each individual state
			ProbabilityState<StateType> * initProbabilityState = memoryPool.allocate();
			*initProbabilityState = ProbabilityState(
				actualIndex
				, 1.0
				, true
			);
			numberTerminal++;
			stateMap.put(actualIndex, initProbabilityState);
			// Explicitly enqueue the initial state--do not use enqueue()
			std::shared_ptr<ProbabilityStatePair<StateType>> initProbabilityStatePair(
				new ProbabilityStatePair<StateType>(initProbabilityState, state)
			);
			if (this->statePriority) {
				statePriority->priority(initProbabilityStatePair);
			}
			statePriorityQueue.push(initProbabilityStatePair);
			initProbabilityState->iterationLastSeen = iteration;
		}
		else {
			StaminaMessages::errorAndExit("Initial state should not exist yet, but does!");
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
				std::shared_ptr<ProbabilityStatePair<StateType>> nextProbabilityStatePair(
					new ProbabilityStatePair<StateType>(nextProbabilityState, state)
				);
				if (this->statePriority) {
					statePriority->priority(nextProbabilityStatePair);
				}
				enqueue(nextProbabilityStatePair);
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
				std::shared_ptr<ProbabilityStatePair<StateType>> nextProbabilityStatePair(
					new ProbabilityStatePair<StateType>(nextProbabilityState, state)
				);
				if (this->statePriority) {
					statePriority->priority(nextProbabilityStatePair);
				}
				enqueue(nextProbabilityStatePair);
				enqueued = true;
			}
		}
		else {
			// This state has not been seen so create a new ProbabilityState
			ProbabilityState<StateType> * nextProbabilityState = memoryPool.allocate();
			*nextProbabilityState = ProbabilityState(
				actualIndex
				, 0.0
				, true
			);
			stateMap.put(actualIndex, nextProbabilityState);
			nextProbabilityState->iterationLastSeen = iteration;
			// exploredStates.emplace(actualIndex);
			std::shared_ptr<ProbabilityStatePair<StateType>> nextProbabilityStatePair(
				new ProbabilityStatePair<StateType>(nextProbabilityState, state)
			);
			if (this->statePriority) {
				statePriority->priority(nextProbabilityStatePair);
			}
			enqueue(nextProbabilityStatePair);

			enqueued = true;
			numberTerminal++;
		}
	}
	return actualIndex;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::enqueue(std::shared_ptr<ProbabilityStatePair<StateType>> probabilityStatePair) {
	// Do not preterminate
	if (!Options::preterminate) {
		statePriorityQueue.push(probabilityStatePair);
		return;
	}

	auto probabilityState = probabilityStatePair->first;
	auto stateReachability = probabilityStatePair->first->getPi();
	auto state = probabilityStatePair->second;
	// We should be somewhat conscious of the reachability that may get added
	double halfNextReachability = stateReachability + this->currentProbabilityState->getPi() / 2;


	bool preTerminateThisIteration = halfNextReachability < windowPower / (double) numberOfExploredStates;
	// Our state is not pre-terminated, and should not be
	if (!probabilityState->isPreTerminated() && !preTerminateThisIteration) {
		statePriorityQueue.push(probabilityStatePair);
		return;
	}
	// Our state is preterminated and should stay that way
	else if (probabilityState->isPreTerminated() && preTerminateThisIteration) {
		return;
	}
	// Only call find() once
	auto inPreTerminatedSet = preTerminatedStates.find(state) != preTerminatedStates.end();
	if (preTerminateThisIteration && !inPreTerminatedSet) {
		// It is the main loop's responsibility to insert the transition
		preTerminatedStates.insert({state, probabilityState->index});
		std::shared_ptr<std::vector<TransitionInfo>> transitionVector(new std::vector<TransitionInfo>());
		probabilityState->preTerminatedTransitions = transitionVector;
		probabilityState->setPreTerminated(true);
	}
	// If it is preterminated but should be un-preterminated
	else if (inPreTerminatedSet && !preTerminateThisIteration) {
		preTerminatedStates.erase(state);
		probabilityState->setPreTerminated(false);
		statePriorityQueue.push(probabilityStatePair);
		for (auto transition : *probabilityState->preTerminatedTransitions) {
			this->createTransition(transition);
		}
		probabilityState->preTerminatedTransitions = nullptr;
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::setupStatePriority(storm::expressions::ExpressionManager & manager) {
	this->statePriority = nullptr;
	switch (Options::event) {
		case EVENTS::RARE:
			this->statePriority = new priority::EventStatePriority<StateType>(true, manager);
			break;
		case EVENTS::COMMON:
			this->statePriority = new priority::EventStatePriority<StateType>(false, manager);
			break;
		case EVENTS::UNDEFINED:
			return;
		default:
			StaminaMessages::errorAndExit("Unknown error! (StaminaPriorityModelBuilder::setupStatePriority())");
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
storm::storage::sparse::ModelComponents<ValueType, RewardModelType>
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::buildModelComponents() {
	StaminaMessages::info("Using STAMINA 3.0 Algorithm");
// 	StaminaMessages::errorAndExit("STAMINA 3.0 is not yet implemented!");
	// Is this model deterministic? (I.e., is there only one choice per state?)
	bool deterministic = generator->isDeterministicModel();

	std::vector<RewardModelBuilder<typename RewardModelType::ValueType>> rewardModelBuilders;
	// Iterate through the reward models and add them to the rewardmodelbuilders
	for (uint64_t i = 0; i < generator->getNumberOfRewardModels(); ++i) {
		rewardModelBuilders.emplace_back(generator->getRewardModelInformation(i));
	}

	// Build choice information and markovian states
	storm::builder::StateAndChoiceInformationBuilder stateAndChoiceInformationBuilder;
	boost::optional<storm::storage::BitVector> markovianStates;

	// Build state valuations if necessary. We may not need this since we operate only on CTMC
	boost::optional<storm::storage::sparse::StateValuationsBuilder> stateValuationsBuilder;
	if (generator->getOptions().isBuildStateValuationsSet()) {
		stateValuationsBuilder = generator->initializeStateValuationsBuilder();
	}
	stateAndChoiceInformationBuilder.setBuildChoiceLabels(generator->getOptions().isBuildChoiceLabelsSet());
	stateAndChoiceInformationBuilder.setBuildChoiceOrigins(generator->getOptions().isBuildChoiceOriginsSet());
	stateAndChoiceInformationBuilder.setBuildStatePlayerIndications(false); // Only applies to SMGs
	stateAndChoiceInformationBuilder.setBuildMarkovianStates(false); // Only applies to markov automata
	stateAndChoiceInformationBuilder.setBuildStateValuations(generator->getOptions().isBuildStateValuationsSet());

	StateSpaceInformation::setVariableInformation(generator->getVariableInformation());

	if (!firstIteration) {
		StaminaMessages::errorAndExit("Could not acquire a satisfactory result!\n\tMore Info: Multiple iterations are currently not supported in the Priority method.\n\tTry again with a higher fudge factor (-F)");
	}
	// Component builders
	storm::storage::SparseMatrixBuilder<ValueType> transitionMatrixBuilder(
			0
			, 0
			, 0
			, false
			, false // All models are deterministic
			, 0
		);

		// Builds matrices and truncates state space
		buildMatrices(
			transitionMatrixBuilder
			, rewardModelBuilders
			, stateAndChoiceInformationBuilder
			, markovianStates
			, stateValuationsBuilder
		);

	// No remapping is necessary
	this->purgeAbsorbingTransitions();
	connectAllTerminalStatesToAbsorbing(transitionMatrixBuilder);
	this->flushToTransitionMatrix(transitionMatrixBuilder);

	generator = std::make_shared<storm::generator::PrismNextStateGenerator<ValueType, StateType>>(modulesFile, this->options);
	this->setGenerator(generator);

	// Using the information from buildMatrices, initialize the model components
	storm::storage::sparse::ModelComponents<ValueType, RewardModelType> modelComponents(
		transitionMatrixBuilder.build(0, transitionMatrixBuilder.getCurrentRowGroupCount())
		, this->buildStateLabeling()
		, std::unordered_map<std::string, RewardModelType>()
		, !generator->isDiscreteTimeModel()
		, std::move(markovianStates)
	);

	// Build choice labeling
	if (stateAndChoiceInformationBuilder.isBuildStatePlayerIndications()) {
		modelComponents.choiceLabeling = stateAndChoiceInformationBuilder.buildChoiceLabeling(modelComponents.transitionMatrix.getRowCount());
	}
	if (generator->getOptions().isBuildChoiceOriginsSet()) {
		auto originData = stateAndChoiceInformationBuilder.buildDataOfChoiceOrigins(modelComponents.transitionMatrix.getRowCount());
		modelComponents.choiceOrigins = generator->generateChoiceOrigins(originData);
	}
	if (generator->isPartiallyObservable()) {
		std::vector<uint32_t> classes;
		classes.resize(stateStorage.getNumberOfStates());
		std::unordered_map<uint32_t, std::vector<std::pair<std::vector<std::string>, uint32_t>>> observationActions;
		for (auto const& bitVectorIndexPair : stateStorage.stateToId) {
			uint32_t varObservation = generator->observabilityClass(bitVectorIndexPair.first);
			classes[bitVectorIndexPair.second] = varObservation;
		}

		modelComponents.observabilityClasses = classes;
		if(generator->getOptions().isBuildObservationValuationsSet()) {
			modelComponents.observationValuations = generator->makeObservationValuation();
		}
	}
	return modelComponents;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::buildMatrices(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
	, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
	, StateAndChoiceInformationBuilder& stateAndChoiceInformationBuilder
	, boost::optional<storm::storage::BitVector>& markovianChoices
	, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
) {
	piHat = 1.0;
	numberTransitions = 0;
	// Builds model
	// Initialize building state valuations (if necessary)
	if (stateAndChoiceInformationBuilder.isBuildStateValuations()) {
		stateAndChoiceInformationBuilder.stateValuationsBuilder() = generator->initializeStateValuationsBuilder();
	}

	this->loadPropertyExpressionFromFormula();

	// Create a callback for the next-state generator to enable it to request the index of states.
	std::function<StateType (CompressedState const&)> stateToIdCallback = std::bind(
		&StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex
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
		numberOfExploredStates = 0;
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

	bool hold = true;
	windowPower = 0; // Always explore at least the first state
	// Perform a search through the model.
	while (hold || (!statePriorityQueue.empty() && (piHat > windowPower / Options::approx_factor))) {
		hold = false;
		auto currentProbabilityStatePair = *statePriorityQueue.top();
		currentProbabilityState = statePriorityQueue.top()->first;
		// std::cout << "Current pi: " << currentProbabilityState->pi << std::endl;
		currentState = statePriorityQueue.top()->second;
		currentIndex = currentProbabilityState->index;
		statePriorityQueue.pop();
		if (currentIndex == 0) {
			StaminaMessages::errorAndExit("Dequeued artificial absorbing state!");
		}

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
				transitionMatrixBuilder.addNextValue(currentIndex, currentIndex, 1.0);
				// We treat this state as terminal even though it is also absorbing and does not
				// go to our artificial absorbing state
				currentProbabilityState->terminal = true;
				numberTerminal++;
				// Do NOT place this in the deque of states we should start with next iteration
				continue;
			}
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
			transitionMatrixBuilder.addNextValue(currentIndex, currentIndex, 1.0);
			continue;
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
					stateAndChoiceInformationBuilder.addChoiceLabel(label, currentIndex);

				}
			}
			if (stateAndChoiceInformationBuilder.isBuildChoiceOrigins() && choice.hasOriginData()) {
				stateAndChoiceInformationBuilder.addChoiceOriginData(choice.getOriginData(), currentIndex);
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
						double piToAdd = currentProbabilityState->getPi() * probability;
						nextProbabilityState->addToPi(piToAdd);
						if (nextProbabilityState->isTerminal()) {
							// std::cout << "Adding " << piToAdd << " to pi" << std::endl;
							piHat += piToAdd;
							// std::cout << "Now piHat is " << piHat << std::endl;
						}
					}


					if (currentProbabilityState->isNew) {
						if (!nextProbabilityState->isPreTerminated()) {
							// Our state is not pre-terminated
							this->createTransition(currentIndex, sPrime, stateProbabilityPair.second);
							// std::cout << "Current index, sPrime, probability: " << currentIndex << ", " << sPrime << ", " << stateProbabilityPair.second << std::endl;
							numberTransitions++;
						}
						else {
							if (!nextProbabilityState->preTerminatedTransitions) {
								StaminaMessages::error("Error with set of preterminated states!");
							}
							// Our state is preterminated and we must keep track of the transition we would have inserted in case we un-preterminate it
							nextProbabilityState->preTerminatedTransitions->emplace_back(TransitionInfo(currentIndex, sPrime, stateProbabilityPair.second));
						}
					}
				}
			}

			++currentRow;
			firstChoiceOfState = false;
		}

		currentProbabilityState->isNew = false;

		if (currentProbabilityState->isTerminal() && numberTerminal > 0) {
			numberTerminal--;
			// std::cout << "Before editing piHat, it is " << piHat << std::endl;
			// std::cout << "Subtracting " << currentProbabilityState->getPi() <<  " from piHat" << std::endl;
			piHat -= currentProbabilityState->getPi();
			// std::cout << "Now piHat is " << piHat << std::endl;
		}
		else if (currentProbabilityState->isTerminal()) {
			StaminaMessages::error("numberTerminal is equal to " + std::to_string(numberTerminal));
		}
		currentProbabilityState->setTerminal(false);
		currentProbabilityState->setPi(0.0);

		++currentRowGroup;

		++numberOfExploredStates;
		windowPower = pow(Options::prob_win, Options::fudge_factor * (std::log10(std::max(numberOfExploredStates, (uint64_t) 2))));
		if (generator->getOptions().isShowProgressSet()) {
			++numberOfExploredStatesSinceLastMessage;

			auto now = std::chrono::high_resolution_clock::now();
			auto durationSinceLastMessage = std::chrono::duration_cast<std::chrono::seconds>(now - timeOfLastMessage).count();
			if (static_cast<uint64_t>(durationSinceLastMessage) >= generator->getOptions().getShowProgressDelay()) {
				auto statesPerSecond = numberOfExploredStatesSinceLastMessage / durationSinceLastMessage;
				auto durationSinceStart = std::chrono::duration_cast<std::chrono::seconds>(now - timeOfStart).count();
				StaminaMessages::info(
					"Explored " + std::to_string(numberOfExploredStates) + " states in " + std::to_string(durationSinceStart) + " seconds (currently " + std::to_string(statesPerSecond) + " states per second)."
				);
				timeOfLastMessage = std::chrono::high_resolution_clock::now();
				numberOfExploredStatesSinceLastMessage = 0;
			}
		}
		// std::cout << piHat << " <=? " << Options::prob_win / Options::approx_factor << std::endl;
		// std::cout << "statePriorityQueue size is " << statePriorityQueue.size() << std::endl;
		// std::cout << "At this iteration, piHat = " << piHat << " and numberTerminal is " << numberTerminal << std::endl;
		// std::cout << "while condition: " << (!statePriorityQueue.empty() && (piHat > Options::prob_win / Options::approx_factor)) << std::endl;
		// std::cout << "From: \n !statePriorityQueue.empty() = " << !statePriorityQueue.empty() << std::endl;
		// std::cout << "(piHat > Options::prob_win / Options::approx_factor) = " << (piHat > (Options::prob_win / Options::approx_factor)) << std::endl;
	}
	this->flushFromPriorityQueueToStatesTerminated();
	// std::cout << "while condition at termination: " << (!statePriorityQueue.empty() && (piHat > Options::prob_win / Options::approx_factor)) << std::endl;
	// std::cout << "From: \n !statePriorityQueue.empty() = " << !statePriorityQueue.empty() << std::endl;
	// std::cout << "(piHat > Options::prob_win / Options::approx_factor) = " << (piHat > (Options::prob_win / Options::approx_factor)) << std::endl;

	numberStates = stateStorage.stateToId.size(); // numberOfExploredStates;

	this->printStateSpaceInformation();
	StaminaMessages::info("Perimeter reachability is " + std::to_string(piHat));

}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::flushFromPriorityQueueToStatesTerminated() {
	// Terminal states are any remaining states in the state transition queue
	while (!statePriorityQueue.empty()) {
		auto currentProbabilityStatePair = statePriorityQueue.top();
		auto currentProbabilityState = statePriorityQueue.top()->first;
		auto currentState = statePriorityQueue.top()->second;
		statePriorityQueue.pop();
		if (!currentProbabilityState->wasPutInTerminalQueue) {
			// Again, rather than removing this state from the terminal queue, we simply ignore it when we iterate through that queue.
			continue;
		}
		statesTerminatedLastIteration.push_back(currentProbabilityStatePair);
	}
	// flush from the preterminated states
	// TODO: make sure no preterminated states were "un"-preterminated
	uint32_t numberOfPreTerminatedStates = 0;
	uint32_t numberOfPreTerminatedTransitions = 0;
	if (!Options::preterminate) {
		return;
	}
	for (auto const & [stateValues, stateId] : preTerminatedStates) {
		numberOfPreTerminatedStates++;
		auto transitions = stateMap.get(stateId)->preTerminatedTransitions;
		if (!transitions) {
			StaminaMessages::errorAndExit("Preterminated transition vector was null!");
		}
		// Loop over each transition in the terminated vector
		for (auto const & transition : *transitions) {
			// actually create the transition
			this->createTransition(transition.from, 0, transition.transition);
			// Make preterminated state have a self loop
			this->createTransition(transition.to, transition.to, 1.0);
			numberOfPreTerminatedTransitions++;
		}
	}
	StaminaMessages::info(std::to_string(numberOfPreTerminatedStates) + " states were pre-terminated, eliminating " + std::to_string(numberOfPreTerminatedTransitions) + " transitions.");
}


template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaPriorityModelBuilder<ValueType, RewardModelType, StateType>::connectAllTerminalStatesToAbsorbing(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
) {
	// Terminal states are any remaining states in the state transition queue
	while (!statesTerminatedLastIteration.empty()) {
		auto currentProbabilityState = statesTerminatedLastIteration.front()->first;
		auto state = statesTerminatedLastIteration.front()->second;
		statesTerminatedLastIteration.pop_front();
		// If the state is not marked as terminal, we've already connected it to absorbing
		if (!currentProbabilityState->isTerminal()) {
			continue;
		}
		this->connectTerminalStatesToAbsorbing(
			transitionMatrixBuilder
			, state
			, currentProbabilityState->index
			, this->terminalStateToIdCallback
		);
		currentProbabilityState->setTerminal(false);
	}

}

template class StaminaPriorityModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace builder
} // namespace stamina
