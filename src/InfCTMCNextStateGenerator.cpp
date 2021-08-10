#include "InfCTMCNextStateGenerator.h"

#include "StaminaOptions.h"
#include "ProbState.h"

#include <map>
#include <unordered_set>
#include <typeinfo>


#include <boost/container/flat_map.hpp>
#include <boost/any.hpp>

#include "storm/models/sparse/StateLabeling.h"

#include "storm/storage/expressions/SimpleValuation.h"
#include "storm/storage/sparse/PrismChoiceOrigins.h"

#include "storm/builder/jit/Distribution.h"

#include "storm/solver/SmtSolver.h"

#include "storm/utility/constants.h"
#include "storm/utility/macros.h"
#include "storm/exceptions/InvalidArgumentException.h"
#include "storm/exceptions/WrongFormatException.h"
#include "storm/exceptions/UnexpectedException.h"


namespace storm {
    namespace generator {
        /**
         * Constructor for InfCTMCNextStateGenerator. 
         * 
         * @param program The PRISM program passed in through the STORM API.
         * @param options Options for the next state generator.
         * */
        template<typename ValueType, typename StateType>
        InfCTMCNextStateGenerator<ValueType, StateType>::InfCTMCNextStateGenerator(
            storm::prism::Program const& program
            , NextStateGeneratorOptions const& options
        ) : InfCTMCNextStateGenerator<ValueType, StateType>(program.substituteConstantsFormulas(), options, false) {
            // Intentionally left empty.
        }
        /**
         * Constructor for InfCTMCNextStateGenerator.
         * 
         * @param program The PRISM program passed in through the STORM API.
         * @param options Options for the next state generator.
         * @param stateActionRewards Not implemented yet
         * */
        template<typename ValueType, typename StateType>
        InfCTMCNextStateGenerator<ValueType, StateType>::InfCTMCNextStateGenerator(
            storm::prism::Program const& program
            , NextStateGeneratorOptions const& options
            , bool stateActionRewards // Not implemented yet
        ) : NextStateGenerator<ValueType, StateType>(program.getManager(), options), program(program), rewardModels(), hasStateActionRewards(false) {
            STORM_LOG_TRACE("Creating next-state generator for PRISM program: " << program);
            STORM_LOG_THROW(
                !this->program.specifiesSystemComposition()
                , storm::exceptions::WrongFormatException
                , "The explicit next-state generator currently does not support custom system compositions."
            );

            // Only after checking validity of the program, we initialize the variable information.
            this->checkValid();
            this->variableInformation = VariableInformation(program, options.getReservedBitsForUnboundedVariables(), options.isAddOutOfBoundsStateSet());

            // Create a proper evalator.
            this->evaluator = std::make_unique<storm::expressions::ExpressionEvaluator<ValueType>>(program.getManager());

            if (this->options.isBuildAllRewardModelsSet()) {
                for (auto const& rewardModel : this->program.getRewardModels()) {
                    rewardModels.push_back(rewardModel);
                }
            } else {
                // Extract the reward models from the program based on the names we were given.
                for (auto const& rewardModelName : this->options.getRewardModelNames()) {
                    if (this->program.hasRewardModel(rewardModelName)) {
                        rewardModels.push_back(this->program.getRewardModel(rewardModelName));
                    } else {
                        STORM_LOG_THROW(
                            rewardModelName.empty()
                            , storm::exceptions::InvalidArgumentException
                            , "Cannot build unknown reward model '" << rewardModelName << "'."
                        );
                        STORM_LOG_THROW(
                            this->program.getNumberOfRewardModels() == 1
                            , storm::exceptions::InvalidArgumentException
                            , "Reference to standard reward model is ambiguous."
                        );
                    }
                }

                // If no reward model was yet added, but there was one that was given in the options, we try to build the
                // standard reward model.
                if (rewardModels.empty() && !this->options.getRewardModelNames().empty()) {
                    rewardModels.push_back(this->program.getRewardModel(0));
                }
            }

            // Determine whether any reward model has state action rewards.
            for (auto const& rewardModel : rewardModels) {
                hasStateActionRewards |= rewardModel.get().hasStateActionRewards();
            }

            // If there are terminal states we need to handle, we now need to translate all labels to expressions.
            if (this->options.hasTerminalStates()) {
                for (auto const& expressionOrLabelAndBool : this->options.getTerminalStates()) {
                    if (expressionOrLabelAndBool.first.isExpression()) {
                        this->terminalStates.push_back(std::make_pair(expressionOrLabelAndBool.first.getExpression(), expressionOrLabelAndBool.second));
                    } else {
                        if (program.hasLabel(expressionOrLabelAndBool.first.getLabel())) {
                            this->terminalStates.push_back(std::make_pair(this->program.getLabelExpression(expressionOrLabelAndBool.first.getLabel()), expressionOrLabelAndBool.second));
                        } else {
                            // If the label is not present in the program and is not a special one, we raise an error.
                            STORM_LOG_THROW(
                                expressionOrLabelAndBool.first.getLabel() == "init" || expressionOrLabelAndBool.first.getLabel() == "deadlock"
                                , storm::exceptions::InvalidArgumentException
                                , "Terminal states refer to illegal label '" << expressionOrLabelAndBool.first.getLabel() << "'."
                            );
                        }
                    }
                }
            }

            if (program.getModelType() == storm::prism::Program::ModelType::SMG) {
                moduleIndexToPlayerIndexMap = program.buildModuleIndexToPlayerIndexMap();
                actionIndexToPlayerIndexMap = program.buildActionIndexToPlayerIndexMap();
            }

            //Instantiate PrismNextStateGenerator to use getInitialStates
            generator = std::make_shared<PrismNextStateGenerator<ValueType, StateType>>(program);
        }
        /**
         * Determines whether STAMINA can handle a specific program. STAMINA handles all except for PTAs.
         * 
         * @param program The PRISM program to pass in, via the STORM API.
         * @return Whether or not STAMINA can handle it.
         * */
        template<typename ValueType, typename StateType>
        bool InfCTMCNextStateGenerator<ValueType, StateType>::canHandle(storm::prism::Program const& program) {
            // We can handle all valid prism programs (except for PTAs)
            return program.getModelType() != storm::prism::Program::ModelType::PTA;
        }
        /**
         * Determines whether this next state generator has entirely valid settings and input. If the program
         * passed in has any issues, e.g., undefined constants, this function will throw an exception directly
         * to the STORM log.
         * */
        template<typename ValueType, typename StateType>
        void InfCTMCNextStateGenerator<ValueType, StateType>::checkValid() const {
            // If the program still contains undefined constants and we are not in a parametric setting, assemble an appropriate error message.
#ifdef STORM_HAVE_CARL
            if (!std::is_same<ValueType, storm::RationalFunction>::value && program.hasUndefinedConstants()) {
#else
            if (program.hasUndefinedConstants()) {
#endif
                std::vector<std::reference_wrapper<storm::prism::Constant const>> undefinedConstants = program.getUndefinedConstants();
                std::stringstream stream;
                bool printComma = false;
                for (auto const& constant : undefinedConstants) {
                    if (printComma) {
                        stream << ", ";
                    } else {
                        printComma = true;
                    }
                    stream << constant.get().getName() << " (" << constant.get().getType() << ")";
                }
                stream << ".";
                STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "Program still contains these undefined constants: " + stream.str());
            }

#ifdef STORM_HAVE_CARL
            else if (std::is_same<ValueType, storm::RationalFunction>::value && !program.undefinedConstantsAreGraphPreserving()) {
                auto undef = program.getUndefinedConstantsAsString();
                STORM_LOG_THROW(
                    false
                    , storm::exceptions::InvalidArgumentException
                    , "The program contains undefined constants that appear in some places other than update probabilities and reward value expressions, which is not admitted. Undefined constants are: " << undef);
            }
#endif
        }
        /**
         * Gets the model type that we are working with.
         * 
         * @return The model type.
         * */
        template<typename ValueType, typename StateType>
        ModelType InfCTMCNextStateGenerator<ValueType, StateType>::getModelType() const {
            switch (program.getModelType()) {
                case storm::prism::Program::ModelType::DTMC: return ModelType::DTMC;
                case storm::prism::Program::ModelType::CTMC: return ModelType::CTMC;
                case storm::prism::Program::ModelType::MDP: return ModelType::MDP;
                case storm::prism::Program::ModelType::MA: return ModelType::MA;
                case storm::prism::Program::ModelType::POMDP: return ModelType::POMDP;
                case storm::prism::Program::ModelType::SMG: return ModelType::SMG;
                default:
                    STORM_LOG_THROW(false, storm::exceptions::WrongFormatException, "Invalid model type.");
            }
        }
        /**
         * Tells if our model (the program we passed in through the constructor) is deterministic.
         * 
         * @return Whether the model is deterministic.
         * */
        template<typename ValueType, typename StateType>
        bool InfCTMCNextStateGenerator<ValueType, StateType>::isDeterministicModel() const {
            return program.isDeterministicModel();
        }
        /**
         * Tells if our model (the PRISM program passed into the constructor) is discrete-time.
         * 
         * @return Whether or not the model is in discrete or continuous time.
         * */
        template<typename ValueType, typename StateType>
        bool InfCTMCNextStateGenerator<ValueType, StateType>::isDiscreteTimeModel() const {
            return program.isDiscreteTimeModel();
        }
        /**
         * Tells if our model (PRISM program) is partially observable.
         * 
         * @return Whether or not the model is partially observable.
         * */
        template<typename ValueType,typename StateType>
        bool InfCTMCNextStateGenerator<ValueType, StateType>::isPartiallyObservable() const {
            return program.isPartiallyObservable();
        }
        /**
         * Gets a vector of our initial states.
         * 
         * @param stateToIdCallback TODO...I'm not entirely sure what this parameter is.
         * @return All initial states
         * */
        template<typename ValueType, typename StateType>
        std::vector<StateType> InfCTMCNextStateGenerator<ValueType, StateType>::getInitialStates(StateToIdCallback const& stateToIdCallback) {
            doReachabilityAnalysis();
#ifdef DEBUG_PRINTS
            std::cout << "In getInitialStates " << std::endl;
#endif
            std::vector<StateType> initialStateIndices = generator->getInitialStates(stateToIdCallback);

            return initialStateIndices;
        }
        /**
         * Gets the state index or adds it to the state-exploration queue.
         * 
         * @param state State to get index of or add to queue.
         * @return The index of that state
         * */
        template<typename ValueType, typename StateType>
        StateType InfCTMCNextStateGenerator<ValueType, StateType>::getOrAddStateIndex(CompressedState const& state) {
#ifdef DEBUG_PRINTS
            std::cout << "std::bind dereferenced correctly to getOrAddStateIndex." << std::endl;
#endif
            StateType actualIndex = static_cast<StateType>(state.getAsInt(0, 64));  // get 64 bits starting at bit 0
#ifdef DEBUG_PRINTS
            std::cout << "Got actualIndex: " << actualIndex << " (in getOrAddStateIndex)" << std::endl;
#endif
	    // Consider doing something similar to this with the stateMap
            // StateType newIndex = static_cast<StateType>(stateStorage.getNumberOfStates());

            // // Check, if the state was already registered.
            // std::pair<StateType, std::size_t> actualIndexBucketPair = stateStorage.stateToId.findOrAddAndGetBucket(state, newIndex);

            // StateType actualIndex = actualIndexBucketPair.first;

            // if (actualIndex == newIndex) {
            //     if (options.explorationOrder == storm::builder::ExplorationOrder::Dfs) {
            //         statesToExplore.emplace_front(state, actualIndex);

            //         // Reserve one slot for the new state in the remapping.
            //         stateRemapping.get().push_back(storm::utility::zero<StateType>());
            //     } else if (options.explorationOrder == storm::builder::ExplorationOrder::Bfs) {
            //         statesToExplore.emplace_back(state, actualIndex);
            //         auto newProbState = new ProbState(actualIndex);
            //         stateMap.emplace(actualIndex, newProbState);
            //     } else {
            //         STORM_LOG_ASSERT(false, "Invalid exploration order.");
            //     }
            // }

            return actualIndex;
        }
        /**
         * Performs a reachability analysis for our state space doing a breadth first search.
         * */
        template<typename ValueType, typename StateType>
        void InfCTMCNextStateGenerator<ValueType, StateType>::doReachabilityAnalysis() {
            // Check if model is based on infinite state system and print warning

            // TODO: display progress

            // Check that the model type is CTMC
            STORM_LOG_THROW(
                program.getModelType() == storm::prism::Program::ModelType::CTMC
                , storm::exceptions::WrongFormatException
                , "Probabilistic model construction not supported for model type: " << program.getModelType()
            );

            // Create unordered_set statesK of ProbStates
            std::unordered_set<ProbState> statesK;
            // Create linked list exploredK of ProbStates
            std::list<ProbState> exploredK;

            // Get initial state and set reachProb
            std::function<StateType (CompressedState const&)> stateToIdCallback = std::bind(&InfCTMCNextStateGenerator<ValueType, StateType>::getOrAddStateIndex, this, std::placeholders::_1);
            std::vector<StateType> initialStates = generator->getInitialStates(stateToIdCallback); //???
            for (auto &state : initialStates) {
                // Check if state is in stateMap
                ProbState probInitState;
                auto initStateFound = stateMap.find(state);
                if(initStateFound != stateMap.end()) {
                    probInitState = initStateFound->second;
                } else {
                    // Add initial state(s) to explore, states, and to the model //???
                    probInitState.setCurReachabilityProb(1);
                    stateMap.emplace(state, probInitState);
                }

                // Add state to exploration queue
                exploredK.push_back(probInitState);
                statesK.emplace(probInitState);
            }

            // Start the exploration
            int prevStateCount = stateMap.size();
            double perimReachability = 1;

            // State search
            //while(perimReachability >= Options.getProbErrorWindow()/Options.getMispredictionFactor()) {
            while(!exploredK.empty()) {
                // Get next state to explore
                ProbState curProbState = exploredK.front();
                // delete first element
                exploredK.pop_front();
                CompressedState curState = curProbState.state;
                StateType curId = curProbState.stateId;

                generator->load(curProbState.state);

                // TODO: figure out the storm equivalent for this
                // if(propertyExpression) {
                //     // set absorbing state if property is not satisfied
                //     curProbState.setStateAbsorbing(true);
                //     // perimeterStates.addElement(curProbState.toString()); //???
                //     curProbState.setStateTerminal(false);
                //     continue;
                // }

                double curStateReachability = curProbState.getCurReachabilityProb();
                bool exploreState = (!curProbState.isStateTerminal() || curStateReachability >= reachabilityThreshold);
                if(exploreState) {
                    storm::generator::StateBehavior<ValueType, StateType> behavior = generator->expand(stateToIdCallback);
#ifdef DEBUG_PRINTS		    // Debug statements that print the variable values in each state		    
		    std::cout << curProbState.stateId << ": " << this->stateToString(curProbState.state) << std::endl;
		    auto valuation = unpackStateIntoValuation(curProbState.state, this->variableInformation, program.getManager());
		    std::cout << "valuation: " << valuation.toString() << std::endl;
#endif		    

                    // optimization: If reachability is 0, we don't need to do
                    //               the rest of the computation
                    if(0 == curStateReachability) {
                        if(behavior.empty()) {
                            //be sad
                            std::cout << "behavior empty" << std::endl;
                        } else {
                            for(auto const &choice : behavior) {
                                for(auto const &stateProbabilityPair : choice) {
                                    // get next ProbState and check if state is in stateMap
                                    auto nextStateFound = stateMap.find(stateProbabilityPair.first);
                                    if(nextStateFound != stateMap.end()) {
                                        ProbState nextProbState = nextStateFound->second;
                                        // Add state to states and explored if new state
                                        if(statesK.emplace(nextProbState).second) {
                                            exploredK.push_back(nextProbState);
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        auto totalNumberOfChoices = behavior.getNumberOfChoices();
                        ValueType totalExitRate = this->isDiscreteTimeModel() ? static_cast<ValueType>(totalNumberOfChoices) : storm::utility::zero<ValueType>();
                        for(auto const &choice : behavior) {
                            totalExitRate += choice.getTotalMass();
                        }

                        for(auto const &choice : behavior) {
                            for(auto const &stateProbabilityPair : choice) {
                                // get next ProbState id and exit rate
                                StateType nextState = stateProbabilityPair.first;
                                ValueType tranRate = stateProbabilityPair.second;
                                ValueType tranProb = tranRate / totalExitRate;
                                ValueType leavingProb = tranProb * curStateReachability;

                                // Check if state exists
                                auto nextStateFound = stateMap.find(nextState);
                                if(nextStateFound != stateMap.end()) {
                                    ProbState* nextProbState = &(nextStateFound->second);
                                    nextProbState->addToReachability(leavingProb);

                                    // Add state to states and explored if new state
                                    if(statesK.emplace(*nextProbState).second) {
                                        exploredK.push_back(*nextProbState);
                                    }
                                } else {
                                    // Create state and add to state map
                                    ProbState nextProbState(nextState);                                    
                                    nextProbState.addToReachability(leavingProb);
                                    stateMap.emplace(nextState, nextProbState);
                                    statesK.emplace(nextProbState);
                                    exploredK.push_back(nextProbState);
                                }
                            }
                        }
                    }
                    curProbState.setCurReachabilityProb(0);
                    curProbState.setStateTerminal(false);
                }

                //update progress with stateMap.size() + 1
                std::cout << "progress: " << stateMap.size() << std::endl;
            }
            exploredK.clear();
            statesK.clear();
            for (auto &state : initialStates) {
                // Check if state is in stateMap
                ProbState probInitState;
                auto initStateFound = stateMap.find(state);
                if(initStateFound != stateMap.end()) {
                    probInitState = initStateFound->second;
                } else {
                    // Add initial state(s) to explore, states, and to the model //???
                    probInitState.setCurReachabilityProb(1);
                    stateMap.emplace(state, probInitState);
                }

                // Add state to exploration queue
                exploredK.push_back(probInitState);
                statesK.emplace(probInitState);
            }
            perimReachability = 0;
            // Iterate over state map
            for(auto &localState : stateMap) {
                // Add reachability to threshold if state is terminal
                if(localState.second.isStateTerminal()) {
                    perimReachability += reachabilityThreshold;
                }
            }

            reachabilityThreshold /= StaminaOptions::getKappaReductionFactor(); //not implemented???

            // Finish progress display
            std::cout << "Finished exploring " << stateMap.size() << " states" << std::endl;

            // TODO: Reset property expression ???
        }
        

        //TODO: Override this method?
        /**
         * Expands all choices for a state. First, state rewards are computed, and then choices are expanded
         * and explored.
         * 
         * @param stateToIdCallback The state to explore.
         * @return A list of state choices and rewards.
         * */
        template<typename ValueType, typename StateType>
        StateBehavior<ValueType, StateType> InfCTMCNextStateGenerator<ValueType, StateType>::expand(StateToIdCallback const& stateToIdCallback) {
            // Prepare the result, in case we return early.
            StateBehavior<ValueType, StateType> result;

            // First, construct the state rewards, as we may return early if there are no choices later and we already
            // need the state rewards then.
            for (auto const& rewardModel : rewardModels) {
                ValueType stateRewardValue = storm::utility::zero<ValueType>();
                if (rewardModel.get().hasStateRewards()) {
                    for (auto const& stateReward : rewardModel.get().getStateRewards()) {
                        if (this->evaluator->asBool(stateReward.getStatePredicateExpression())) {
                            stateRewardValue += ValueType(this->evaluator->asRational(stateReward.getRewardValueExpression()));
                        }
                    }
                }
                result.addStateReward(stateRewardValue);
            }

            // If a terminal expression was set and we must not expand this state, return now.
            if (!this->terminalStates.empty()) {
                for (auto const& expressionBool : this->terminalStates) {
                    if (this->evaluator->asBool(expressionBool.first) == expressionBool.second) {
                        return result;
                    }
                }
            }

            // Get all choices for the state.
            result.setExpanded();

            std::vector<Choice<ValueType>> allChoices;
            if (this->getOptions().isApplyMaximalProgressAssumptionSet()) {
                // First explore only edges without a rate
                allChoices = getUnlabeledChoices(*this->state, stateToIdCallback, CommandFilter::Probabilistic);
                addLabeledChoices(allChoices, *this->state, stateToIdCallback, CommandFilter::Probabilistic);
                if (allChoices.empty()) {
                    // Expand the Markovian edges if there are no probabilistic ones.
                    allChoices = getUnlabeledChoices(*this->state, stateToIdCallback, CommandFilter::Markovian);
                    addLabeledChoices(allChoices, *this->state, stateToIdCallback, CommandFilter::Markovian);
                }
            } else {
                allChoices = getUnlabeledChoices(*this->state, stateToIdCallback);
                addLabeledChoices(allChoices, *this->state, stateToIdCallback);
            }

            std::size_t totalNumberOfChoices = allChoices.size();

            // If there is not a single choice, we return immediately, because the state has no behavior (other than
            // the state reward).
            if (totalNumberOfChoices == 0) {
                return result;
            }

            // If the model is a deterministic model, we need to fuse the choices into one.
            if (this->isDeterministicModel() && totalNumberOfChoices > 1) {
                Choice<ValueType> globalChoice;

                if (this->options.isAddOverlappingGuardLabelSet()) {
                    this->overlappingGuardStates->push_back(stateToIdCallback(*this->state));
                }

                // For CTMCs, we need to keep track of the total exit rate to scale the action rewards later. For DTMCs
                // this is equal to the number of choices, which is why we initialize it like this here.
                ValueType totalExitRate = this->isDiscreteTimeModel() ? static_cast<ValueType>(totalNumberOfChoices) : storm::utility::zero<ValueType>();

                // Iterate over all choices and combine the probabilities/rates into one choice.
                for (auto const& choice : allChoices) {
                    for (auto const& stateProbabilityPair : choice) {
                        if (this->isDiscreteTimeModel()) {
                            globalChoice.addProbability(stateProbabilityPair.first, stateProbabilityPair.second / totalNumberOfChoices);
                        } else {
                            globalChoice.addProbability(stateProbabilityPair.first, stateProbabilityPair.second);
                        }
                    }

                    if (hasStateActionRewards && !this->isDiscreteTimeModel()) {
                        totalExitRate += choice.getTotalMass();
                    }

                    if (this->options.isBuildChoiceLabelsSet() && choice.hasLabels()) {
                        globalChoice.addLabels(choice.getLabels());
                    }

                    if (this->options.isBuildChoiceOriginsSet() && choice.hasOriginData()) {
                        globalChoice.addOriginData(choice.getOriginData());
                    }
                }

                // Now construct the state-action reward for all selected reward models.
                for (auto const& rewardModel : rewardModels) {
                    ValueType stateActionRewardValue = storm::utility::zero<ValueType>();
                    if (rewardModel.get().hasStateActionRewards()) {
                        for (auto const& stateActionReward : rewardModel.get().getStateActionRewards()) {
                            for (auto const& choice : allChoices) {
                                if (stateActionReward.getActionIndex() == choice.getActionIndex() && this->evaluator->asBool(stateActionReward.getStatePredicateExpression())) {
                                    stateActionRewardValue += ValueType(this->evaluator->asRational(stateActionReward.getRewardValueExpression())) * choice.getTotalMass();
                                }
                            }

                        }
                    }
                    if (hasStateActionRewards) {
                        globalChoice.addReward(stateActionRewardValue / totalExitRate);
                    }
                }

                // Move the newly fused choice in place.
                allChoices.clear();
                allChoices.push_back(std::move(globalChoice));
            }

            // For SMG we check whether the state has a unique player
            if (program.getModelType() == storm::prism::Program::ModelType::SMG && allChoices.size() > 1) {
                auto choiceIt = allChoices.begin();
                // This should have been catched while creating the choice already
                STORM_LOG_ASSERT(
                    choiceIt->hasPlayerIndex()
                    , "State '" << this->stateToString(*this->state) << "' features a choice without player index."
                ); 
                storm::storage::PlayerIndex statePlayerIndex = choiceIt->getPlayerIndex();
                // This should have been catched while creating the choice already
                STORM_LOG_ASSERT(
                    statePlayerIndex != storm::storage::INVALID_PLAYER_INDEX
                    , "State '" << this->stateToString(*this->state) << "' features a choice with invalid player index."
                ); 
                for (++choiceIt; choiceIt != allChoices.end(); ++choiceIt) {
                    // This should have been catched while creating the choice already
                    STORM_LOG_ASSERT(choiceIt->hasPlayerIndex(), "State '" << this->stateToString(*this->state) << "' features a choice without player index."); 
                    // This should have been catched while creating the choice already
                    STORM_LOG_ASSERT(
                        choiceIt->getPlayerIndex() != storm::storage::INVALID_PLAYER_INDEX
                        , "State '" << this->stateToString(*this->state) << "' features a choice with invalid player index."
                    ); 
                    STORM_LOG_THROW(
                        statePlayerIndex == choiceIt->getPlayerIndex()
                        , storm::exceptions::WrongFormatException
                        , "The player for state '" << this->stateToString(*this->state) << "' is not unique. At least one choice is owned by player '" 
                                                << statePlayerIndex << "' while another is owned by player '" << choiceIt->getPlayerIndex() << "'."
                    );
                }
            }

            // Move all remaining choices in place.
            for (auto& choice : allChoices) {
                result.addChoice(std::move(choice));
            }

            this->postprocess(result);

            return result;
        }
        /**
         * Updates a state and returns the updated state.
         * 
         * @param state State to update
         * @param update The update to be made to the state
         * @return The updated state
         * */
        template<typename ValueType, typename StateType>
        CompressedState InfCTMCNextStateGenerator<ValueType, StateType>::applyUpdate(CompressedState const& state, storm::prism::Update const& update) {
            CompressedState newState(state);

            // NOTE: the following process assumes that the assignments of the update are ordered in such a way that the
            // assignments to boolean variables precede the assignments to all integer variables and that within the
            // types, the assignments to variables are ordered (in ascending order) by the expression variables.
            // This is guaranteed for PRISM models, by sorting the assignments as soon as an update is created.

            auto assignmentIt = update.getAssignments().begin();
            auto assignmentIte = update.getAssignments().end();

            // Iterate over all boolean assignments and carry them out.
            auto boolIt = this->variableInformation.booleanVariables.begin();
            for (; assignmentIt != assignmentIte && assignmentIt->getExpression().hasBooleanType(); ++assignmentIt) {
                while (assignmentIt->getVariable() != boolIt->variable) {
                    ++boolIt;
                }
                newState.set(boolIt->bitOffset, this->evaluator->asBool(assignmentIt->getExpression()));
            }

            // Iterate over all integer assignments and carry them out.
            auto integerIt = this->variableInformation.integerVariables.begin();
            for (; assignmentIt != assignmentIte && assignmentIt->getExpression().hasIntegerType(); ++assignmentIt) {
                while (assignmentIt->getVariable() != integerIt->variable) {
                    ++integerIt;
                }
                int_fast64_t assignedValue = this->evaluator->asInt(assignmentIt->getExpression());
                if (this->options.isAddOutOfBoundsStateSet()) {
                    if (assignedValue < integerIt->lowerBound || assignedValue > integerIt->upperBound) {
                        return this->outOfBoundsState;
                    }
                } else if (integerIt->forceOutOfBoundsCheck || this->options.isExplorationChecksSet()) {
                    STORM_LOG_THROW(
                        assignedValue >= integerIt->lowerBound
                        , storm::exceptions::WrongFormatException
                        , "The update " << update << " leads to an out-of-bounds value (" << assignedValue << ") for the variable '" << assignmentIt->getVariableName() << "'."
                    );
                    STORM_LOG_THROW(
                        assignedValue <= integerIt->upperBound
                        , storm::exceptions::WrongFormatException
                        , "The update " << update << " leads to an out-of-bounds value (" << assignedValue << ") for the variable '" << assignmentIt->getVariableName() << "'."
                    );
                }
                newState.setFromInt(integerIt->bitOffset, integerIt->bitWidth, assignedValue - integerIt->lowerBound);
                STORM_LOG_ASSERT(
                    static_cast<int_fast64_t>(newState.getAsInt(integerIt->bitOffset, integerIt->bitWidth)) + integerIt->lowerBound == assignedValue
                    , "Writing to the bit vector bucket failed (read " << newState.getAsInt(integerIt->bitOffset, integerIt->bitWidth) << " but wrote " << assignedValue << ")."
                );
            }

            // Check that we processed all assignments.
            STORM_LOG_ASSERT(assignmentIt == assignmentIte, "Not all assignments were consumed.");
#ifdef DEBUG_PRINTS
            std::cout << "Returning new state: " << newState << std::endl;
#endif
            return newState;
        }

        struct ActiveCommandData {
            ActiveCommandData(
                storm::prism::Module const* modulePtr
                , std::set<uint_fast64_t> const* commandIndicesPtr
                , typename std::set<uint_fast64_t>::const_iterator currentCommandIndexIt
            ) : modulePtr(modulePtr), commandIndicesPtr(commandIndicesPtr), currentCommandIndexIt(currentCommandIndexIt) {
                // Intentionally left empty
            }
            storm::prism::Module const* modulePtr;
            std::set<uint_fast64_t> const* commandIndicesPtr;
            typename std::set<uint_fast64_t>::const_iterator currentCommandIndexIt;
        };
        /**
         * Gets all active commands, filtered by a CommandFilter, at a specific action index.
         * 
         * @param actionIndex The index to look for active commands.
         * @param commandFilter The filter to apply.
         * @return All active commands.
         * */
        template<typename ValueType, typename StateType>
        boost::optional<std::vector<std::vector<std::reference_wrapper<storm::prism::Command const>>>> InfCTMCNextStateGenerator<ValueType, StateType>::getActiveCommandsByActionIndex(
            uint_fast64_t const& actionIndex
            , CommandFilter const& commandFilter
        ) {

            // First check whether there is at least one enabled command at each module
            // This avoids evaluating unnecessarily many guards.
            // If we find one module without an enabled command, we return boost::none.
            // At the same time, we store pointers to the relevant modules, the relevant command sets and the first enabled command within each set.

            // Iterate over all modules.
            std::vector<ActiveCommandData> activeCommands;
            for (uint_fast64_t i = 0; i < program.getNumberOfModules(); ++i) {
                storm::prism::Module const& module = program.getModule(i);

                // If the module has no command labeled with the given action, we can skip this module.
                if (!module.hasActionIndex(actionIndex)) {
                    continue;
                }

                std::set<uint_fast64_t> const& commandIndices = module.getCommandIndicesByActionIndex(actionIndex);

                // If the module contains the action, but there is no command in the module that is labeled with
                // this action, we don't have any feasible command combinations.
                if (commandIndices.empty()) {
                    return boost::none;
                }

                // Look up commands by their indices and check if the guard evaluates to true in the given state.
                bool hasOneEnabledCommand = false;
                for (auto commandIndexIt = commandIndices.begin(), commandIndexIte = commandIndices.end(); commandIndexIt != commandIndexIte; ++commandIndexIt) {
                    storm::prism::Command const& command = module.getCommand(*commandIndexIt);
                    if (commandFilter != CommandFilter::All) {
                        STORM_LOG_ASSERT(commandFilter == CommandFilter::Markovian || commandFilter == CommandFilter::Probabilistic, "Unexpected command filter.");
                        if ((commandFilter == CommandFilter::Markovian) != command.isMarkovian()) {
                            continue;
                        }
                    }
                    if (this->evaluator->asBool(command.getGuardExpression())) {
                        // Found the first enabled command for this module.
                        hasOneEnabledCommand = true;
                        activeCommands.emplace_back(&module, &commandIndices, commandIndexIt);
                        break;
                    }
                }

                if (!hasOneEnabledCommand) {
                    return boost::none;
                }
            }

            // If we reach this point, there has to be at least one active command for each relevant module.
            std::vector<std::vector<std::reference_wrapper<storm::prism::Command const>>> result;

            // Iterate over all command sets.
            for (auto const& activeCommand : activeCommands) {
                std::vector<std::reference_wrapper<storm::prism::Command const>> commands;

                auto commandIndexIt = activeCommand.currentCommandIndexIt;
                // The command at the current position is already known to be enabled
                commands.push_back(activeCommand.modulePtr->getCommand(*commandIndexIt));

                // Look up commands by their indices and add them if the guard evaluates to true in the given state.
                auto commandIndexIte = activeCommand.commandIndicesPtr->end();
                for (++commandIndexIt; commandIndexIt != commandIndexIte; ++commandIndexIt) {
                    storm::prism::Command const& command = activeCommand.modulePtr->getCommand(*commandIndexIt);
                    if (commandFilter != CommandFilter::All) {
                        STORM_LOG_ASSERT(commandFilter == CommandFilter::Markovian || commandFilter == CommandFilter::Probabilistic, "Unexpected command filter.");
                        if ((commandFilter == CommandFilter::Markovian) != command.isMarkovian()) {
                            continue;
                        }
                    }
                    if (this->evaluator->asBool(command.getGuardExpression())) {
                        commands.push_back(command);
                    }
                }

                result.push_back(std::move(commands));
            }

            STORM_LOG_ASSERT(!result.empty(), "Expected non-empty list.");
            return result;
        }
        /**
         * Gets a std::vector containing all unlabeled choices in our PRISM model.
         * 
         * @param state The state to get all unlabeled choices from.
         * @param stateToIdCallBack
         * @param commandFilter The filter to 
         * @return All unlabeled choices in the form of a std::vector.
         * */
        template<typename ValueType, typename StateType>
        std::vector<Choice<ValueType>> InfCTMCNextStateGenerator<ValueType, StateType>::getUnlabeledChoices(
            CompressedState const& state
            , StateToIdCallback stateToIdCallback
            , CommandFilter const& commandFilter
        ) {
            std::vector<Choice<ValueType>> result;

            // Iterate over all modules.
            for (uint_fast64_t i = 0; i < program.getNumberOfModules(); ++i) {
                storm::prism::Module const& module = program.getModule(i);

                // Iterate over all commands.
                for (uint_fast64_t j = 0; j < module.getNumberOfCommands(); ++j) {
                    storm::prism::Command const& command = module.getCommand(j);

                    // Only consider unlabeled commands.
                    if (command.isLabeled()) continue;

                    if (commandFilter != CommandFilter::All) {
                        STORM_LOG_ASSERT(commandFilter == CommandFilter::Markovian || commandFilter == CommandFilter::Probabilistic, "Unexpected command filter.");
                        if ((commandFilter == CommandFilter::Markovian) != command.isMarkovian()) {
                            continue;
                        }
                    }

                    // Skip the command, if it is not enabled.
                    if (!this->evaluator->asBool(command.getGuardExpression())) {
                        continue;
                    }

                    result.push_back(Choice<ValueType>(command.getActionIndex(), command.isMarkovian()));
                    Choice<ValueType>& choice = result.back();

                    // Remember the choice origin only if we were asked to.
                    if (this->options.isBuildChoiceOriginsSet()) {
                        CommandSet commandIndex { command.getGlobalIndex() };
                        choice.addOriginData(boost::any(std::move(commandIndex)));
                    }

                    // Iterate over all updates of the current command.
                    ValueType probabilitySum = storm::utility::zero<ValueType>();
                    for (uint_fast64_t k = 0; k < command.getNumberOfUpdates(); ++k) {
                        storm::prism::Update const& update = command.getUpdate(k);

                        ValueType probability = this->evaluator->asRational(update.getLikelihoodExpression());
                        if (probability != storm::utility::zero<ValueType>()) {
                            // Obtain target state index and add it to the list of known states. If it has not yet been
                            // seen, we also add it to the set of states that have yet to be explored.
                            storm::generator::CompressedState updatedState = applyUpdate(state, update);
                            // StateType stateIndex = this->getOrAddStateIndex(updatedState); 
                            StateType stateIndex = stateToIdCallback(updatedState);
#ifdef DEBUG_PRINTS
                            std::cout << "Got State Index " << stateIndex << std::endl;
                            if (stateToIdCallback == nullptr) {
                                std::cout << "stateToIdCallback is nullptr." << std::endl;
                            }
                            else {
                                // stateToIdCallback = std::bind(&InfCTMCNextStateGenerator<ValueType, StateType>::getOrAddStateIndex, this, std::placeholders::_1);
                                std::cout << "stateToIdCallback is NOT nullptr." << std::endl;
                            }
                            // std::cout << "stateToIdCallback is " << stateToIdCallback.target() << std::endl;
#endif

                            // Update the choice by adding the probability/target state to it.
                            choice.addProbability(stateIndex, probability);
                            if (this->options.isExplorationChecksSet()) {
                                probabilitySum += probability;
                            }
                        }
                    }

                    // Create the state-action reward for the newly created choice.
                    for (auto const& rewardModel : rewardModels) {
                        ValueType stateActionRewardValue = storm::utility::zero<ValueType>();
                        if (rewardModel.get().hasStateActionRewards()) {
                            for (auto const& stateActionReward : rewardModel.get().getStateActionRewards()) {
                                if (stateActionReward.getActionIndex() == choice.getActionIndex() && this->evaluator->asBool(stateActionReward.getStatePredicateExpression())) {
                                    stateActionRewardValue += ValueType(this->evaluator->asRational(stateActionReward.getRewardValueExpression()));
                                }
                            }
                        }
                        choice.addReward(stateActionRewardValue);
                    }

                    if (program.getModelType() == storm::prism::Program::ModelType::SMG) {
                        storm::storage::PlayerIndex const& playerOfModule = moduleIndexToPlayerIndexMap.at(i);
                        STORM_LOG_THROW(
                            playerOfModule != storm::storage::INVALID_PLAYER_INDEX
                            , storm::exceptions::WrongFormatException
                            , "Module " << module.getName() << " is not owned by any player but has at least one enabled, unlabeled command."
                        );
                        choice.setPlayerIndex(playerOfModule);
                    }

                    if (this->options.isExplorationChecksSet()) {
                        // Check that the resulting distribution is in fact a distribution.
                        STORM_LOG_THROW(
                            !program.isDiscreteTimeModel() || this->comparator.isOne(probabilitySum)
                            , storm::exceptions::WrongFormatException
                            , "Probabilities do not sum to one for command '" << command << "' (actually sum to " << probabilitySum << ")."
                        );
                    }
                }
            }

            return result;
        }
        /**
         * Recursively generates a synchronized distribution of states.
         * 
         * @param state The state to start at.
         * @param probability The probability of taking that state
         * @param position The index of that state.
         * @param iteratorList A list of commands that we are to go through, applying iteratorList[position]'s updates to state each call.
         * */
        template<typename ValueType, typename StateType>
        void InfCTMCNextStateGenerator<ValueType, StateType>::generateSynchronizedDistribution(
            storm::storage::BitVector const& state
            , ValueType const& probability
            , uint64_t position
            , std::vector<std::vector<std::reference_wrapper<storm::prism::Command const>>::const_iterator> const& iteratorList
            , storm::builder::jit::Distribution<StateType, ValueType>& distribution
            , StateToIdCallback stateToIdCallback
        ) {

            if (storm::utility::isZero<ValueType>(probability)) {
                return;
            }

            if (position >= iteratorList.size()) {
                StateType id = stateToIdCallback(state);
                distribution.add(id, probability);
            } else {
                storm::prism::Command const& command = *iteratorList[position];
                for (uint_fast64_t j = 0; j < command.getNumberOfUpdates(); ++j) {
                    storm::prism::Update const& update = command.getUpdate(j);
                    generateSynchronizedDistribution(
                        applyUpdate(state, update)
                        , probability * this->evaluator->asRational(update.getLikelihoodExpression())
                        , position + 1
                        , iteratorList
                        , distribution
                        , stateToIdCallback
                    );
                }
            }
        }
        /**
         * Gets labeled choices to the next state's choice list
         * 
         * @param choices Choices to add.
         * @param state State to add them to
         * @param stateToIdCallback Needed to get the distribution
         * @param commandFilter Filter for commands.
         * 
         * @return Labeled choices for the state
         * */
        template<typename ValueType, typename StateType>
        void InfCTMCNextStateGenerator<ValueType, StateType>::addLabeledChoices(
            std::vector<Choice<ValueType>>& choices
            , CompressedState const& state
            , StateToIdCallback stateToIdCallback
            , CommandFilter const& commandFilter
        ) {

            for (uint_fast64_t actionIndex : program.getSynchronizingActionIndices()) {
                boost::optional<std::vector<std::vector<std::reference_wrapper<storm::prism::Command const>>>> optionalActiveCommandLists = getActiveCommandsByActionIndex(actionIndex, commandFilter);

                // Only process this action label, if there is at least one feasible solution.
                if (!optionalActiveCommandLists) {
                    continue; // Continue to next iteration of loop.
                }

                std::vector<std::vector<std::reference_wrapper<storm::prism::Command const>>> const& activeCommandList = optionalActiveCommandLists.get();
                std::vector<std::vector<std::reference_wrapper<storm::prism::Command const>>::const_iterator> iteratorList(activeCommandList.size());

                // Initialize the list of iterators.
                for (size_t i = 0; i < activeCommandList.size(); ++i) {
                    iteratorList[i] = activeCommandList[i].cbegin();
                }

                storm::builder::jit::Distribution<StateType, ValueType> distribution;

                // As long as there is one feasible combination of commands, keep on expanding it.
                bool done = false;
                while (!done) {
                    distribution.clear();
                    generateSynchronizedDistribution(state, storm::utility::one<ValueType>(), 0, iteratorList, distribution, stateToIdCallback);
                    distribution.compress();

                    // At this point, we applied all commands of the current command combination and newTargetStates
                    // contains all target states and their respective probabilities. That means we are now ready to
                    // add the choice to the list of transitions.
                    choices.push_back(Choice<ValueType>(actionIndex));

                    // Now create the actual distribution.
                    Choice<ValueType>& choice = choices.back();

                    if (program.getModelType() == storm::prism::Program::ModelType::SMG) {
                        storm::storage::PlayerIndex const& playerOfAction = actionIndexToPlayerIndexMap.at(actionIndex);
                        STORM_LOG_THROW(
                            playerOfAction != storm::storage::INVALID_PLAYER_INDEX
                            , storm::exceptions::WrongFormatException
                            , "Action " << program.getActionName(actionIndex) << " is not owned by any player but has at least one enabled, unlabeled (synchronized) command."
                        );
                        choice.setPlayerIndex(playerOfAction);
                    }

                    // Remember the choice label and origins only if we were asked to.
                    if (this->options.isBuildChoiceLabelsSet()) {
                        choice.addLabel(program.getActionName(actionIndex));
                    }
                    if (this->options.isBuildChoiceOriginsSet()) {
                        CommandSet commandIndices;
                        for (uint_fast64_t i = 0; i < iteratorList.size(); ++i) {
                            commandIndices.insert(iteratorList[i]->get().getGlobalIndex());
                        }
                        choice.addOriginData(boost::any(std::move(commandIndices)));
                    }

                    // Add the probabilities/rates to the newly created choice.
                    ValueType probabilitySum = storm::utility::zero<ValueType>();
                    choice.reserve(std::distance(distribution.begin(), distribution.end()));
                    for (auto const& stateProbability : distribution) {
                        choice.addProbability(stateProbability.getState(), stateProbability.getValue());
                        if (this->options.isExplorationChecksSet()) {
                            probabilitySum += stateProbability.getValue();
                        }
                    }

                    if (this->options.isExplorationChecksSet()) {
                        // Check that the resulting distribution is in fact a distribution.
                        STORM_LOG_THROW(
                            !program.isDiscreteTimeModel() || !this->comparator.isConstant(probabilitySum) || this->comparator.isOne(probabilitySum)
                            , storm::exceptions::WrongFormatException
                            , "Sum of update probabilities do not some to one for some command (actually sum to " << probabilitySum << ")."
                        );
                    }

                    // Create the state-action reward for the newly created choice.
                    for (auto const& rewardModel : rewardModels) {
                        ValueType stateActionRewardValue = storm::utility::zero<ValueType>();
                        if (rewardModel.get().hasStateActionRewards()) {
                            for (auto const& stateActionReward : rewardModel.get().getStateActionRewards()) {
                                if (stateActionReward.getActionIndex() == choice.getActionIndex() && this->evaluator->asBool(stateActionReward.getStatePredicateExpression())) {
                                    stateActionRewardValue += ValueType(this->evaluator->asRational(stateActionReward.getRewardValueExpression()));
                                }
                            }
                        }
                        choice.addReward(stateActionRewardValue);
                    }

                    // Now, check whether there is one more command combination to consider.
                    bool movedIterator = false;
                    for (int_fast64_t j = iteratorList.size() - 1; !movedIterator && j >= 0; --j) {
                        ++iteratorList[j];
                        if (iteratorList[j] != activeCommandList[j].end()) {
                            movedIterator = true;
                        } else {
                            // Reset the iterator to the beginning of the list.
                            iteratorList[j] = activeCommandList[j].begin();
                        }
                    }

                    done = !movedIterator;
                }
            }
        }
        /**
         * 
         * */
        template<typename ValueType, typename StateType>
        std::map<std::string, storm::storage::PlayerIndex> InfCTMCNextStateGenerator<ValueType, StateType>::getPlayerNameToIndexMap() const {
            return program.getPlayerNameToIndexMapping();
        }

        template<typename ValueType, typename StateType>
        storm::models::sparse::StateLabeling InfCTMCNextStateGenerator<ValueType, StateType>::label(
            storm::storage::sparse::StateStorage<StateType> const& stateStorage
            , std::vector<StateType> const& initialStateIndices
            , std::vector<StateType> const& deadlockStateIndices
        ) {
            // Gather a vector of labels and their expressions.
            std::vector<std::pair<std::string, storm::expressions::Expression>> labels;
            if (this->options.isBuildAllLabelsSet()) {
                for (auto const& label : program.getLabels()) {
                    labels.push_back(std::make_pair(label.getName(), label.getStatePredicateExpression()));
                }
            } else {
                for (auto const& labelName : this->options.getLabelNames()) {
                    if (program.hasLabel(labelName)) {
                        labels.push_back(std::make_pair(labelName, program.getLabelExpression(labelName)));
                    } else {
                        STORM_LOG_THROW(labelName == "init" || labelName == "deadlock", storm::exceptions::InvalidArgumentException, "Cannot build labeling for unknown label '" << labelName << "'.");
                    }
                }
            }

            return NextStateGenerator<ValueType, StateType>::label(stateStorage, initialStateIndices, deadlockStateIndices, labels);
        }

        template<typename ValueType, typename StateType>
        storm::storage::BitVector InfCTMCNextStateGenerator<ValueType, StateType>::evaluateObservationLabels(CompressedState const& state) const {
            // TODO consider to avoid reloading by computing these bitvectors in an earlier build stage
            unpackStateIntoEvaluator(state, this->variableInformation, *this->evaluator);

            storm::storage::BitVector result(program.getNumberOfObservationLabels() * 64);
            for (uint64_t i = 0; i < program.getNumberOfObservationLabels(); ++i) {
                result.setFromInt(64*i,64,this->evaluator->asInt(program.getObservationLabels()[i].getStatePredicateExpression()));
            }
            return result;
        }


        template<typename ValueType, typename StateType>
        std::size_t InfCTMCNextStateGenerator<ValueType, StateType>::getNumberOfRewardModels() const {
            return rewardModels.size();
        }

        template<typename ValueType, typename StateType>
        storm::builder::RewardModelInformation InfCTMCNextStateGenerator<ValueType, StateType>::getRewardModelInformation(uint64_t const& index) const {
            storm::prism::RewardModel const& rewardModel = rewardModels[index].get();
            return storm::builder::RewardModelInformation(rewardModel.getName(), rewardModel.hasStateRewards(), rewardModel.hasStateActionRewards(), rewardModel.hasTransitionRewards());
        }

        template<typename ValueType, typename StateType>
        std::shared_ptr<storm::storage::sparse::ChoiceOrigins> InfCTMCNextStateGenerator<ValueType, StateType>::generateChoiceOrigins(std::vector<boost::any>& dataForChoiceOrigins) const {
            if (!this->getOptions().isBuildChoiceOriginsSet()) {
                return nullptr;
            }

            std::vector<uint_fast64_t> identifiers;
            identifiers.reserve(dataForChoiceOrigins.size());

            std::map<CommandSet, uint_fast64_t> commandSetToIdentifierMap;
            // The empty commandset (i.e., the choices without origin) always has to get identifier getIdentifierForChoicesWithNoOrigin() -- which is assumed to be 0
            STORM_LOG_ASSERT(storm::storage::sparse::ChoiceOrigins::getIdentifierForChoicesWithNoOrigin() == 0, "The no origin identifier is assumed to be zero");
            commandSetToIdentifierMap.insert(std::make_pair(CommandSet(), 0));
            uint_fast64_t currentIdentifier = 1;
            for (boost::any& originData : dataForChoiceOrigins) {
                STORM_LOG_ASSERT(originData.empty() || boost::any_cast<CommandSet>(&originData) != nullptr, "Origin data has unexpected type: " << originData.type().name() << ".");

                CommandSet currentCommandSet = originData.empty() ? CommandSet() : boost::any_cast<CommandSet>(std::move(originData));
                auto insertionRes = commandSetToIdentifierMap.insert(std::make_pair(std::move(currentCommandSet), currentIdentifier));
                identifiers.push_back(insertionRes.first->second);
                if (insertionRes.second) {
                    ++currentIdentifier;
                }
            }

            std::vector<CommandSet> identifierToCommandSetMapping(currentIdentifier);
            for (auto const& setIdPair : commandSetToIdentifierMap) {
                identifierToCommandSetMapping[setIdPair.second] = setIdPair.first;
            }

            return std::make_shared<storm::storage::sparse::PrismChoiceOrigins>(std::make_shared<storm::prism::Program>(program), std::move(identifiers), std::move(identifierToCommandSetMapping));
        }


        template class InfCTMCNextStateGenerator<double>;

#ifdef STORM_HAVE_CARL
        // template class InfCTMCNextStateGenerator<storm::RationalNumber>;
        // template class InfCTMCNextStateGenerator<storm::RationalFunction>;
#endif
    }
}
