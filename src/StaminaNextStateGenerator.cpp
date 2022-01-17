#include "StaminaNextStateGenerator.h"

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

/*
 * Not super happy I have to include this. A lot of these methods are really similar to the PrismNextStateGenerator,
 * from which it inherits, but I can't call them because what we need to do is *slightly* different and there are
 * private data members.
 * */

using namespace stamina;

template<typename ValueType, typename StateType>
StateBehavior<ValueType, StateType>
StaminaNextStateGenerator<ValueType, StateType>::expand(StateToIdCallback const& stateToIdCallback) {
	// Result to return
	StateBehavior<ValueType, StateType> result;

	// No need to construct state rewards as that's not the type of CTMC we deal with

	// Return empty result if terminal expression is set
	if (!this->terminalStates.empty()) {
		for (auto const& expression : this->terminalStates) {
			if (this->evaluator->asBool(expression.first) == expression.second) {
				return result;
			}
		}
	}

	// Indicate that the result has been expanded
	result.setExpanded();

	// Create a list of all choices
	std::vector<Choice<ValueType>> allChoices;
	if (this->getOptions().isApplyMaximalProgressAssumptionSet()) {
		// First explore only edges without a rate
		allChoices = getAsynchronousChoices(*this->state, stateToIdCallback, CommandFilter::Probabilistic);
		addSynchronousChoices(allChoices, *this->state, stateToIdCallback, CommandFilter::Probabilistic);
		if (allChoices.empty()) {
			// Expand the Markovian edges if there are no probabilistic ones.
			allChoices = getAsynchronousChoices(*this->state, stateToIdCallback, CommandFilter::Markovian);
			addSynchronousChoices(allChoices, *this->state, stateToIdCallback, CommandFilter::Markovian);
		}
	} else {
		allChoices = getAsynchronousChoices(*this->state, stateToIdCallback);
		addSynchronousChoices(allChoices, *this->state, stateToIdCallback);
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
// 		STORM_LOG_ASSERT(choiceIt->hasPlayerIndex(), "State '" << this->stateToString(*this->state) << "' features a choice without player index."); // This should have been catched while creating the choice already
		storm::storage::PlayerIndex statePlayerIndex = choiceIt->getPlayerIndex();
// 		STORM_LOG_ASSERT(statePlayerIndex != storm::storage::INVALID_PLAYER_INDEX, "State '" << this->stateToString(*this->state) << "' features a choice with invalid player index."); // This should have been catched while creating the choice already
		for (++choiceIt; choiceIt != allChoices.end(); ++choiceIt) {
// 			STORM_LOG_ASSERT(choiceIt->hasPlayerIndex(), "State '" << this->stateToString(*this->state) << "' features a choice without player index."); // This should have been catched while creating the choice already
// 			STORM_LOG_ASSERT(choiceIt->getPlayerIndex() != storm::storage::INVALID_PLAYER_INDEX, "State '" << this->stateToString(*this->state) << "' features a choice with invalid player index."); // This should have been catched while creating the choice already
			if (!(statePlayerIndex == choiceIt->getPlayerIndex(), storm::exceptions::WrongFormatException)) {
				std::cerr << "The player for state '" << this->stateToString(*this->state) << "' is not unique. At least one choice is owned by player '" << statePlayerIndex << "' while another is owned by player '" << choiceIt->getPlayerIndex() << "'.";
			}
		}
	}

	// Move all remaining choices in place.
	for (auto& choice : allChoices) {
		result.addChoice(std::move(choice));
	}

	this->postprocess(result);

	return result;

}

template<typename ValueType, typename StateType>
std::vector<Choice<ValueType>>
StaminaNextStateGenerator<ValueType, StateType>::getAsynchronousChoices(
	CompressedState const& state
	, StateToIdCallback stateToIdCallback
	, CommandFilter const& commandFilter = CommandFilter::All
) {
	std::vector<Choice<ValueType>> result;

	// Iterate over all modules.
	for (uint_fast64_t i = 0; i < program.getNumberOfModules(); ++i) {
		storm::prism::Module const& module = program.getModule(i);

		// Iterate over all commands.
		for (uint_fast64_t j = 0; j < module.getNumberOfCommands(); ++j) {
			storm::prism::Command const& command = module.getCommand(j);

			// Only consider commands that are not possibly synchronizing.
			if (isCommandPotentiallySynchronizing(command)) continue;

			if (commandFilter != CommandFilter::All) {
// 				STORM_LOG_ASSERT(commandFilter == CommandFilter::Markovian || commandFilter == CommandFilter::Probabilistic, "Unexpected command filter.");
				if ((commandFilter == CommandFilter::Markovian) != command.isMarkovian()) {
					continue;
				}
			}
			if (this->actionMask != nullptr) {
				if (!this->actionMask->query(*this, command.getActionIndex())) {
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
					CompressedState newState = applyUpdate(state, update);
					StateType newStateIndex = newState.second;
					// TODO: don't callback if we don't need to
					StateType stateIndex = stateToIdCallback(newState);

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

			if (this->options.isBuildChoiceLabelsSet() && command.isLabeled()) {
				choice.addLabel(program.getActionName(command.getActionIndex()));
			}

			if (program.getModelType() == storm::prism::Program::ModelType::SMG) {
				storm::storage::PlayerIndex const& playerOfModule = moduleIndexToPlayerIndexMap.at(i);
// 				STORM_LOG_THROW(playerOfModule != storm::storage::INVALID_PLAYER_INDEX, storm::exceptions::WrongFormatException, "Module " << module.getName() << " is not owned by any player but has at least one enabled, unlabeled command.");
				choice.setPlayerIndex(playerOfModule);
			}

			if (this->options.isExplorationChecksSet()) {
				// Check that the resulting distribution is in fact a distribution.
// 				STORM_LOG_THROW(!program.isDiscreteTimeModel() || this->comparator.isOne(probabilitySum), storm::exceptions::WrongFormatException, "Probabilities do not sum to one for command '" << command << "' (actually sum to " << probabilitySum << ").");
			}
		}
	}

	return result;
}

template<typename ValueType, typename StateType>
void
StaminaNextStateGenerator<ValueType, StateType>::addSynchronousChoices(
	std::vector<Choice<ValueType>>& choices
	, CompressedState const& state
	, StateToIdCallback stateToIdCallback
	, CommandFilter const& commandFilter = CommandFilter::All
) {
	for (uint_fast64_t actionIndex : program.getSynchronizingActionIndices()) {
		if (this->actionMask != nullptr) {
			if (!this->actionMask->query(*this, actionIndex)) {
				continue;
			}
		}
		boost::optional<std::vector<std::vector<std::reference_wrapper<storm::prism::Command const>>>> optionalActiveCommandLists = getActiveCommandsByActionIndex(actionIndex, commandFilter);

		// Only process this action label, if there is at least one feasible solution.
		if (optionalActiveCommandLists) {
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
// 					STORM_LOG_THROW(playerOfAction != storm::storage::INVALID_PLAYER_INDEX, storm::exceptions::WrongFormatException, "Action " << program.getActionName(actionIndex) << " is not owned by any player but has at least one enabled, unlabeled (synchronized) command.");
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
// 					STORM_LOG_THROW(!program.isDiscreteTimeModel() || !this->comparator.isConstant(probabilitySum) || this->comparator.isOne(probabilitySum), storm::exceptions::WrongFormatException, "Sum of update probabilities do not some to one for some command (actually sum to " << probabilitySum << ").");
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
}

template<typename ValueType, typename StateType>
void
StaminaNextStateGenerator<ValueType, StateType>::generateSynchronizedDistribution(
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
			generateSynchronizedDistribution(applyUpdate(state, update), probability * this->evaluator->asRational(update.getLikelihoodExpression()), position + 1, iteratorList, distribution, stateToIdCallback);
		}
	}
}

template<typename ValueType, typename StateType>
CompressedState
StaminaNextStateGenerator<ValueType, StateType>::applyUpdate(
	CompressedState const& state
	, storm::prism::Update const& update
) {

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
			if (!(assignedValue >= integerIt->lowerBound)) {
				throw storm::exceptions::WrongFormatException("The update " << update << " leads to an out-of-bounds value (" << assignedValue << ") for the variable '" << assignmentIt->getVariableName() << "'.");
			}
			if (!(assignedValue <= integerIt->upperBound)) {
				throw storm::exceptions::WrongFormatException("The update " << update << " leads to an out-of-bounds value (" << assignedValue << ") for the variable '" << assignmentIt->getVariableName() << "'.");
			}
		}
		newState.setFromInt(integerIt->bitOffset, integerIt->bitWidth, assignedValue - integerIt->lowerBound);
		if (!(static_cast<int_fast64_t>(newState.getAsInt(integerIt->bitOffset, integerIt->bitWidth)) + integerIt->lowerBound == assignedValue)) {
			std::cerr << "Writing to the bit vector bucket failed (read " << newState.getAsInt(integerIt->bitOffset, integerIt->bitWidth) << " but wrote " << assignedValue << ").";
		}
	}

	// Check that we processed all assignments.
	if (assignmentIt != assignmentIte) {
		std::cerr << "Not all assignments were consumed." << std::endl;
	}

	return newState;
}
