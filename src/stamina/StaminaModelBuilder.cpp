/**
* Implementation for StaminaModelBuilder methods.
*
* Created by Josh Jeppson on 8/17/2021
* */
#include "StaminaModelBuilder.h"
#include "StaminaMessages.h"
#include "StateSpaceInformation.h"

// Frequency for info/debug messages in terms of number of states explored.
#define MSG_FREQUENCY 100000
// #define MSG_FREQUENCY 4000

#include <functional>
#include <sstream>

#include "storm/builder/RewardModelBuilder.h"
#include "storm/builder/StateAndChoiceInformationBuilder.h"

#include "storm/generator/PrismNextStateGenerator.h"
#include "storm/generator/JaniNextStateGenerator.h"

#include "storm/models/sparse/Dtmc.h"
#include "storm/models/sparse/Ctmc.h"
#include "storm/models/sparse/Mdp.h"
#include "storm/models/sparse/MarkovAutomaton.h"
#include "storm/models/sparse/StandardRewardModel.h"

#include "storm/settings/modules/BuildSettings.h"

#include "storm/storage/expressions/ExpressionManager.h"
#include "storm/storage/jani/Model.h"
#include "storm/storage/jani/Automaton.h"
#include "storm/storage/jani/AutomatonComposition.h"
#include "storm/storage/jani/ParallelComposition.h"

#include "storm/utility/builder.h"
#include "storm/utility/constants.h"
#include "storm/utility/prism.h"
#include "storm/utility/macros.h"
#include "storm/utility/ConstantsComparator.h"
#include "storm/utility/SignalHandler.h"

#define RESERVE_VALUE 10000

using namespace stamina;

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
) : generator(generator)
	, stateStorage(*(new storm::storage::sparse::StateStorage<StateType>(generator->getStateSize())))
	, absorbingWasSetUp(false)
	, fresh(true)
	, firstIteration(true)
	, localKappa(Options::kappa)
	, numberTerminal(0)
{
	// Optimization for hashmaps
	exploredStates.max_load_factor(0.25);
	stateMap.max_load_factor(0.25);
	exploredStates.reserve(RESERVE_VALUE);
	stateMap.reserve(RESERVE_VALUE);
}

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions
) : StaminaModelBuilder( // Invoke other constructor
	std::make_shared<storm::generator::PrismNextStateGenerator<ValueType, StateType>>(program, generatorOptions)
)
{
	// Intentionally left empty
}


template <typename ValueType, typename RewardModelType, typename StateType>
std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::build() {
	try {
		switch (generator->getModelType()) {
			// Only supports CTMC models.
			case storm::generator::ModelType::CTMC:
				isCtmc = true;
				return storm::utility::builder::buildModelFromComponents(storm::models::ModelType::Ctmc, buildModelComponents());
			case storm::generator::ModelType::DTMC:
				isCtmc = false;
				StaminaMessages::warning("This model is a DTMC. If you are using this in the STAMINA program, currently, only CTMCs are supported. You may get an error in checking.");
				return storm::utility::builder::buildModelFromComponents(storm::models::ModelType::Dtmc, buildModelComponents());
			case storm::generator::ModelType::MDP:
			case storm::generator::ModelType::POMDP:
			case storm::generator::ModelType::MA:
			default:
				StaminaMessages::error("This model type is not supported!");
		}
		return nullptr;
	}
	catch(const std::exception& e) {
		std::stringstream ss;
		ss << "STAMINA encountered the following error (possibly in the interface with STORM)";
		ss << " in the function StaminaModelBuilder::build():\n\t" << e.what();
		StaminaMessages::error(ss.str());
	}

}

template <typename ValueType, typename RewardModelType, typename StateType>
std::vector<StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getPerimeterStates() {
	std::vector<StateType> perimeterStates;
	for (const auto & [ key, value ] : stateMap) {
		if (value->isTerminal()) {
			perimeterStates.push_back(value->index);
		}
	}

	return perimeterStates;
}

template <typename ValueType, typename RewardModelType, typename StateType>
StateType
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex(CompressedState const& state) {
	StateType actualIndex;
	StateType newIndex = static_cast<StateType>(stateStorage.getNumberOfStates());
	if (stateStorage.stateToId.contains(state)) {
		actualIndex = stateStorage.stateToId.getValue(state);
	}
	else {
		// Create new index just in case we need it
		actualIndex = newIndex;
	}

	auto nextState = stateMap.find(actualIndex);
	bool stateIsExisting = nextState != stateMap.end();

	stateStorage.stateToId.findOrAdd(state, actualIndex);
	// Handle conditional enqueuing
	if (isInit) {
		if (!stateIsExisting) {
			// Create a ProbabilityState for each individual state
			std::shared_ptr<ProbabilityState> initProbabilityState(new ProbabilityState(
				state
				, actualIndex
				, 1.0
				, true
			));
			numberTerminal++;
			stateMap.insert({actualIndex, initProbabilityState});
			statesToExplore.push(initProbabilityState);
			initProbabilityState->enqueued = true;
			return actualIndex;
		}
		std::shared_ptr<ProbabilityState> initProbabilityState = nextState->second;
		initProbabilityState->enqueued = true;
		stateMap.insert({actualIndex, initProbabilityState});
		statesToExplore.push(initProbabilityState);
		return actualIndex;
	}

	// This bit handles the non-initial states
	// The previous state has reachability of 0
	if (currentProbabilityState->getPi() == 0) {
		if (stateIsExisting) {
			// Don't rehash if we've already called find()
			std::shared_ptr<ProbabilityState> nextProbabilityState = nextState->second;
			auto emplaced = exploredStates.emplace(actualIndex);
			if (emplaced.second) {
				// Enqueue
				statesToExplore.push(nextProbabilityState);
				nextProbabilityState->enqueued = true;
			}
			else {
				nextProbabilityState->enqueued = false;
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
			std::shared_ptr<ProbabilityState> nextProbabilityState = nextState->second;
			auto emplaced = exploredStates.emplace(actualIndex);
			if (emplaced.second) {
				// Enqueue
				statesToExplore.push(nextProbabilityState);
				nextProbabilityState->enqueued = true;
			}
			else {
				nextProbabilityState->enqueued = false;
			}
		}
		else {
			// This state has not been seen so create a new ProbabilityState
			std::shared_ptr<ProbabilityState> nextProbabilityState(new ProbabilityState(state, actualIndex, 0.0, true));
			nextProbabilityState->enqueued = true;
			stateMap.insert({actualIndex, nextProbabilityState});
			exploredStates.emplace(actualIndex);
			statesToExplore.push(nextProbabilityState);
			numberTerminal++;
		}
	}
	return actualIndex;
}

template <typename ValueType, typename RewardModelType, typename StateType>
StateType
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getStateIndexOrAbsorbing(CompressedState const& state) {
	if (stateStorage.stateToId.contains(state)) {
		return stateStorage.stateToId.getValue(state);
	}
	// This state should not exist yet and should point to the absorbing state
	return 0;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::buildMatrices(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
	, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
	, StateAndChoiceInformationBuilder& stateAndChoiceInformationBuilder
	, boost::optional<storm::storage::BitVector>& markovianChoices
	, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
) {
	fresh = false;
	// Builds model
	// Initialize building state valuations (if necessary)
	if (stateAndChoiceInformationBuilder.isBuildStateValuations()) {
		stateAndChoiceInformationBuilder.stateValuationsBuilder() = generator->initializeStateValuationsBuilder();
	}

	// Create a callback for the next-state generator to enable it to request the index of states.
	std::function<StateType (CompressedState const&)> stateToIdCallback = std::bind(
		&StaminaModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex
		, this
		, std::placeholders::_1
	);

	// The perimeter states require a second custom stateToIdCallback which does not enqueue or
	// register new states
	std::function<StateType (CompressedState const&)> stateToIdCallback2 = std::bind(
		&StaminaModelBuilder<ValueType, RewardModelType, StateType>::getStateIndexOrAbsorbing
		, this
		, std::placeholders::_1
	);

	// Create absorbing state
	setUpAbsorbingState(
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
	for (StateType index : this->stateStorage.initialStateIndices) {
		exploredStates.insert(index);
	}

	currentRowGroup = 1;
	currentRow = 1;

	auto timeOfStart = std::chrono::high_resolution_clock::now();
	auto timeOfLastMessage = std::chrono::high_resolution_clock::now();
	uint64_t numberOfExploredStates = 0;
	uint64_t numberOfExploredStatesSinceLastMessage = 0;

	StateType currentIndex;
	CompressedState currentState;

	isInit = false;
	// Perform a search through the model.
	while (!statesToExplore.empty()) {
		currentProbabilityState = statesToExplore.pop();

		// Get the first state in the queue.
		currentIndex = currentProbabilityState->index;
		currentState = currentProbabilityState->state;
		if (currentIndex == 0) {
			StaminaMessages::errorAndExit("Dequeued artificial absorbing state!");
		}

		// Set our state variable in the class

		if (currentIndex % MSG_FREQUENCY == 0) {
			StaminaMessages::info("Exploring state with id " + std::to_string(currentIndex) + ".");
		}

		if (stateAndChoiceInformationBuilder.isBuildStateValuations()) {
			generator->addStateValuation(currentIndex, stateAndChoiceInformationBuilder.stateValuationsBuilder());
		}
		// Add the state rewards to the corresponding reward models.
		// Do not explore if state is terminal and its reachability probability is less than kappa
		if (currentProbabilityState->isTerminal() && currentProbabilityState->getPi() < localKappa) {
			connectTerminalStatesToAbsorbing(
				transitionMatrixBuilder
				, currentState
				, currentIndex
				, stateToIdCallback2
			);
			++numberOfExploredStates;
			++currentRow;
			++currentRowGroup;
			continue;
		}
		// Load state for us to use
		generator->load(currentState);

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
			StaminaMessages::errorAndExit("Behavior for state " + std::to_string(currentIndex) + " was empty!");
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

				auto nextProbabilityStatePair = stateMap.find(sPrime);
				if (nextProbabilityStatePair != stateMap.end()) {
					auto nextProbabilityState = nextProbabilityStatePair->second;
					if (!shouldEnqueueAll) {
						nextProbabilityState->addToPi(currentProbabilityState->getPi() * probability);
					}

					if (nextProbabilityState->enqueued && sPrime != 0) {
						// row, column, value
						transitionMatrixBuilder.addNextValue(currentIndex, sPrime, stateProbabilityPair.second);
						nextProbabilityState->enqueued = false;
					}
				}
			}

			++currentRow;
			firstChoiceOfState = false;
		}
		if (currentProbabilityState->isTerminal() && numberTerminal > 0) {
			numberTerminal--;
		}
		currentProbabilityState->setTerminal(false);
		currentProbabilityState->setPi(0.0);

		++currentRowGroup;

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
	StaminaMessages::info("Finished state space truncation. Explored " + std::to_string(numberOfExploredStates) + " states in total.");
}

template <typename ValueType, typename RewardModelType, typename StateType>
storm::storage::sparse::ModelComponents<ValueType, RewardModelType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::buildModelComponents() {
	// Is this model deterministic? (I.e., is there only one choice per state?)
	bool deterministic = generator->isDeterministicModel();

	// Component builders
	storm::storage::SparseMatrixBuilder<ValueType> transitionMatrixBuilder(0, 0, 0, false, !deterministic, 0);
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
	StateSpaceInformation::printVariableNames();
	// Builds matrices and truncates state space
	buildMatrices(
		transitionMatrixBuilder
		, rewardModelBuilders
		, stateAndChoiceInformationBuilder
		, markovianStates
		, stateValuationsBuilder
	);

	// Using the information from buildMatrices, initialize the model components
	storm::storage::sparse::ModelComponents<ValueType, RewardModelType> modelComponents(
		transitionMatrixBuilder.build(0, transitionMatrixBuilder.getCurrentRowGroupCount())
		, buildStateLabeling()
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
storm::models::sparse::StateLabeling
StaminaModelBuilder<ValueType, RewardModelType, StateType>::buildStateLabeling() {
	return generator->label(stateStorage, stateStorage.initialStateIndices, stateStorage.deadlockStateIndices);
}

template <typename ValueType, typename RewardModelType, typename StateType>
double
StaminaModelBuilder<ValueType, RewardModelType, StateType>::accumulateProbabilities() {
	double totalProbability = numberTerminal * localKappa;
	// Reduce kappa
	localKappa /= Options::reduce_kappa;
	return totalProbability;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::setUpAbsorbingState(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
	, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
	, StateAndChoiceInformationBuilder& choiceInformationBuilder
	, boost::optional<storm::storage::BitVector>& markovianChoices
	, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
) {
	if (absorbingWasSetUp) {
		return;
	}
	this->absorbingState = CompressedState(generator->getVariableInformation().getTotalBitOffset(true)); // CompressedState(64);
	bool gotVar = false;
	for (auto variable : generator->getVariableInformation().integerVariables) {
		if (variable.getName() == "Absorbing") {
			this->absorbingState.setFromInt(variable.bitOffset + 1, variable.bitWidth, 1);
			if (this->absorbingState.getAsInt(variable.bitOffset + 1, variable.bitWidth) != 1) {
				StaminaMessages::errorAndExit("Absorbing state setup failed!");
			}
			gotVar = true;
			break;
		}
	}
	if (!gotVar) {
		StaminaMessages::errorAndExit("Did not get \"Absorbing\" variable!");
	}
	// Add index 0 to deadlockstateindecies because the absorbing state is in deadlock
	stateStorage.deadlockStateIndices.push_back(0);
	// Check if state is already registered
	std::pair<StateType, std::size_t> actualIndexPair = stateStorage.stateToId.findOrAddAndGetBucket(absorbingState, 0);

	StateType actualIndex = actualIndexPair.first;
	if (actualIndex != 0) {
		StaminaMessages::errorAndExit("Absorbing state should be index 0! Got " + std::to_string(actualIndex));
	}
	absorbingWasSetUp = true;
	// transitionMatrixBuilder.addNextValue(0, 0, storm::utility::one<ValueType>());
	// This state shall be Markovian (to not introduce Zeno behavior)
	if (choiceInformationBuilder.isBuildMarkovianStates()) {
		choiceInformationBuilder.addMarkovianState(0);
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
stamina::StaminaModelBuilder<ValueType, RewardModelType, StateType>::reset() {
	if (fresh) {
		return;
	}
	statesToExplore = StatePriorityQueue();
	exploredStates.clear(); // States explored in our current iteration
	// API reset
	if (stateRemapping) { stateRemapping->clear(); }
	// stateStorage = storm::storage::sparse::StateStorage<StateType>(generator->getStateSize());
	absorbingWasSetUp = false;
}


template <typename ValueType, typename RewardModelType, typename StateType>
void
stamina::StaminaModelBuilder<ValueType, RewardModelType, StateType>::setGenerator(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> generator
) {
	this->generator = generator;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
stamina::StaminaModelBuilder<ValueType, RewardModelType, StateType>::setLocalKappaToGlobal() {
	Options::kappa = localKappa;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::connectTerminalStatesToAbsorbing(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
	, CompressedState terminalState
	, StateType stateId
	, std::function<StateType (CompressedState const&)> stateToIdCallback
) {
	bool addedValue = false;
	generator->load(terminalState);
	storm::generator::StateBehavior<ValueType, StateType> behavior = generator->expand(stateToIdCallback);
	// If there is no behavior, we have an error.
	if (behavior.empty()) {
		StaminaMessages::errorAndExit("Behavior for perimeter state was empty!");
	}
	for (auto const& choice : behavior) {
		double totalRateToAbsorbing = 0;
		for (auto const& stateProbabilityPair : choice) {
			if (stateProbabilityPair.first != 0) {
				// row, column, value
				transitionMatrixBuilder.addNextValue(stateId, stateProbabilityPair.first, stateProbabilityPair.second);
			}
			else {
				totalRateToAbsorbing += stateProbabilityPair.second;
			}
		}
		addedValue = true;
		// Absorbing state
		transitionMatrixBuilder.addNextValue(stateId, 0, totalRateToAbsorbing);
	}
	if (!addedValue) {
		StaminaMessages::errorAndExit("Did not add to transition matrix!");
	}
}
namespace stamina {
// Explicitly instantiate the class.
	template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

template <typename StateType>
bool set_contains(std::unordered_set<StateType> current_set, StateType value) {
	auto search = current_set.find(value);
	return (search != current_set.end());
}

} // namespace stamina
