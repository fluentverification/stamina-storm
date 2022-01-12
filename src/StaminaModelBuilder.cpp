/**
* Implementation for StaminaModelBuilder methods.
*
* Created by Josh Jeppson on 8/17/2021
* */
#include "StaminaModelBuilder.h"

#define DEBUG_PRINTS


#include <functional>
#include <sstream>

#include "storm/builder/RewardModelBuilder.h"
#include "storm/builder/StateAndChoiceInformationBuilder.h"

#include "storm/exceptions/AbortException.h"
#include "storm/exceptions/WrongFormatException.h"
#include "storm/exceptions/IllegalArgumentException.h"

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

using namespace stamina;

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
	Options * options
	, std::function<void(std::string)> err
	, std::function<void(std::string)> warn
	, std::function<void(std::string)> info
	, std::function<void(std::string)> good
	, std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> const& generator
) : generator(generator)
	, stateStorage(generator->getStateSize())
	, options(options)
	, err(err)
	, warn(warn)
	, info(info)
	, good(good)
{
	// Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
	Options * options
	, std::function<void(std::string)> err
	, std::function<void(std::string)> warn
	, std::function<void(std::string)> info
	, std::function<void(std::string)> good
	, storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions
) : StaminaModelBuilder( // Invoke other constructor
	options
	, err
	, warn
	, info
	, good
	, std::make_shared<storm::generator::PrismNextStateGenerator<ValueType, StateType>>(program, generatorOptions)
)
{
	// Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
	Options * options
	, std::function<void(std::string)> err
	, std::function<void(std::string)> warn
	, std::function<void(std::string)> info
	, std::function<void(std::string)> good
	, storm::jani::Model const& model
	, storm::generator::NextStateGeneratorOptions const& generatorOptions
) : StaminaModelBuilder( // Invoke other constructor
	options
	, err
	, warn
	, info
	, good
	, std::make_shared<storm::generator::JaniNextStateGenerator<ValueType, StateType>>(model, generatorOptions)
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
				return storm::utility::builder::buildModelFromComponents(storm::models::ModelType::Ctmc, buildModelComponents());
			case storm::generator::ModelType::DTMC:
			case storm::generator::ModelType::MDP:
			case storm::generator::ModelType::POMDP:
			case storm::generator::ModelType::MA:
			default:
				err("This model type is not supported!");
		}
		return nullptr;
	}
	catch(const std::exception& e) {
		std::stringstream ss;
		ss << "STAMINA encountered the following error (possibly in the interface with STORM)";
		ss << " in the function StaminaModelBuilder::build():\n\t" << e.what();
		err(ss.str());
	}

}

template <typename ValueType, typename RewardModelType, typename StateType>
StateType
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex(CompressedState const& state) {
	// Create new index just in case we need it
	StateType newIndex = static_cast<StateType>(stateStorage.getNumberOfStates());

	// Check if state is already registered
	std::pair<StateType, std::size_t> actualIndexPair = stateStorage.stateToId.findOrAddAndGetBucket(state, newIndex);

	StateType actualIndex = actualIndexPair.first;

	// Keeping this until I implement what's below so I know how to emplace a state.
// 	Determines if we need to insert the state
// 	if (actualIndex == newIndex) {
// 		Always does breadth first search
// 		statesToExplore.emplace_back(state, actualIndex);
// 	}

	/* PSEUDOCODE FOR WHAT I PLAN TO DO */
	// Check if the state exists within the unordered_map of reachability probabilities
	// If it does not
		// Add it with reachability probability 0.0
		// Show ERROR that unexpected behavior has been encountered (we've reached a state we shouldn't have been able to)

	// If state is not in T or its reachability probability is greater than kappa
		// Load state into generator
		// Expand generator into next states
		// If its reachability probability is 0
			// Enqueue all of its successors (for (auto choice : behavior) for (auto stateProbabilityPair : choice) stuff...)
		// Else it has a nonzero reachability probability
			// Remove state from T if it's in T
			// For each next state called state_prime (assume that we only have one behavior as this model is/must be deterministic)
				// Add the transition probability of going from state -> state_prime to state_prime's reachability probability
				// If NOT (state_prime in S (state set) and state_prime has been explored)
					// Add state_prime to the explored states
					// Enqueue it to the statesToExplore
					// If s_prime is not in the S set
						// Add it to T and S
			// Set the reachability probablity of state to 0
	// Set the reachability probability of S to the sum of the reachability probabilities in T TODO: should this be a level outer (reference VMCAI paper)

	return actualIndex;
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

	// Let the generator create all initial states.
	this->stateStorage.initialStateIndices = generator->getInitialStates(stateToIdCallback);
	if (!this->stateStorage.initialStateIndices.empty()) {
		err("Initial states are empty!");
	}

	// Now explore the current state until there is no more reachable state.
	uint_fast64_t currentRowGroup = 0;
	uint_fast64_t currentRow = 0;

	auto timeOfStart = std::chrono::high_resolution_clock::now();
	auto timeOfLastMessage = std::chrono::high_resolution_clock::now();
	uint64_t numberOfExploredStates = 0;
	uint64_t numberOfExploredStatesSinceLastMessage = 0;

	StateType currentIndex;

	// Perform a search through the model.
	while (!statesToExplore.empty()) {
		// Get the first state in the queue.
		CompressedState currentState = statesToExplore.front().first;
		currentIndex = statesToExplore.front().second;
		statesToExplore.pop_front();

		if (currentIndex % 100000 == 0) {
			info("Exploring state with id " + std::to_string(currentIndex) + ".");
		}

		generator->load(currentState);
		if (stateAndChoiceInformationBuilder.isBuildStateValuations()) {
			generator->addStateValuation(currentIndex, stateAndChoiceInformationBuilder.stateValuationsBuilder());
		}
		storm::generator::StateBehavior<ValueType, StateType> behavior = generator->expand(stateToIdCallback);

		// If there is no behavior, we might have to introduce a self-loop.
		if (behavior.empty()) {
			if (!storm::settings::getModule<storm::settings::modules::BuildSettings>().isDontFixDeadlocksSet() || !behavior.wasExpanded()) {
				// If the behavior was actually expanded and yet there are no transitions, then we have a deadlock state.
				if (behavior.wasExpanded()) {
					this->stateStorage.deadlockStateIndices.push_back(currentIndex);
				}

				if (!generator->isDeterministicModel()) {
					transitionMatrixBuilder.newRowGroup(currentRow);
				}

				transitionMatrixBuilder.addNextValue(currentRow, currentIndex, storm::utility::one<ValueType>());

				for (auto& rewardModelBuilder : rewardModelBuilders) {
					if (rewardModelBuilder.hasStateRewards()) {
						rewardModelBuilder.addStateReward(storm::utility::zero<ValueType>());
					}
				}

				// This state shall be Markovian (to not introduce Zeno behavior)
				if (stateAndChoiceInformationBuilder.isBuildMarkovianStates()) {
					stateAndChoiceInformationBuilder.addMarkovianState(currentRowGroup);
				}
				// Other state-based information does not need to be treated, in particular:
				// * StateValuations have already been set above
				// * The associated player shall be the "default" player, i.e. INVALID_PLAYER_INDEX

				++currentRow;
				++currentRowGroup;
			}
			else {
				err(
					"Error while creating sparse matrix from probabilistic program: found deadlock state ("
					+ generator->stateToString(currentState)
					+ "). For fixing these, please provide the appropriate option."
				);
			}
		}
		else {
			// Add the state rewards to the corresponding reward models.
			auto stateRewardIt = behavior.getStateRewards().begin();
			for (auto& rewardModelBuilder : rewardModelBuilders) {
				if (rewardModelBuilder.hasStateRewards()) {
					rewardModelBuilder.addStateReward(*stateRewardIt);
				}
				++stateRewardIt;
			}

			// Now add all choices.
			bool firstChoiceOfState = true;
			for (auto const& choice : behavior) {

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

				// Add the probabilistic behavior to the matrix.
				for (auto const& stateProbabilityPair : choice) {
					transitionMatrixBuilder.addNextValue(currentRow, stateProbabilityPair.first, stateProbabilityPair.second);
				}

				++currentRow;
				firstChoiceOfState = false;
			}

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
				std::cout << "Explored " << numberOfExploredStates << " states in " << durationSinceStart << " seconds (currently " << statesPerSecond << " states per second)." << std::endl;
				timeOfLastMessage = std::chrono::high_resolution_clock::now();
				numberOfExploredStatesSinceLastMessage = 0;
			}
		}

		if (storm::utility::resources::isTerminate()) {
			auto durationSinceStart = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - timeOfStart).count();
			std::cout << "Explored " << numberOfExploredStates << " states in " << durationSinceStart << " seconds before abort." << std::endl;
			break;
		}
	}


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
	modelComponents.choiceLabeling = stateAndChoiceInformationBuilder.buildChoiceLabeling(modelComponents.transitionMatrix.getRowCount());
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
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::setReachabilityThreshold(double threshold) {
	reachabilityThreshold = threshold;
}

template <typename ValueType, typename RewardModelType, typename StateType>
ProbState
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getOrAddProbStateToGlobalSet(StateType nextState) {
	// Create new probstate or find existing
	auto sPrime = stateMap.find(nextState);
	if (sPrime == stateMap.end()) {
		// Add our Probstate
		ProbState newState(nextState);
		stateMap.insert(newState);
		sPrime = stateMap.find(nextState);
		// Sanity check
		if (sPrime == stateMap.end()) {
			err("Something happened in stateMap.insert()");
		}
	}
	return *sPrime;
}

template <typename ValueType, typename RewardModelType, typename StateType>
bool
StaminaModelBuilder<ValueType, RewardModelType, StateType>::isInTMap(StateType s) {
	ProbState p((uint32_t) s);
	return set_contains(tMap, p);
}

// Explicitly instantiate the class.
template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

// STAMINA DOES NOT USE ANY OF THESE FORWARD DEFINITIONS
// #ifdef STORM_HAVE_CARL
// template class StaminaModelBuilder<storm::RationalNumber, storm::models::sparse::StandardRewardModel<storm::RationalNumber>, uint32_t>;
// template class StaminaModelBuilder<storm::RationalFunction, storm::models::sparse::StandardRewardModel<storm::RationalFunction>, uint32_t>;
// template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<storm::Interval>, uint32_t>;
// #endif


bool stamina::set_contains(std::unordered_set<ProbState> current_set, ProbState value) {
	auto search = current_set.find(value);
	return (search != current_set.end());
}
