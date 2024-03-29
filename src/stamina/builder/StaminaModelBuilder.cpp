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

/**
* Implementation for StaminaModelBuilder methods.
*
* Created by Josh Jeppson on 8/17/2021
* */
#include "StaminaModelBuilder.h"
#include "core/StaminaMessages.h"
#include "core/StateSpaceInformation.h"

#include "builder/threads/ControlThread.h"
#include "builder/threads/ExplorationThread.h"

#include <functional>
#include <sstream>
#include <algorithm>

namespace stamina {
namespace builder {

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
	, storm::prism::Program const& modulesFile
	, storm::generator::NextStateGeneratorOptions const & options
) : generator(generator)
	, stateStorage(*(new storm::storage::sparse::StateStorage<
		StateType
		>(generator->getStateSize())))
	, absorbingWasSetUp(false)
	, fresh(true)
	, firstIteration(true)
	, localKappa(core::Options::kappa)
	, numberTerminal(0)
	, iteration(0)
	, leftPropertyExpression(nullptr)
	, rightPropertyExpression(nullptr)
	, formulaMatchesExpression(true)
	, modulesFile(modulesFile)
	, options(options)
	, terminalStateToIdCallback(
		std::bind(
			&StaminaModelBuilder<ValueType, RewardModelType, StateType>::getStateIndexOrAbsorbing
			, this
			, std::placeholders::_1
		)
	)
	, currentProbabilityState(nullptr)
	, isInit(true)
	, isCtmc(true)
	, numberStates(0)
	, numberTransitions(0)
	, currentRow(0)
	, currentRowGroup(0)
	, hasAbsorbingTransitions(false)
{
	// Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& generatorOptions
) : StaminaModelBuilder( // Invoke other constructor
	std::make_shared<storm::generator::PrismNextStateGenerator<ValueType, StateType>>(program, generatorOptions)
	, program
	, generatorOptions
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
	return nullptr;
}

template <typename ValueType, typename RewardModelType, typename StateType>
std::vector<StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getPerimeterStates() {
	std::vector<StateType> perimeterStates = stateMap.getPerimeterStates();
	return perimeterStates;
}

template <typename ValueType, typename RewardModelType, typename StateType>
std::vector<ProbabilityState<StateType> *>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getPerimeterStatesAsProbabilityStates() {
	return stateMap.getPerimeterStatesAsProbStates();
}

template <typename ValueType, typename RewardModelType, typename StateType>
double
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getLocalKappa() {
	return localKappa;
}

template <typename ValueType, typename RewardModelType, typename StateType>
StateType
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex(
	CompressedState const& state
) {
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

	stateStorage.stateToId.findOrAdd(
		state
		, actualIndex
	);

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
StaminaModelBuilder<ValueType, RewardModelType, StateType>::flushToTransitionMatrix(storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder) {
	for (StateType row = 0; row < transitionsToAdd.size(); ++row) {
		if (transitionsToAdd[row].empty() && row != 0) {
			// This state is deadlock
			StaminaMessages::errorAndExit("State " + std::to_string(row) + " did not have any successive transitions!");
			// transitionMatrixBuilder.addNextValue(row, row, 1);
		}
		else {
			for (TransitionInfo & tInfo : transitionsToAdd[row]) {
				if (tInfo.transition == 0.0) {
					continue;
				}
				transitionMatrixBuilder.addNextValue(tInfo.from, tInfo.to, tInfo.transition);
			}
		}
		// Remove all transitions to the absorbing state
		transitionsToAdd[row].erase(
			std::remove_if(
				transitionsToAdd[row].begin()
				, transitionsToAdd[row].end()
				, [&](TransitionInfo t) {
					return t.to == 0;
				}
			)
			, transitionsToAdd[row].end()
		);

	}
	// transitionsToAdd.clear();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::createTransition(StateType from, StateType to, ValueType probability) {
	if (probability == 0) {
		StaminaMessages::warning("Will not create transition of probability 0 from state " + std::to_string(from) + " to " + std::to_string(to));
		return;
	}
	TransitionInfo tInfo(from, to, probability);
	// Create an element for both from and to
	while (transitionsToAdd.size() <= std::max(from, to)) {
		transitionsToAdd.push_back(std::vector<TransitionInfo>());
	}
	numberTransitions++;
#ifdef STAMINA_CHECK_TRANSITION_LIST
	// Quick check
	for (auto & trans : transitionsToAdd[from]) {
		if (trans.from != from) {
			StaminaMessages::errorAndExit("Transition list is malformed!");
		}
		if (trans.to == to) {
			StaminaMessages::warning("Attempting to create transition to a state there is already a transition to!\n\tFrom: " + std::to_string(from) + " To: " + std::to_string(to) + " Rates: " + std::to_string(probability) + " / " + std::to_string(trans.transition) );
			if (trans.transition != probability) {
				StaminaMessages::error("The transitions should have the same probability but do not!");
			}
			// trans.transition += probability;
			return;
		}
	}
#else
	// We will spot check the transition before this one
	int numberTransitionsForCurrentState = transitionsToAdd[from].size();
	if (numberTransitionsForCurrentState > 0) {
		auto & lastTransition = transitionsToAdd[from][numberTransitionsForCurrentState - 1];
		if (lastTransition.to == to && lastTransition.from == from && lastTransition.transition == probability) {
			return;
		}
	}
#endif // STAMINA_CHECK_TRANSITION_LIST
	// auto & it = tra
	transitionsToAdd[from].push_back(tInfo);
	// transitionsToAdd[from].sort(); // TODO: Change
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::createTransition(
	typename StaminaModelBuilder<ValueType, RewardModelType, StateType>::TransitionInfo transitionInfo
) {
	if (transitionInfo.transition == 0) {
		StaminaMessages::warning("Will not create transition of probability 0 from state " + std::to_string(transitionInfo.from) + " to " + std::to_string(transitionInfo.to));
		return;
	}
	numberTransitions++;
	// Create an element for both from and to
	while (transitionsToAdd.size() <= std::max(transitionInfo.from, transitionInfo.to)) {
		transitionsToAdd.push_back(std::vector<TransitionInfo>());
	}
	transitionsToAdd[transitionInfo.from].push_back(transitionInfo);

}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::printStateSpaceInformation() {
	StaminaMessages::info("Finished state space truncation.\n\tExplored " + std::to_string(numberStates) + " states in total.\n\tGot " + std::to_string(numberTransitions) + " transitions.");
}



template <typename ValueType, typename RewardModelType, typename StateType>
storm::models::sparse::StateLabeling
StaminaModelBuilder<ValueType, RewardModelType, StateType>::buildStateLabeling() {
	auto labeling = generator->label(stateStorage, stateStorage.initialStateIndices, stateStorage.deadlockStateIndices);
	labeling.addLabel("Absorbing");
	labeling.addLabelToState("Absorbing", 0);
	return labeling;
}

template <typename ValueType, typename RewardModelType, typename StateType>
double
StaminaModelBuilder<ValueType, RewardModelType, StateType>::accumulateProbabilities() {
	double totalProbability = numberTerminal * localKappa;
	// Reduce kappa
	localKappa /= core::Options::reduce_kappa;
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
	if (firstIteration) {
		this->absorbingState = CompressedState(generator->getVariableInformation().getTotalBitOffset(true)); // CompressedState(64);
		// Set all values to 1
		for (uint_fast64_t i = 0; i < generator->getVariableInformation().getTotalBitOffset(true); i++) {
			this->absorbingState.set(i);
		}
		// Add index 0 to deadlockstateindecies because the absorbing state is in deadlock
		stateStorage.deadlockStateIndices.push_back(0);
		// Check if state is already registered
		auto actualIndexPair = stateStorage.stateToId.findOrAddAndGetBucket(absorbingState, 0);

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
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::reset() {
	if (fresh) {
		return;
	}
	statesToExplore.clear(); // = StatePriorityQueue();
	// exploredStates.clear(); // States explored in our current iteration
	// API reset
	// if (stateRemapping) { stateRemapping->clear(); }
	// stateStorage = storm::storage::sparse::StateStorage<StateType>(generator->getStateSize());
	absorbingWasSetUp = false;
}


template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::setGenerator(
	std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> generator
) {
	this->generator = generator;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::setLocalKappaToGlobal() {
	core::Options::kappa = localKappa;
}

template <typename ValueType, typename RewardModelType, typename StateType>
uint8_t
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getIteration() {
	return iteration;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::connectTerminalStatesToAbsorbing(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
	, CompressedState const & terminalState
	, StateType stateId
	, std::function<StateType (CompressedState const&)> stateToIdCallback
) {
	bool addedValue = false;
	generator->load(terminalState);
	storm::generator::StateBehavior<ValueType, StateType> behavior = generator->expand(stateToIdCallback);
	// If there is no behavior, we have an error.
	if (behavior.empty()) {
#if defined DIE_ON_DEADLOCK
		StaminaMessages::errorAndExit("Behavior for perimeter state (id = " + std::to_string(stateId) + ") was empty!");
#elif defined WARN_ON_DEADLOCK
		StaminaMessages::warning("Behavior for perimeter state (id = " + std::to_string(stateId) + ") was empty!");
#endif // DIE_ON_DEADLOCK
		stateStorage.deadlockStateIndices.push_back(stateId);
		createTransition(stateId, stateId, 1.0); // Create Self-loop
		return;
	}
	hasAbsorbingTransitions = true;
	bool firstChoice = true;
	for (auto const& choice : behavior) {
		if (!firstChoice) {
			StaminaMessages::errorAndExit("Model should be deterministic! Got multiple choices in behavior for state " + std::to_string(stateId));
		}
		double totalRateToAbsorbing = 0;
		for (auto const& stateProbabilityPair : choice) {
			if (stateProbabilityPair.first != 0) {
				// row, column, value
				createTransition(stateId, stateProbabilityPair.first, stateProbabilityPair.second);
			}
			else {
				totalRateToAbsorbing += stateProbabilityPair.second;
			}
		}
		addedValue = true;
		// Absorbing state. We wrap it in this if statement to not create useless transitions
		// In case the perimeter state just loops back into all existing states
		if (totalRateToAbsorbing != 0) {
			createTransition(stateId, 0, totalRateToAbsorbing);
		}
		firstChoice = false;
	}
	if (!addedValue) {
		StaminaMessages::errorAndExit("Did not add to transition matrix!");
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
std::shared_ptr<storm::expressions::Expression>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getPropertyExpression() {
	return rightPropertyExpression;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::setPropertyFormula(
	std::shared_ptr<const storm::logic::Formula> formula
	, const storm::prism::Program & modulesFile
) {
	formulaMatchesExpression = false;
	std::shared_ptr<const storm::logic::ProbabilityOperatorFormula> formulaProb
		= std::static_pointer_cast<const storm::logic::ProbabilityOperatorFormula>(formula);
	const storm::logic::BoundedUntilFormula & pathFormula
		= static_cast<const storm::logic::BoundedUntilFormula &>(formulaProb->getSubformula());
	propertyFormula = std::make_shared<storm::logic::BoundedUntilFormula>(pathFormula);
	this->expressionManager = &modulesFile.getManager();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::loadPropertyExpressionFromFormula() {
	if (formulaMatchesExpression) {
		return;
	}
	// If we are called here, we assume that core::Options::no_prop_refine is false
	const storm::logic::StateFormula & leftStateFormula
		= static_cast<const storm::logic::StateFormula &>(propertyFormula->getLeftSubformula());
	const storm::logic::StateFormula & rightStateFormula
		= static_cast<const storm::logic::StateFormula &>(propertyFormula->getRightSubformula());
	// Make sure they are state formulas
	if (!(leftStateFormula.isAtomicExpressionFormula()
			|| leftStateFormula.isBinaryBooleanStateFormula()
			|| leftStateFormula.isBooleanLiteralFormula()
			|| leftStateFormula.isUnaryBooleanStateFormula()
		)
	) {
		StaminaMessages::warning("Left sub formula should have been state formula. Cannot do property based refinement!");
		return;
	}
	std::shared_ptr<storm::expressions::Expression> leftExpression(
		// Invoke copy constructor
		new storm::expressions::Expression(
			leftStateFormula.toExpression(
				// Expression manager
				*(this->expressionManager)
			)
		)
	);
	leftPropertyExpression = leftExpression;
	if (!(rightStateFormula.isAtomicExpressionFormula()
		|| rightStateFormula.isBinaryBooleanStateFormula()
		|| rightStateFormula.isBooleanLiteralFormula()
		|| rightStateFormula.isUnaryBooleanStateFormula()
	)
	) {
		StaminaMessages::warning("Right sub formula should have been state formula. Cannot do property based refinement!");
		return;
	}
	std::shared_ptr<storm::expressions::Expression> rightExpression(
		// Invoke copy constructor
		new storm::expressions::Expression(
			rightStateFormula.toExpression(
				// Expression manager
				*(this->expressionManager)
			)
		)
	);
	rightPropertyExpression = rightExpression;
	// Set this flag so that we know we've already done it.
	formulaMatchesExpression = true;
}

template <typename ValueType, typename RewardModelType, typename StateType>
storm::storage::sparse::StateStorage<StateType> &
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getStateStorage() const {
	return this->stateStorage;
}

template <typename ValueType, typename RewardModelType, typename StateType>
std::vector<typename threads::ExplorationThread<ValueType, RewardModelType, StateType> *> const &
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getExplorationThreads() const {
	StaminaMessages::warning("Using base class implementation of getExplorationThreads()! This is almost certainly a mistake!");
	return explorationThreads;
}

template <typename ValueType, typename RewardModelType, typename StateType>
util::StateMemoryPool<ProbabilityState<StateType>> &
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getMemoryPool() {
	return memoryPool;
}

template <typename ValueType, typename RewardModelType, typename StateType>
std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getGenerator() {
	return generator;
}

template <typename ValueType, typename RewardModelType, typename StateType>
util::StateIndexArray<StateType, ProbabilityState<StateType>> &
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getStateMap() {
	return this->stateMap;
}

template <typename ValueType, typename RewardModelType, typename StateType>
CompressedState &
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getAbsorbingState() {
	return this->absorbingState;
}

template <typename ValueType, typename RewardModelType, typename StateType>
uint64_t
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getStateCount() {
	return static_cast<uint64_t>(stateStorage.getNumberOfStates());
}

template <typename ValueType, typename RewardModelType, typename StateType>
uint64_t
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getTransitionCount() {
	return numberTransitions;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::printTransitionActions() {
	if (Options::export_trans == "") {
		StaminaMessages::warning("Transition file is empty! Defaulting to \"export.tra\"");
		Options::export_trans = "export.tra";
	}
	if (transitionsToAdd.size() == 0) {
		StaminaMessages::error("Cannot call printTransitionActions() AFTER model checking!");
	}
	std::ofstream out(Options::export_trans);
	for (auto & transitionBucket : transitionsToAdd) {
		for (auto & transition : transitionBucket) {
			out << transition.from << " " << transition.to << " " << transition.transition << std::endl;
		}
	}
	out.close();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::purgeAbsorbingTransitions() {
	if (!hasAbsorbingTransitions) {
		return;
	}
	for (auto & transitionBucket : transitionsToAdd) {
		for (auto & transition : transitionBucket) {
			if (transition.to == 0) {
				transition.transition = 0.0;
			}
		}
	}
	hasAbsorbingTransitions = false;
}

// Explicitly instantiate the class.
template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

template <typename StateType>
bool set_contains(std::unordered_set<StateType> current_set, StateType value) {
	auto search = current_set.find(value);
	return (search != current_set.end());
}

} // namespace builder
} // namespace stamina
