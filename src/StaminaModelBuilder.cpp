/**
 * Implementation for StaminaModelBuilder methods.
 * 
 * Created by Josh Jeppson on 8/17/2021
 * */
#include "StaminaModelBuilder.h"

#include <functional>
#include <sstream>

#include "storm/builder/RewardModelBuilder.h"
#include "storm/builder/ChoiceInformationBuilder.h"

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

    // Determines if we need to insert the state
    if (actualIndex == newIndex) {
        // Always does breadth first search
        statesToExplore.emplace_back(state, actualIndex);
    }

    return actualIndex;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void 
StaminaModelBuilder<ValueType, RewardModelType, StateType>::buildMatrices(
    storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
    , std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
    , ChoiceInformationBuilder& choiceInformationBuilder
    , boost::optional<storm::storage::BitVector>& markovianChoices
    , boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
) {
    doReachabilityAnalysis();

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
    storm::builder::ChoiceInformationBuilder choiceInformationBuilder;
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
        , choiceInformationBuilder
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

    // Finalize the reward models
    for (RewardModelBuilder<typename RewardModelType::ValueType> & rewardModelBuilder : rewardModelBuilders) {
        modelComponents.rewardModels.emplace(
            rewardModelBuilder.getName()
            , rewardModelBuilder.build(
                modelComponents.transitionMatrix.getRowCount()
                , modelComponents.transitionMatrix.getColumnCount()
                , modelComponents.transitionMatrix.getRowGroupCount()
            )
        );
    }

    // Build choice labeling
    modelComponents.choiceLabeling = choiceInformationBuilder.buildChoiceLabeling(modelComponents.transitionMatrix.getRowCount());
    if (generator->getOptions().isBuildChoiceOriginsSet()) {
        auto originData = choiceInformationBuilder.buildDataOfChoiceOrigins(modelComponents.transitionMatrix.getRowCount());
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

/**
 * This method needs to be rewritten without the use of ProbState
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::doReachabilityAnalysis() {
    auto startTime = std::chrono::high_resolution_clock::now();
    // TODO: set up

    // Create explored states queue and map of all states
    std::deque<ProbState> exploredStates;
    std::unordered_set<ProbState> statesK;

    // Create a callback to our getOrAddStateIndex so that our PrismNextStateGenerator can access it
    std::function<StateType (CompressedState const&)> stateToIdCallback = std::bind(
        &StaminaModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex
        , this
        , std::placeholders::_1
    );

    // Let the PrismNextStateGenerator get the initial states
    stateStorage.initialStateIndices = generator->getInitialStates(stateToIdCallback);
    // If no initial states, we can't continue
    if (stateStorage.initialStateIndices.empty()) {
        err("The initial states for this model are undefined!");
        std::exit(1);
    }
    // Add each state in our initial states to our states to explore
    for (auto & state : stateStorage.initialStateIndices) {
        // getOrAddStateIndex(state);
    }

    double perimReachability = 1.0;
    double targetPerimReachability = options->prob_win / options->approx_factor;
    // State search
    while (perimReachability >= targetPerimReachability) {
        // Breadth first search
        while (!exploredStates.empty()) {
            ProbState currentProbState = exploredStates.front();
            exploredStates.pop_front();
            CompressedState currentState = currentProbState.state;
            // Load state for next state generator
            generator->load(currentState);
            double currentStateReachability = currentProbState.getCurReachabilityProb();
            // TODO: evaluate property expression and if so set state terminal and absorbing and continue
            // Determine if we need to explore state and if so, explore it.
            // Only explore if the state is not terminal or if the reachability is greater than the threshold
            if (!currentProbState.isStateTerminal() || currentStateReachability >= reachabilityThreshold) {
                storm::generator::StateBehavior<ValueType, StateType> behavior = generator->expand(stateToIdCallback);
                // If state has a reachability of 0, don't perform calculation
                if (currentStateReachability == 0.0) {
                    // If no behavior, can't do much of anything
                    if (behavior.empty()) {
                        warn("State behavior is empty!");
                    }
                    else {
                        // Iterate through each choice
                        for (auto const & choice : behavior) {
                            for (auto const & stateProbabilityPair : choice) {
                                // See if our next ProbState is already in our stateMap
                                auto nextStateFound = stateMap.find(stateProbabilityPair.first);
                                // If it exists
                                if (nextStateFound != stateMap.end()) {
                                    ProbState nextProbState = nextStateFound->second;
                                    // Add state and explore if new
                                    if (statesK.emplace(nextProbState).second) {
                                        exploredStates.push_back(nextProbState);
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    auto totalNumberOfChoices = behavior.getNumberOfChoices();
                    // Get our total exit rate
                    ValueType totalExitRate;
                    if (generator->isDiscreteTimeModel()) {
                        totalExitRate = static_cast<ValueType>(totalNumberOfChoices);
                    }
                    else {
                        totalExitRate = storm::utility::zero<ValueType>();
                    }
                    // Accumulate total mass of choices
                    for (auto const & choice : behavior) {
                        totalExitRate += choice.getTotalMass();
                    }
                    // Iterate through our choices
                    for (auto const & choice : behavior) {
                        for (auto const & stateProbabilityPair : choice) {
                            // Get next ProbState id and exit rate
                            StateType nextState = stateProbabilityPair.first;
                            ValueType tranRate = stateProbabilityPair.second;
                            ValueType tranProb = tranRate / totalExitRate;
                            ValueType leavingProb = tranProb * currentStateReachability;

                            // Check if state exists
                            auto nextStateFound = stateMap.find(nextState);
                            // If it doesn't exist
                            if(nextStateFound != stateMap.end()) {
                                ProbState* nextProbState = &(nextStateFound->second);
                                nextProbState->addToReachability(leavingProb);

                                // Add state to states and explored if new state
                                if(statesK.emplace(*nextProbState).second) {
                                    exploredStates.push_back(*nextProbState);
                                }
                            }
                            else {
                                // Create state and add to state map
                                ProbState nextProbState(nextState);                                    
                                nextProbState.addToReachability(leavingProb);
                                stateMap.emplace(nextState, nextProbState);
                                statesK.emplace(nextProbState);
                                exploredStates.push_back(nextProbState);
                            }
                        }
                    }
                }
                currentProbState.setCurReachabilityProb(0);
                currentProbState.setStateTerminal(false);
            }
            // Show progress
            info("Progress: " + std::to_string(stateMap.size()));
        }
        exploredStates.clear();
        statesK.clear();
        // Add initial states back to our explore queue
        for (auto & state : stateStorage.initialStateIndices) {
            // getOrAddStateIndex(state);
        }
        // Recalculate perimeter reachability
        perimReachability = 0.0;
        // Iterate over our state map
        for (auto & localState : stateMap) {
            if (localState.second.isStateTerminal()) {
                perimReachability += reachabilityThreshold;
            }
        }
        // Reduce kappa again
        reachabilityThreshold /= options->reduce_kappa;
    }
    options->kappa = reachabilityThreshold;
    // Tell us how much time has elapsed
    auto endTime = std::chrono::high_resolution_clock::now();
    auto timeDiff = endTime - startTime;
    std::stringstream ss;
    ss << "Finished exploration of state space and state truncation (with permReachability ";
    ss << perimReachability << ") in " << timeDiff.count() << " seconds,\n";
    ss << "\tExplored " << stateMap.size() << " states.";
    good(ss.str());
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::setReachabilityThreshold(double threshold) {
    reachabilityThreshold = threshold;
}


// Explicitly instantiate the class.
template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

// STAMINA DOES NOT USE ANY OF THESE FORWARD DEFINITIONS
// #ifdef STORM_HAVE_CARL
// template class StaminaModelBuilder<storm::RationalNumber, storm::models::sparse::StandardRewardModel<storm::RationalNumber>, uint32_t>;
// template class StaminaModelBuilder<storm::RationalFunction, storm::models::sparse::StandardRewardModel<storm::RationalFunction>, uint32_t>;
// template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<storm::Interval>, uint32_t>;
// #endif