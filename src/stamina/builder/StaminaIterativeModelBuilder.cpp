/**
 * STAMINA - the [ST]ochasic [A]pproximate [M]odel-checker for [IN]finite-state [A]nalysis
 * Copyright (C) 2023 Fluent Verification, Utah State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 *
 **/

#include "StaminaIterativeModelBuilder.h"
#include "core/StateSpaceInformation.h"

#include <functional>
#include <sstream>

// #define CHECK_TERMINAL_COUNT

namespace stamina {
namespace builder {

template<typename ValueType, typename RewardModelType, typename StateType>
StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>::StaminaIterativeModelBuilder(
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
StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>::StaminaIterativeModelBuilder(
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
StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>::buildMatrices(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
	, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
	, StateAndChoiceInformationBuilder& stateAndChoiceInformationBuilder
	, boost::optional<storm::storage::BitVector>& markovianChoices
	, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
) {
	fresh = false;
	// numberTransitions = 0;
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
	while (!statesToExplore.empty()) {
		auto currentProbabilityStatePair = statesToExplore.front();
		currentProbabilityState = statesToExplore.front().first;
		currentState = statesToExplore.front().second;
		statesToExplore.pop_front();
		// Get the first state in the queue.
		currentIndex = currentProbabilityState->index;
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

		if (formulaMatchesExpression && !Options::no_prop_refine) {
			storm::expressions::SimpleValuation valuation = generator->currentStateToSimpleValuation();
			// bool evaluationAtCurrentState = propertyExpression->evaluateAsBool(&valuation);
			bool leftEvaluation = leftPropertyExpression->evaluateAsBool(&valuation);
			bool rightEvaluation = rightPropertyExpression->evaluateAsBool(&valuation);;
			// The left evaluation is, for a property P=?[ phi1 U[] phi2 ], the state evaluation
			// of phi1(s), and the right evaluation is phi2(s). Our formula for early termination
			// is !leftEvaluation || rightEvaluation, because
			//   - If !leftEvaluation we know that the property ALREADY fails, so no further
			//     exploration of the path will be useful
			//   - If rightEvaluation is evaluated IN THIS EXPRESSION, then we know that
			//     leftEvaluation evaluated to true. If leftEvaluation AND rightEvaluation
			//     are both true (as would happen here), then we know that the property
			//     SUCCEEDS, so again, no further evaluation needed
			if (!leftEvaluation || rightEvaluation) {
				this->createTransition(currentIndex, currentIndex, 1.0);
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

		if (currentProbabilityState->wasPutInTerminalQueue) {
			// Mark as not put in terminal queue
			// Note that it still will be IN the terminal queue, but
			// that when we flush this queue, it will be ignored
			currentProbabilityState->wasPutInTerminalQueue = false;
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
#if defined DIE_ON_DEADLOCK
			StaminaMessages::errorAndExit("Behavior for state " + std::to_string(currentIndex) + " was empty!");
#elif defined WARN_ON_DEADLOCK
			StaminaMessages::warning("State value caused empty behavior:\n" + StateSpaceInformation::stateToString(currentState));
#endif // DIE_ON_DEADLOCK / WARN_ON_DEADLOCK
			// If we are not yet aware that this is a deadlock state
			// we should make future iterations aware of this
			if (!currentProbabilityState->deadlock) {
				this->createTransition(currentIndex, currentIndex, 1.0);
				stateStorage.deadlockStateIndices.push_back(currentIndex);
				currentProbabilityState->deadlock = true;
			}
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
			if (choice.size() == 0) {
				StaminaMessages::warning("Found deadlock state (from model description): state ID " + std::to_string(currentIndex));
			}
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
						// numberTransitions++;
					}
				}
				// Sanity Check
				else if (currentProbabilityState->getPi() != 0) {
					StaminaMessages::errorAndExit("Should have gotten a successor for unexplored state!");
				}

			}

			if (currentIndex >= currentRow) {
				++currentRow;
			}
			firstChoiceOfState = false;
		}

		currentProbabilityState->isNew = false;
#ifdef CHECK_TERMINAL_COUNT
		uint32_t terminalCount = stateMap.getNumberTerminal();
		if (terminalCount != numberTerminal) {
			StaminaMessages::error("Terminal Count is wrong!\n\tActual: " + std::to_string(terminalCount) + "\n\tBookkept: " + std::to_string(numberTerminal));
		}
#endif // CHECK_TERMINAL_COUNT
		if (currentProbabilityState->isTerminal() && numberTerminal > 0) {
			numberTerminal--;
		}
		else if (currentProbabilityState->isTerminal()) {
			StaminaMessages::errorAndExit("Number terminal should have been positive, but was zero! (State was marked terminal but not accounted for!");
		}
		currentProbabilityState->setTerminal(false);
		currentProbabilityState->setPi(0.0);

		if (currentRow >= currentRowGroup) {
			++currentRowGroup;
		}

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
}

template <typename ValueType, typename RewardModelType, typename StateType>
StateType
StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex(CompressedState const& state) {
	if (state == this->absorbingState) {
		StaminaMessages::errorAndExit("Got Absorbing state in stateToIdCallback!");
		return 0;
	}
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
				// Sanity check
				if (nextProbabilityState->iterationLastSeen > iteration) {
					StaminaMessages::errorAndExit("Iteration last seen for state was greater than current iteration!");
				}
				nextProbabilityState->iterationLastSeen = iteration;
				// Enqueue
				statesToExplore.push_back(std::make_pair(nextProbabilityState, state));
				enqueued = true;
			}
		}
		else {
			// State does not exist yet in this iteration
			// so we must create it
			ProbabilityState<StateType> * nextProbabilityState = memoryPool.allocate();
			*nextProbabilityState = ProbabilityState<StateType>(
				actualIndex
				, 0.0
				, true
			);
			// Set the iteration last seen
			nextProbabilityState->iterationLastSeen = iteration;
			statesToExplore.push_back(std::make_pair(nextProbabilityState, state));
			enqueued = true;
		}
	}
	else {
		if (stateIsExisting) {
			// Don't rehash if we've already called find()
			ProbabilityState<StateType> * nextProbabilityState = nextState;
			// auto emplaced = exploredStates.emplace(actualIndex);
			if (nextProbabilityState->iterationLastSeen != iteration) {
				// Sanity check
				if (nextProbabilityState->iterationLastSeen > iteration) {
					StaminaMessages::errorAndExit("Iteration last seen for state was greater than current iteration!");
				}
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

template <typename ValueType, typename RewardModelType, typename StateType>
storm::storage::sparse::ModelComponents<ValueType, RewardModelType>
StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>::buildModelComponents() {
	StaminaMessages::info("Using STAMINA 2.5 Algorithm");
	// Is this model deterministic? (I.e., is there only one choice per state?)
	bool deterministic = generator->isDeterministicModel();
	if (!deterministic) {
		StaminaMessages::errorAndExit("Model is not deterministic! STAMINA only supports deterministic models!");
	}

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
		// Builds matrices and truncates state space
		buildMatrices(
				transitionMatrixBuilder
				, rewardModelBuilders
				, stateAndChoiceInformationBuilder
				, markovianStates
				, stateValuationsBuilder
				);

		piHat = this->accumulateProbabilities();
		innerLoopCount++;
	}

	// No remapping is necessary
	this->purgeAbsorbingTransitions();
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
StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>::flushStatesTerminated() {
	while (!statesTerminatedLastIteration.empty()) {
		auto probabilityStatePair = statesTerminatedLastIteration.front();
		// States can be marked as not put in terminal queue and when we flush the terminal queue we
		// ignore those estates
		if (! probabilityStatePair.first->wasPutInTerminalQueue) {
			statesTerminatedLastIteration.pop_front();
			continue;
		}
		statesToExplore.emplace_back(probabilityStatePair);
		probabilityStatePair.first->wasPutInTerminalQueue = false;
		statesTerminatedLastIteration.pop_front();
		probabilityStatePair.first->isNew = true;
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType>::connectAllTerminalStatesToAbsorbing(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
) {
	// The perimeter states require a second custom stateToIdCallback which does not enqueue or
	// register new states
	while (!statesTerminatedLastIteration.empty()) {
		auto currentProbabilityState = statesTerminatedLastIteration.front().first;
		auto state = statesTerminatedLastIteration.front().second;
// 		std::cerr << "Connecting state to absorbing" << StateSpaceInformation::stateToString(currentProbabilityState->state, currentProbabilityState->getPi()) << std::endl;
		statesTerminatedLastIteration.pop_front();
		// If the state is not marked as terminal, we've already connected it to absorbing
		// Additionally, if it was marked as not being put in the terminal queue, it should be
		// ignored by this step.
		if (!currentProbabilityState->isTerminal()
				|| !currentProbabilityState->wasPutInTerminalQueue
				|| currentProbabilityState->deadlock) {
			continue;
		}
		this->connectTerminalStatesToAbsorbing(
			transitionMatrixBuilder
			, state
			, currentProbabilityState->index
			, this->terminalStateToIdCallback
		);
		currentProbabilityState->setTerminal(false);
		currentProbabilityState->wasPutInTerminalQueue = false;
	}
}

template class StaminaIterativeModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

} // namespace builder
} // namespace stamina
