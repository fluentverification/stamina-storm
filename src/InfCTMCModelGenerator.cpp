#include "InfCTMCModelGenerator.h"
#include "StaminaOptions.h"
#include <map>


#include "storm/builder/RewardModelBuilder.h"
#include "storm/builder/ChoiceInformationBuilder.h"

#include "storm/exceptions/AbortException.h"
#include "storm/exceptions/WrongFormatException.h"

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


/* 
    If function/method definition names are going to have names that are long enough to 
    reach halfway across the planet, we should at *least* put a line break in there
    somewhere, right?

    - Josh

    Here's what I was thinking style-wise for such long templated names:

    returnType<ValueType, RewardModelType, StateType> InfCTMCModelGenerator::functionName(
            ParameterOneType<ValueType, RewardModelType, StateType> param1
            , ParameterTwoType<ValueType, RewardModelType, StateType> param2
            , ParameterThreeType<ValueType, RewardModelType, StateType> param3
        )
        : superClassThing(), superClassThing2() {
        // Actual function code.
    }

    If someone doesn't like this, let me know and I'll change it back. If you're cool with
    it, then delete this message or something I guess.
*/

/**
 * Constructor for InfCTMCModelGenerator::Options()
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::Options::Options() 
    : explorationOrder(storm::settings::getModule<storm::settings::modules::BuildSettings>().getExplorationOrder()) {
    // Intentionally left empty.
}
/**
 * Constructor for InfCTMCModelGenerator().
 * 
 * @param generator std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> Smart pointer to the STORM generator
 * @param options Options& Options for the generator.
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::InfCTMCModelGenerator(
        std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> const& generator
        , Options const& options
    ) 
    : generator(generator), options(options), stateStorage(generator->getStateSize()) {
    // Intentionally left empty.
}
/**
 * Constructor for InfCTMCModelGenerator().
 * 
 * @param generatorOptions storm::prism::Program const& program, storm::generator::NextStateGeneratorOptions&
 * @param options Options& Options for the generator.
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::InfCTMCModelGenerator(
        storm::prism::Program const& program, storm::generator::NextStateGeneratorOptions const& generatorOptions
        , Options const& builderOptions
    ) 
    : InfCTMCModelGenerator(std::make_shared<storm::generator::PrismNextStateGenerator<ValueType, StateType>>(program, generatorOptions), builderOptions) {
    // Intentionally left empty.
}
/**
 * Constructor for InfCTMCModelGenerator().
 * 
 * @param model storm::jani::Model& The model to start with.
 * @param generatorOptions storm::generator::NextStateGeneratorOptions& The options for the model generator.
 * @param builderOptions Options& The options for the builder.
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::InfCTMCModelGenerator(
        storm::jani::Model const& model
        , storm::generator::NextStateGeneratorOptions const& generatorOptions
        , Options const& builderOptions
    ) 
    : InfCTMCModelGenerator(std::make_shared<storm::generator::JaniNextStateGenerator<ValueType, StateType>>(model, generatorOptions), builderOptions) {
    // Intentionally left empty.
}
/** 
 * Builds STORM model components based on the model type of the generator.
 * 
 * details This function takes a parameter of type storm::generator::VariableInformation, which it then stores into this particular instance of InfCTMCModelGenerator.
 * After it has finished this, it takes a look at the model type of this->generator, and builds a model from that. If the model type is not one of the following:
 * DTMC, CTMC, MDP, POMDP, or MA, it throws a WrongFormatException and returns nullptr.
 * 
 * @param variableInformation storm::generator::VariableInformation& The variable information to store for the STORM model.
 * 
 * @return Pointer to the model generated from components.
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::build(storm::generator::VariableInformation const& variableInformation) {
    STORM_LOG_DEBUG("Exploration order is: " << options.explorationOrder);
    this->variableInformation = variableInformation;
    switch (generator->getModelType()) {
        case storm::generator::ModelType::DTMC:
            return storm::utility::builder::buildModelFromComponents(storm::models::ModelType::Dtmc, buildModelComponents());
        case storm::generator::ModelType::CTMC:
            return storm::utility::builder::buildModelFromComponents(storm::models::ModelType::Ctmc, buildModelComponents());
        case storm::generator::ModelType::MDP:
            return storm::utility::builder::buildModelFromComponents(storm::models::ModelType::Mdp, buildModelComponents());
        case storm::generator::ModelType::POMDP:
            return storm::utility::builder::buildModelFromComponents(storm::models::ModelType::Pomdp, buildModelComponents());
        case storm::generator::ModelType::MA:
            return storm::utility::builder::buildModelFromComponents(storm::models::ModelType::MarkovAutomaton, buildModelComponents());
        default:
            STORM_LOG_THROW(false, storm::exceptions::WrongFormatException, "Error while creating model: cannot handle this model type.");
    }

    return nullptr;
}
/**
 * Gets the index of a state of type CompressedState& and returns it. 
 * 
 * details This function takes a state of type CompressedState and tries to register a new index
 * for that in our breadth-first traversal or depth first traversal. If the state is already registered 
 * with an index, this function will not add it to the states yet to explore. However, if it does not,
 * this method will add it to the states to explore in either depth first or breadth first, asserting
 * to the STORM log if the exploration order is neither one.
 * 
 * @param state CompressedState& The state to add (if not already added) into our states to explore.
 * 
 * @return The actual index of this state after it has been added.
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
StateType InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::getOrAddStateIndex(CompressedState const& state) {
    StateType newIndex = static_cast<StateType>(stateStorage.getNumberOfStates());

    // Check, if the state was already registered.
    std::pair<StateType, std::size_t> actualIndexBucketPair = stateStorage.stateToId.findOrAddAndGetBucket(state, newIndex);

    StateType actualIndex = actualIndexBucketPair.first;

    if (actualIndex == newIndex) {
        if (options.explorationOrder == storm::builder::ExplorationOrder::Dfs) {
            statesToExplore.emplace_front(state, actualIndex);

            // Reserve one slot for the new state in the remapping.
            stateRemapping.get().push_back(storm::utility::zero<StateType>());
        } 
        else if (options.explorationOrder == storm::builder::ExplorationOrder::Bfs) {
            statesToExplore.emplace_back(state, actualIndex);
            auto newProbState = new ProbState(actualIndex);
            stateMap.emplace(actualIndex, newProbState);
        } 
        else {
            STORM_LOG_ASSERT(false, "Invalid exploration order.");
        }
    }

    return actualIndex;
}
/**
 * Gets the index of traversal for an absorbing state.
 * 
 * @param state CompressedState& the state to get the index of.
 * 
 * @return Always returns 0, since state is absorbing.
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
StateType InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::getAbsorbingStateIndex(CompressedState const& state) {
    return 0;
}
/**
 * brief buildMatrices - Builds the transition matrices of the truncated state space.
 * 
 * details This function performs the traversal of the state space and truncates it at the majority of the probability mass.
 * I believe this version currently uses the STAMINA 1.0 algorithm, so we need to update it to STAMINA 2.0 (the one that Riley
 * came up with). TODO: update to STAMINA 2.0 algorithm.
 * 
 * @param transitionMatrixBuilder SparseMatrixBuilder&
 * @param RewardModelBuilders RewardModelBuilder&
 * @param ChoiceInformationBuilder ChoiceInformationBuilder&
 * @param markovianStates optional&
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
void InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::buildMatrices(
    storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
    , std::vector<storm::builder::RewardModelBuilder<typename RewardModelType::ValueType>>& RewardModelBuilders
    , storm::builder::ChoiceInformationBuilder& ChoiceInformationBuilder, boost::optional<storm::storage::BitVector>& markovianStates) {

    // Create markovian states bit vector, if required.
    if (generator->getModelType() == storm::generator::ModelType::MA) {
        // The bit vector will be resized when the correct size is known.
        markovianStates = storm::storage::BitVector(1000);
    }

    // Create a callback for the next-state generator to enable it to request the index of states.
    std::function<StateType (CompressedState const&)> stateToIdCallback = std::bind(&InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::getOrAddStateIndex, this, std::placeholders::_1);
    std::function<StateType (CompressedState const&)> absorbingStateIndexFunc = std::bind(&InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::getAbsorbingStateIndex, this, std::placeholders::_1);
    // If the exploration order is something different from breadth-first, we need to keep track of the remapping
    // from state ids to row groups. For this, we actually store the reversed mapping of row groups to state-ids
    // and later reverse it.
    if (options.explorationOrder != storm::builder::ExplorationOrder::Bfs) {
        stateRemapping = std::vector<uint_fast64_t>();
    }
    stateMap.reserve(16000);

    uint_fast64_t currentRowGroup = 0;
    uint_fast64_t currentRow = 0;



    //Make absorbing state
    CompressedState absorbingState(variableInformation.getTotalBitOffset(true));
    stateStorage.stateToId.findOrAddAndGetBucket(absorbingState, 0);
    /*auto boolIt = variableInformation.booleanVariables.begin();
    auto boolEnd = variableInformation.booleanVariables.end();
    while (boolIt != boolEnd) {
        absorbingState.set(boolIt->bitOffset, false);
        boolIt++;
    }
    auto integerIt = variableInformation.integerVariables.begin();
    auto integerEnd = variableInformation.integerVariables.end();
    while (integerIt != integerEnd) {
        absorbingState.set(integerIt->bitOffset, -1);
        integerIt++;
    }*/
    transitionMatrixBuilder.addNextValue(currentRow, 0, storm::utility::one<ValueType>());
    this->stateStorage.deadlockStateIndices.push_back(0);
    currentRow++;
    currentRowGroup++;

    // Let the generator create all initial states.
    this->stateStorage.initialStateIndices = generator->getInitialStates(stateToIdCallback);
    stateMap.find(this->stateStorage.initialStateIndices[0])->second->setCurReachabilityProb(1);
    STORM_LOG_THROW(!this->stateStorage.initialStateIndices.empty(), storm::exceptions::WrongFormatException, "The model does not have a single initial state.");

    // Now explore the current state until there is no more reachable state.


    auto timeOfStart = std::chrono::high_resolution_clock::now();
    auto timeOfLastMessage = std::chrono::high_resolution_clock::now();
    uint64_t numberOfExploredStates = 0;
    uint64_t numberOfExploredStatesSinceLastMessage = 0;

    // Perform a search through the model.
    while (!statesToExplore.empty()) {
        // Get the first state in the queue.
        CompressedState currentState = statesToExplore.front().first;
        StateType currentIndex = statesToExplore.front().second;
        statesToExplore.pop_front();

        // If the exploration order differs from breadth-first, we remember that this row group was actually
        // filled with the transitions of a different state.
        if (options.explorationOrder != storm::builder::ExplorationOrder::Bfs) {
            stateRemapping.get()[currentIndex] = currentRowGroup;
        }

        if (currentIndex % 100000 == 0) {
            STORM_LOG_TRACE("Exploring state with id " << currentIndex << ".");
        }
        currentStateReachability = stateMap.find(currentIndex)->second->getCurReachabilityProb();
        if (currentStateReachability >= StaminaOptions::getReachabilityThreshold()) {

            generator->load(currentState);

            storm::generator::StateBehavior<ValueType, StateType> behavior = generator->expand(
                    stateToIdCallback);


            // If there is no behavior, we might have to introduce a self-loop.
            if (behavior.empty()) {
                if (!storm::settings::getModule<storm::settings::modules::BuildSettings>().isDontFixDeadlocksSet() ||
                    !behavior.wasExpanded()) {
                    // If the behavior was actually expanded and yet there are no transitions, then we have a deadlock state.
                    if (behavior.wasExpanded()) {
                        this->stateStorage.deadlockStateIndices.push_back(currentIndex);
                    }

                    if (markovianStates) {
                        markovianStates.get().grow(currentRowGroup + 1, false);
                        markovianStates.get().set(currentRowGroup);
                    }

                    if (!generator->isDeterministicModel()) {
                        transitionMatrixBuilder.newRowGroup(currentRow);
                    }

                    transitionMatrixBuilder.addNextValue(currentRow, currentIndex,
                                                            storm::utility::one<ValueType>());

                    for (auto &RewardModelBuilder : RewardModelBuilders) {
                        if (RewardModelBuilder.hasStateRewards()) {
                            RewardModelBuilder.addStateReward(storm::utility::zero<ValueType>());
                        }

                        if (RewardModelBuilder.hasStateActionRewards()) {
                            RewardModelBuilder.addStateActionReward(storm::utility::zero<ValueType>());
                        }
                    }

                    ++currentRow;
                    ++currentRowGroup;
                } else {
                    STORM_LOG_THROW(false, storm::exceptions::WrongFormatException,
                                    "Error while creating sparse matrix from probabilistic program: found deadlock state ("
                                            << generator->toValuation(currentState).toString(true)
                                            << "). For fixing these, please provide the appropriate option.");
                }
            } else {
                // Add the state rewards to the corresponding reward models.
                auto stateRewardIt = behavior.getStateRewards().begin();
                for (auto &RewardModelBuilder : RewardModelBuilders) {
                    if (RewardModelBuilder.hasStateRewards()) {
                        RewardModelBuilder.addStateReward(*stateRewardIt);
                    }
                    ++stateRewardIt;
                }

                // If the model is nondeterministic, we need to open a row group.
                if (!generator->isDeterministicModel()) {
                    transitionMatrixBuilder.newRowGroup(currentRow);
                }

                // Now add all choices.
                for (auto const &choice : behavior) {

                    // add the generated choice information
                    if (choice.hasLabels()) {
                        for (auto const &label : choice.getLabels()) {
                            ChoiceInformationBuilder.addLabel(label, currentRow);
                        }
                    }
                    if (choice.hasOriginData()) {
                        ChoiceInformationBuilder.addOriginData(choice.getOriginData(), currentRow);
                    }

                    // If we keep track of the Markovian choices, store whether the current one is Markovian.
                    if (markovianStates && choice.isMarkovian()) {
                        markovianStates.get().grow(currentRowGroup + 1, false);
                        markovianStates.get().set(currentRowGroup);
                    }

                    // Add the probabilistic behavior to the matrix.
                    for (auto const &stateProbabilityPair : choice) {
                        transitionMatrixBuilder.addNextValue(currentRow, stateProbabilityPair.first,
                                                                stateProbabilityPair.second);
                        if(currentIndex == 544) {
                            auto x = 1;
                        }
                        auto currentProbState = stateMap.find(currentIndex)->second;
                        auto nextState =  stateMap.find(stateProbabilityPair.first)->second;


                        nextState->updatePredecessorProbMap(currentIndex, stateProbabilityPair.second);

                        double nextReachabilityProb = 0.0;
                        for(auto element : nextState->predecessorPropMap) {
                            nextReachabilityProb += stateMap.find(element.first)->second->getCurReachabilityProb() * element.second;
                        }
                        if (nextReachabilityProb > 1.0) {
                            //throw new stormException("Path Probability greater than 1.0");
                        }
                        nextState->setCurReachabilityProb(nextReachabilityProb);
                    }

                    // Add the rewards to the reward models.
                    auto choiceRewardIt = choice.getRewards().begin();
                    for (auto &RewardModelBuilder : RewardModelBuilders) {
                        if (RewardModelBuilder.hasStateActionRewards()) {
                            RewardModelBuilder.addStateActionReward(*choiceRewardIt);
                        }
                        ++choiceRewardIt;
                    }
                    ++currentRow;
                }
                ++currentRowGroup;
            }

            ++numberOfExploredStates;
            if (generator->getOptions().isShowProgressSet()) {
                ++numberOfExploredStatesSinceLastMessage;

                auto now = std::chrono::high_resolution_clock::now();
                auto durationSinceLastMessage = std::chrono::duration_cast<std::chrono::seconds>(
                        now - timeOfLastMessage).count();
                if (static_cast<uint64_t>(durationSinceLastMessage) >=
                    generator->getOptions().getShowProgressDelay()) {
                    auto statesPerSecond = numberOfExploredStatesSinceLastMessage / durationSinceLastMessage;
                    auto durationSinceStart = std::chrono::duration_cast<std::chrono::seconds>(
                            now - timeOfStart).count();
                    std::cout << "Explored " << numberOfExploredStates << " states in " << durationSinceStart
                                << " seconds (currently " << statesPerSecond << " states per second)."
                                << std::endl;
                    timeOfLastMessage = std::chrono::high_resolution_clock::now();
                    numberOfExploredStatesSinceLastMessage = 0;
                }
            }

            if (storm::utility::resources::isTerminate()) {
                auto durationSinceStart = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::high_resolution_clock::now() - timeOfStart).count();
                std::cout << "Explored " << numberOfExploredStates << " states in " << durationSinceStart
                            << " seconds before abort." << std::endl;
                STORM_LOG_THROW(false, storm::exceptions::AbortException,
                                "Aborted in state space exploration.");
                break;
            }
        }
        else {
            generator->load(currentState);
            storm::generator::StateBehavior<ValueType, StateType> behavior = generator->expand(absorbingStateIndexFunc);


            // If there is no behavior, we might have to introduce a self-loop.
            if (behavior.empty()) {
                if (!storm::settings::getModule<storm::settings::modules::BuildSettings>().isDontFixDeadlocksSet() ||
                    !behavior.wasExpanded()) {
                    // If the behavior was actually expanded and yet there are no transitions, then we have a deadlock state.
                    if (behavior.wasExpanded()) {
                        this->stateStorage.deadlockStateIndices.push_back(currentIndex);
                    }

                    if (markovianStates) {
                        markovianStates.get().grow(currentRowGroup + 1, false);
                        markovianStates.get().set(currentRowGroup);
                    }

                    if (!generator->isDeterministicModel()) {
                        transitionMatrixBuilder.newRowGroup(currentRow);
                    }

                    transitionMatrixBuilder.addNextValue(currentRow, currentIndex,
                                                            storm::utility::one<ValueType>());

                    for (auto &RewardModelBuilder : RewardModelBuilders) {
                        if (RewardModelBuilder.hasStateRewards()) {
                            RewardModelBuilder.addStateReward(storm::utility::zero<ValueType>());
                        }

                        if (RewardModelBuilder.hasStateActionRewards()) {
                            RewardModelBuilder.addStateActionReward(storm::utility::zero<ValueType>());
                        }
                    }

                    ++currentRow;
                    ++currentRowGroup;
                } else {
                    STORM_LOG_THROW(false, storm::exceptions::WrongFormatException,
                                    "Error while creating sparse matrix from probabilistic program: found deadlock state ("
                                            << generator->toValuation(currentState).toString(true)
                                            << "). For fixing these, please provide the appropriate option.");
                }
            } else {
                // Add the state rewards to the corresponding reward models.
                auto stateRewardIt = behavior.getStateRewards().begin();
                for (auto &RewardModelBuilder : RewardModelBuilders) {
                    if (RewardModelBuilder.hasStateRewards()) {
                        RewardModelBuilder.addStateReward(*stateRewardIt);
                    }
                    ++stateRewardIt;
                }

                // If the model is nondeterministic, we need to open a row group.
                if (!generator->isDeterministicModel()) {
                    transitionMatrixBuilder.newRowGroup(currentRow);
                }

                // Now add all choices.
                for (auto const &choice : behavior) {

                    // add the generated choice information
                    if (choice.hasLabels()) {
                        for (auto const &label : choice.getLabels()) {
                            ChoiceInformationBuilder.addLabel(label, currentRow);
                        }
                    }
                    if (choice.hasOriginData()) {
                        ChoiceInformationBuilder.addOriginData(choice.getOriginData(), currentRow);
                    }

                    // If we keep track of the Markovian choices, store whether the current one is Markovian.
                    if (markovianStates && choice.isMarkovian()) {
                        markovianStates.get().grow(currentRowGroup + 1, false);
                        markovianStates.get().set(currentRowGroup);
                    }

                    // Add the probabilistic behavior to the matrix.
                    for (auto const &stateProbabilityPair : choice) {
                        transitionMatrixBuilder.addNextValue(currentRow, stateProbabilityPair.first,
                                                                stateProbabilityPair.second);
                    }

                    // Add the rewards to the reward models.
                    auto choiceRewardIt = choice.getRewards().begin();
                    for (auto &RewardModelBuilder : RewardModelBuilders) {
                        if (RewardModelBuilder.hasStateActionRewards()) {
                            RewardModelBuilder.addStateActionReward(*choiceRewardIt);
                        }
                        ++choiceRewardIt;
                    }
                    ++currentRow;
                }
                ++currentRowGroup;
            }

            ++numberOfExploredStates;
            if (generator->getOptions().isShowProgressSet()) {
                ++numberOfExploredStatesSinceLastMessage;

                auto now = std::chrono::high_resolution_clock::now();
                auto durationSinceLastMessage = std::chrono::duration_cast<std::chrono::seconds>(
                        now - timeOfLastMessage).count();
                if (static_cast<uint64_t>(durationSinceLastMessage) >=
                    generator->getOptions().getShowProgressDelay()) {
                    auto statesPerSecond = numberOfExploredStatesSinceLastMessage / durationSinceLastMessage;
                    auto durationSinceStart = std::chrono::duration_cast<std::chrono::seconds>(
                            now - timeOfStart).count();
                    std::cout << "Explored " << numberOfExploredStates << " states in " << durationSinceStart
                                << " seconds (currently " << statesPerSecond << " states per second)."
                                << std::endl;
                    timeOfLastMessage = std::chrono::high_resolution_clock::now();
                    numberOfExploredStatesSinceLastMessage = 0;
                }
            }

            if (storm::utility::resources::isTerminate()) {
                auto durationSinceStart = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::high_resolution_clock::now() - timeOfStart).count();
                std::cout << "Explored " << numberOfExploredStates << " states in " << durationSinceStart
                            << " seconds before abort." << std::endl;
                STORM_LOG_THROW(false, storm::exceptions::AbortException,
                                "Aborted in state space exploration.");
                break;
            }
        }
    }

    if (markovianStates) {
        // Since we now know the correct size, cut the bit vector to the correct length.
        markovianStates->resize(currentRowGroup, false);
    }

    // If the exploration order was not breadth-first, we need to fix the entries in the matrix according to
    // (reversed) mapping of row groups to indices.
    if (options.explorationOrder != storm::builder::ExplorationOrder::Bfs) {
        STORM_LOG_ASSERT(stateRemapping, "Unable to fix columns without mapping.");
        std::vector<uint_fast64_t> const& remapping = stateRemapping.get();

        // We need to fix the following entities:
        // (a) the transition matrix
        // (b) the initial states
        // (c) the hash map storing the mapping states -> ids
        // (d) fix remapping for state-generation labels

        // Fix (a).
        transitionMatrixBuilder.replaceColumns(remapping, 0);

        // Fix (b).
        std::vector<StateType> newInitialStateIndices(this->stateStorage.initialStateIndices.size());
        std::transform(this->stateStorage.initialStateIndices.begin(), this->stateStorage.initialStateIndices.end(), newInitialStateIndices.begin(), [&remapping] (StateType const& state) { return remapping[state]; } );
        std::sort(newInitialStateIndices.begin(), newInitialStateIndices.end());
        this->stateStorage.initialStateIndices = std::move(newInitialStateIndices);

        // Fix (c).
        this->stateStorage.stateToId.remap([&remapping] (StateType const& state) { return remapping[state]; } );

        this->generator->remapStateIds([&remapping] (StateType const& state) { return remapping[state]; });
    }
}
/**
 * Builds the model components for this model.
 * 
 * details This function builds the ModelComponents for this model. In order to do this, it first calls buildMatrices(), which
 * are then placed into the ModelComponents class. After this is done we finalize the models and build labeling.
 * 
 * @return The model components we have built.
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
storm::storage::sparse::ModelComponents<ValueType, RewardModelType> InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::buildModelComponents() {

    // Determine whether we have to combine different choices to one or whether this model can have more than
    // one choice per state.
    bool deterministicModel = generator->isDeterministicModel();

    // Prepare the component builders
    storm::storage::SparseMatrixBuilder<ValueType> transitionMatrixBuilder(0, 0, 0, false, !deterministicModel, 0);
    std::vector<storm::builder::RewardModelBuilder<typename RewardModelType::ValueType>> RewardModelBuilders;
    for (uint64_t i = 0; i < generator->getNumberOfRewardModels(); ++i) {
        RewardModelBuilders.emplace_back(generator->getRewardModelInformation(i));
    }
    storm::builder::ChoiceInformationBuilder ChoiceInformationBuilder;
    boost::optional<storm::storage::BitVector> markovianStates;

    buildMatrices(transitionMatrixBuilder, RewardModelBuilders, ChoiceInformationBuilder, markovianStates);

    // Initialize the model components with the obtained information.
    storm::storage::sparse::ModelComponents<ValueType, RewardModelType> modelComponents(
        transitionMatrixBuilder.build(0, transitionMatrixBuilder.getCurrentRowGroupCount())
        , buildStateLabeling()
        , std::unordered_map<std::string, RewardModelType>()
        , !generator->isDiscreteTimeModel()
        , std::move(markovianStates)
    );

    // Now finalize all reward models.
    for (auto& RewardModelBuilder : RewardModelBuilders) {
        modelComponents.rewardModels.emplace(
            RewardModelBuilder.getName()
            , RewardModelBuilder.build(
                modelComponents.transitionMatrix.getRowCount()
                , modelComponents.transitionMatrix.getColumnCount()
                , modelComponents.transitionMatrix.getRowGroupCount()
            )
        );
    }
    // Build the choice labeling
    modelComponents.choiceLabeling = ChoiceInformationBuilder.buildChoiceLabeling(modelComponents.transitionMatrix.getRowCount());

    // If requested, build the state valuations and choice origins
    if (generator->getOptions().isBuildStateValuationsSet()) {
        std::vector<storm::expressions::SimpleValuation> valuations(modelComponents.transitionMatrix.getRowGroupCount());
        for (auto const& bitVectorIndexPair : stateStorage.stateToId) {
            valuations[bitVectorIndexPair.second] = generator->toValuation(bitVectorIndexPair.first);
        }
        modelComponents.stateValuations = storm::storage::sparse::StateValuations(std::move(valuations));
    }
    if (generator->getOptions().isBuildChoiceOriginsSet()) {
        auto originData = ChoiceInformationBuilder.buildDataOfChoiceOrigins(modelComponents.transitionMatrix.getRowCount());
        modelComponents.choiceOrigins = generator->generateChoiceOrigins(originData);
    }
    if (generator->isPartiallyObservable()) {
        std::vector<uint32_t> classes;
        uint32_t newObservation = 0;
        classes.resize(stateStorage.getNumberOfStates());
        std::unordered_map<uint32_t, std::vector<std::pair<std::vector<std::string>, uint32_t>>> observationActions;
        for (auto const& bitVectorIndexPair : stateStorage.stateToId) {
            uint32_t varObservation = generator->observabilityClass(bitVectorIndexPair.first);
            uint32_t observation = -1; // Is replaced later on.
            bool checkActionNames = false;
            if (checkActionNames) {
                bool foundActionSet = false;
                std::vector<std::string> actionNames;
                bool addedAnonymousAction = false;
                for (uint64_t choice = modelComponents.transitionMatrix.getRowGroupIndices()[bitVectorIndexPair.second];
                        choice < modelComponents.transitionMatrix.getRowGroupIndices()[bitVectorIndexPair.second +
                                                                                    1]; ++choice) {
                    if (modelComponents.choiceLabeling.get().getLabelsOfChoice(choice).empty()) {
                        STORM_LOG_THROW(!addedAnonymousAction, storm::exceptions::WrongFormatException,
                                        "Cannot have multiple anonymous actions, as these cannot be mapped correctly.");
                        actionNames.push_back("");
                        addedAnonymousAction = true;
                    } else {
                        STORM_LOG_ASSERT(
                                modelComponents.choiceLabeling.get().getLabelsOfChoice(choice).size() == 1,
                                "Expect choice labelling to contain exactly one label at this point, but found "
                                        << modelComponents.choiceLabeling.get().getLabelsOfChoice(
                                                choice).size());
                        actionNames.push_back(
                                *modelComponents.choiceLabeling.get().getLabelsOfChoice(choice).begin());
                    }
                }
                STORM_LOG_TRACE("VarObservation: " << varObservation << " Action Names: "
                                                    << storm::utility::vector::toString(actionNames));
                auto it = observationActions.find(varObservation);
                if (it == observationActions.end()) {
                    observationActions.emplace(varObservation,
                                                std::vector<std::pair<std::vector<std::string>, uint32_t>>());
                } else {
                    for (auto const &entries : it->second) {
                        STORM_LOG_TRACE(storm::utility::vector::toString(entries.first));
                        if (entries.first == actionNames) {
                            observation = entries.second;
                            foundActionSet = true;
                            break;
                        }
                    }

                    STORM_LOG_THROW(
                            generator->getOptions().isInferObservationsFromActionsSet() || foundActionSet,
                            storm::exceptions::WrongFormatException,
                            "Two states with the same observation have a different set of enabled actions, this is only allowed with a special option.");

                }
                if (!foundActionSet) {
                    observation = newObservation;
                    observationActions.find(varObservation)->second.emplace_back(actionNames, newObservation);
                    ++newObservation;
                }

                classes[bitVectorIndexPair.second] = observation;
            } else {
                classes[bitVectorIndexPair.second] = varObservation;
            }
        }
        modelComponents.observabilityClasses = classes;
    }
    return modelComponents;
}
/**
 * brief buildStateLabeling - Builds a state space labelling for our state space.
 * 
 * @return A labelling for our state space.
 * */
template <typename ValueType, typename RewardModelType, typename StateType>
storm::models::sparse::StateLabeling InfCTMCModelGenerator<ValueType, RewardModelType, StateType>::buildStateLabeling() {
    return generator->label(stateStorage, stateStorage.initialStateIndices, stateStorage.deadlockStateIndices);
}

// Explicitly instantiate the class.
template class InfCTMCModelGenerator<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

#ifdef STORM_HAVE_CARL
        /*template class InfCTMCModelGenerator<RationalNumber, storm::models::sparse::StandardRewardModel<RationalNumber>, uint32_t>;
        template class InfCTMCModelGenerator<RationalFunction, storm::models::sparse::StandardRewardModel<RationalFunction>, uint32_t>;
        template class InfCTMCModelGenerator<double, storm::models::sparse::StandardRewardModel<storm::Interval>, uint32_t>;*/
#endif