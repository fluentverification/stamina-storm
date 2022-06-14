#include "StaminaReExploringModelBuilder.h"
#include "../StateSpaceInformation.h"

#include <functional>
#include <sstream>

namespace stamina {
namespace builder {

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaReExploringModelBuilder<ValueType, RewardModelType, StateType>::StaminaReExploringModelBuilder(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, storm::prism::Program const& modulesFile
	, storm::generator::NextStateGeneratorOptions const & options
) // Invoke super constructor
	: StaminaModelBuilder<ValueType, RewardModelType, StateType>(
		generator
		, modulesFile
		, options
	)
{
	// Intentionally left empty
}

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaReExploringModelBuilder<ValueType, RewardModelType, StateType>::StaminaReExploringModelBuilder(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions
) // Invoke super constructor
	: StaminaModelBuilder<ValueType, RewardModelType, StateType>(
		program
		, generatorOptions
	)
{
	// Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaReExploringModelBuilder<ValueType, RewardModelType, StateType>::buildMatrices(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
	, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
	, StateAndChoiceInformationBuilder& stateAndChoiceInformationBuilder
	, boost::optional<storm::storage::BitVector>& markovianChoices
	, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
) {
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
		&StaminaReExploringModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex
		, this
		, std::placeholders::_1
	);
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
	if (firstIteration) {
		currentRowGroup = 1;
		currentRow = 1;
	}
	numberOfExploredStates = 0;
	numberOfExploredStatesSinceLastMessage = 0;


	auto timeOfStart = std::chrono::high_resolution_clock::now();
	auto timeOfLastMessage = std::chrono::high_resolution_clock::now();

	StateType currentIndex;
	CompressedState currentState;

	isInit = false;
	// Perform a search through the model.
	while (!statesToExplore.empty()) {
		currentProbabilityState = statesToExplore.front();
		statesToExplore.pop_front();
		// Get the first state in the queue.
		currentIndex = currentProbabilityState->index;
		currentState = currentProbabilityState->state;

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
				this->createTransition(currentRow, currentIndex, 1.0);
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
			// Do not connect to absorbing yet--only connect at the end
			statesTerminatedLastIteration.push_back(currentProbabilityState);
			++numberOfExploredStates;
			++currentRow;
			++currentRowGroup;
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
			this->createTransition(currentRow, currentIndex, 1.0);
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

			if (currentIndex >= currentRow) {
				++currentRow;
			}
			firstChoiceOfState = false;
		}
		currentProbabilityState->isNew = false;
		if (currentProbabilityState->isTerminal() && numberTerminal > 0) {
			numberTerminal--;
		}
		currentProbabilityState->setTerminal(false);
		currentProbabilityState->setPi(0.0);

		if (currentRow >= currentRowGroup) {
			++currentRowGroup;
		}

		++numberOfExploredStates;
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

	}
	iteration++;
	numberStates = numberOfExploredStates;
	firstIteration = false;
// 	std::cout << "State space truncation finished for this iteration. Explored " << numberStates << " states. pi = " << accumulateProbabilities() << std::endl;
}

template <typename ValueType, typename RewardModelType, typename StateType>
StateType
StaminaReExploringModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex(CompressedState const& state) {
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
			ProbabilityState * initProbabilityState = memoryPool.allocate();
			*initProbabilityState = ProbabilityState(
				state
				, actualIndex
				, 1.0
				, true
			);
			numberTerminal++;
			stateMap.put(actualIndex, initProbabilityState);
			statesToExplore.emplace_back(initProbabilityState);
			initProbabilityState->iterationLastSeen = iteration;
		}
		else {
			ProbabilityState * initProbabilityState = nextState;
			stateMap.put(actualIndex, initProbabilityState);
			statesToExplore.push_back(initProbabilityState);
			initProbabilityState->iterationLastSeen = iteration;
		}
		if (actualIndex == newIndex) {
			stateRemapping.get().push_back(storm::utility::zero<StateType>());
		}
		return actualIndex;
	}

	bool enqueued = false;

	// This bit handles the non-initial states
	// The previous state has reachability of 0
	if (currentProbabilityState->getPi() == 0) {
		if (stateIsExisting) {
			// Don't rehash if we've already called find()
			ProbabilityState * nextProbabilityState = nextState;
			if (nextProbabilityState->iterationLastSeen != iteration) {
				nextProbabilityState->iterationLastSeen = iteration;
				// Enqueue
				statesToExplore.push_back(nextProbabilityState);
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
			ProbabilityState * nextProbabilityState = nextState;
			// auto emplaced = exploredStates.emplace(actualIndex);
			if (nextProbabilityState->iterationLastSeen != iteration) {
				nextProbabilityState->iterationLastSeen = iteration;
				// Enqueue
				statesToExplore.push_back(nextProbabilityState);
				enqueued = true;
			}
		}
		else {
			// This state has not been seen so create a new ProbabilityState
			ProbabilityState * nextProbabilityState = memoryPool.allocate();
			*nextProbabilityState = ProbabilityState(state, actualIndex, 0.0, true);
			stateMap.put(actualIndex, nextProbabilityState);
			nextProbabilityState->iterationLastSeen = iteration;
			// exploredStates.emplace(actualIndex);
			statesToExplore.push_back(nextProbabilityState);
			enqueued = true;
			numberTerminal++;
		}
	}
	if (enqueued && actualIndex == newIndex) {
		stateRemapping.get().push_back(storm::utility::zero<StateType>());
	}
	return actualIndex;
}

template <typename ValueType, typename RewardModelType, typename StateType>
storm::storage::sparse::ModelComponents<ValueType, RewardModelType>
StaminaReExploringModelBuilder<ValueType, RewardModelType, StateType>::buildModelComponents() {
	StaminaMessages::info("Using STAMINA 2.0 Algorithm");
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

	// Component builders
	storm::storage::SparseMatrixBuilder<ValueType> transitionMatrixBuilder(
			0
			, 0
			, 0
			, false
			, false // All models are deterministic
			, 0
		);

	double piHat = 1.0;
	int innerLoopCount = 0;

	// Continuously decrement kappa
	while (piHat >= Options::prob_win / Options::approx_factor) {
		statesTerminatedLastIteration.clear();
		// Builds matrices and truncates state space
		buildMatrices(
			transitionMatrixBuilder
			, rewardModelBuilders
			, stateAndChoiceInformationBuilder
			, markovianStates
			, stateValuationsBuilder
		);

		generator = std::make_shared<storm::generator::PrismNextStateGenerator<ValueType, StateType>>(modulesFile, this->options);

		piHat = this->accumulateProbabilities();
		innerLoopCount++;
	}

	this->printStateSpaceInformation();

	// No remapping is necessary
	connectAllTerminalStatesToAbsorbing(transitionMatrixBuilder);
	this->flushToTransitionMatrix(transitionMatrixBuilder);

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
StaminaReExploringModelBuilder<ValueType, RewardModelType, StateType>::connectAllTerminalStatesToAbsorbing(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
) {
// 	std::cout << "connecting all terminal states to absorbing" << std::endl;
// 	std::cout << "The number of states to connect is " << statesTerminatedLastIteration.size() << "." << std::endl;
	// The perimeter states require a second custom stateToIdCallback which does not enqueue or
	// register new states
	while (!statesTerminatedLastIteration.empty()) {
		auto currentProbabilityState = statesTerminatedLastIteration.front();
// 		std::cout << "Connecting state " << StateSpaceInformation::stateToString(currentProbabilityState->state, 0) << " to terminal" << std::endl;
		this->connectTerminalStatesToAbsorbing(
			transitionMatrixBuilder
			, currentProbabilityState->state
			, currentProbabilityState->index
			, this->terminalStateToIdCallback
		);
		statesTerminatedLastIteration.pop_front();
	}
}

template class StaminaReExploringModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace builder
} // namespace stamina
