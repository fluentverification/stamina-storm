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
	, propertyExpression(nullptr)
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
		if (transitionsToAdd[row].empty()) {
			// This state is deadlock
			transitionMatrixBuilder.addNextValue(row, row, 1);
		}
		else {
			for (TransitionInfo tInfo : transitionsToAdd[row]) {
				transitionMatrixBuilder.addNextValue(row, tInfo.to, tInfo.transition);
			}
		}
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::createTransition(StateType from, StateType to, ValueType probability) {
	TransitionInfo tInfo(from, to, probability);
	// Create an element for both from and to
	while (transitionsToAdd.size() <= std::max(from, to)) {
		transitionsToAdd.push_back(std::vector<TransitionInfo>());
	}
	transitionsToAdd[from].push_back(tInfo);
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::createTransition(
	typename StaminaModelBuilder<ValueType, RewardModelType, StateType>::TransitionInfo transitionInfo
) {
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
	return generator->label(stateStorage, stateStorage.initialStateIndices, stateStorage.deadlockStateIndices);
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
		bool gotVar = false;
		for (auto variable : generator->getVariableInformation().booleanVariables) {
			if (variable.getName() == "Absorbing") {
				this->absorbingState.setFromInt(variable.bitOffset + 1, 1, 1);
				if (this->absorbingState.getAsInt(variable.bitOffset + 1, 1) != 1) {
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
	, CompressedState & terminalState
	, StateType stateId
	, std::function<StateType (CompressedState const&)> stateToIdCallback
) {
	bool addedValue = false;
	generator->load(terminalState);
	storm::generator::StateBehavior<ValueType, StateType> behavior = generator->expand(stateToIdCallback);
	// If there is no behavior, we have an error.
	if (behavior.empty()) {
		StaminaMessages::warning("Behavior for perimeter state was empty!");
		return;
	}
	for (auto const& choice : behavior) {
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
		// Absorbing state
		createTransition(stateId, 0, totalRateToAbsorbing);
	}
	if (!addedValue) {
		StaminaMessages::errorAndExit("Did not add to transition matrix!");
	}
}

template <typename ValueType, typename RewardModelType, typename StateType>
storm::expressions::Expression *
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getPropertyExpression() {
	return propertyExpression;
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::setPropertyFormula(
	std::shared_ptr<const storm::logic::Formula> formula
	, const storm::prism::Program & modulesFile
) {
	formulaMatchesExpression = false;
	propertyFormula = formula;
	this->expressionManager = &modulesFile.getManager();
}

template <typename ValueType, typename RewardModelType, typename StateType>
void
StaminaModelBuilder<ValueType, RewardModelType, StateType>::loadPropertyExpressionFromFormula() {
	if (formulaMatchesExpression) {
		return;
	}
	// If we are called here, we assume that core::Options::no_prop_refine is false
	std::shared_ptr<storm::expressions::Expression> pExpression(
		// Invoke copy constructor
		new storm::expressions::Expression(
			propertyFormula->toExpression(
				// Expression manager
				*(this->expressionManager)
			)
		)
	);
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

// Explicitly instantiate the class.
template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

template <typename StateType>
bool set_contains(std::unordered_set<StateType> current_set, StateType value) {
	auto search = current_set.find(value);
	return (search != current_set.end());
}

} // namespace builder
} // namespace stamina
