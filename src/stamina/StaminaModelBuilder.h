/**
* Stamina Model Builder Class
* Created by Josh Jeppson on 8/17/2021
*
* If you look closely, you'll see this is fairly similar to storm::builder::ExplicitModelBuilder
* */
#ifndef STAMINAMODELBUILDER_H
#define STAMINAMODELBUILDER_H

#include <memory>
#include <utility>
#include <vector>
#include <deque>
#include <queue>
#include <cstdint>
#include <functional>

#include "Options.h"
#include "StaminaMessages.h"

#include <boost/functional/hash.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>

#include "storm/builder/ExplicitModelBuilder.h"
#include "storm/models/sparse/StandardRewardModel.h"

#include "storm/api/storm.h"
#include "storm-parsers/parser/PrismParser.h"
#include "storm-parsers/api/storm-parsers.h"
#include "storm/api/properties.h"
#include "storm/storage/jani/Property.h"
#include "storm/modelchecker/results/CheckResult.h"
#include "storm/modelchecker/results/ExplicitQuantitativeCheckResult.h"
#include "storm/utility/initialize.h"
#include "storm-parsers/api/properties.h"
#include "storm/models/sparse/Ctmc.h"
#include "storm/modelchecker/csl/SparseCtmcCslModelChecker.h"
#include "storm/modelchecker/csl/helper/SparseCtmcCslHelper.h"
#include "storm/storage/expressions/Expression.h"
#include "storm/storage/expressions/BaseExpression.h"
#include "storm/storage/expressions/BinaryExpression.h"
#include "storm/storage/expressions/BinaryRelationExpression.h"
#include "storm/storage/expressions/VariableExpression.h"
#include "storm/storage/expressions/ExpressionManager.h"
#include "storm/storage/expressions/UnaryBooleanFunctionExpression.h"
#include "storm/storage/expressions/BinaryBooleanFunctionExpression.h"
#include "storm/logic/Formula.h"
#include "storm/storage/prism/Constant.h"
#include "storm/settings/SettingsManager.h"
#include "storm/storage/expressions/Valuation.h"
#include "storm/environment/Environment.h"
#include "storm/modelchecker/results/CheckResult.h"
#include "storm/builder/BuilderOptions.h"
#include "storm/generator/VariableInformation.h"

namespace stamina {

	using namespace storm::builder;
	using namespace storm::utility::prism;
	using namespace storm::generator;

	typedef storm::models::sparse::Ctmc<double> Ctmc;
	typedef storm::modelchecker::SparseCtmcCslModelChecker<Ctmc> CtmcModelChecker;

	template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
	class StaminaModelBuilder {
	public:
		/* Sub-class for states with probabilities */
		class ProbabilityState {
		public:
			CompressedState const state;
			StateType const index;
			bool enqueued;
			ProbabilityState(
				CompressedState state
				, StateType index
				, double pi = 0.0
				, bool terminal = true
				, bool enqueued = true
			) : state(state)
				, index(index)
				, pi(pi)
				, terminal(terminal)
				, enqueued(enqueued)
			{
				// Intentionally left empty
			}
			double getPi() {
				return pi;
			}
			void addToPi(double add) {
				pi += add;
			}
			void setPi(double pi) {
				this->pi = pi;
			}
			bool isTerminal() {
				return terminal;
			}
			void setTerminal(bool term) {
				terminal = term;
			}
			inline bool operator==(const ProbabilityState & rhs) const {
				return index == rhs.index;
			}
			inline bool operator>=(const ProbabilityState & rhs) const {
				return index >= rhs.index;
			}
			inline bool operator<=(const ProbabilityState & rhs) const {
				return index <= rhs.index;
			}
			inline bool operator>(const ProbabilityState & rhs) const {
				return index > rhs.index;
			}
			inline bool operator<(const ProbabilityState & rhs) const {
				return index < rhs.index;
			}
		private:
			double pi;
			bool terminal;

		};

		class StatePriorityQueue {
		public:
			std::vector<std::shared_ptr<ProbabilityState>> stateQueue;
			StatePriorityQueue()
			{
				// Intentionally left empty
			}
			bool empty() {
				return stateQueue.empty();
			}
			std::shared_ptr<ProbabilityState> pop() {
				std::shared_ptr<ProbabilityState> front = stateQueue.front();
				stateQueue.erase(stateQueue.begin());
				return front;
			}
			void push(std::shared_ptr<ProbabilityState> state) {
				uint_fast32_t pos = stateQueue.size();
				while (pos > 0 && stateQueue[pos - 1]->index > state->index) {
					pos--;
				}
				stateQueue.insert(stateQueue.begin() + pos, state);
			}
		};

		/**
		* Constructs a StaminaModelBuilder with a given storm::generator::PrismNextStateGenerator
		*
		* @param generator The generator we are going to use.
		* */
		StaminaModelBuilder(
			std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
		);
		/**
		* Constructs a StaminaModelBuilder with a PRISM program and generatorOptions
		*
		* @param program The PRISM program we are going to use to build the model with.
		* @param generatorOptions Options for the storm::generator::PrismNextStateGenerator we are going to use.
		* */
		StaminaModelBuilder(
			storm::prism::Program const& program
			, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
		);
		/**
		* Creates a model with a truncated state space for the program provided during construction. State space
		* is truncated during this method using the STAMINA II truncation method described by Riley Roberts and Zhen
		* Zhang, and corresponding to the same algorithm used in the Java version of STAMINA.
		*
		* @return The truncated model.
		* */
		std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> build();
		/**
		 * Gets the state ID of a state known to already exist. This does NOT perform state-space truncation for future states
		 * */
		StateType getStateIndexIfKnown(CompressedState const& state);
		/**
		 * Accumulates all probabilities in T Map and returns
		 * */
		double accumulateProbabilities();
		/**
		 * Resets stuff to "fresh" state.
		 */
		void reset();
		/**
		 * Sets the generator while deleting the old one
		 * @param generator The new generator
		 */
		void setGenerator(
			std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> generator
		);
		/**
		 * Creates and returns a vector of all perimeter states
		 *
		 * @return a vector of all perimeter states
		 */
		 std::vector<StateType> getPerimeterStates();
		 /**
		  * Sets the value of &kappa; in Options to what we have stored locally here
		  * */
		void setLocalKappaToGlobal();
	protected:
		/**
		* Gets the state ID of a current state, or adds it to the internal state storage. Performs state exploration
		* and state space truncation from that state.
		*
		* @param state Pointer to the state we are looking it
		* @return A pair with the state id and whether or not it was already discovered
		* */
		StateType getOrAddStateIndex(CompressedState const& state);
		/**
		 * Alterate state ID grabber. Returns state ID if exists. If it does not, returns the absorbing state
		 * This is used as an alternative callback function for terminal (perimeter) states
		 * */
		StateType getStateIndexOrAbsorbing(CompressedState const& state);
		/**
		 * Connects all terminal states to the absorbing state
		 * */
		void connectTerminalStatesToAbsorbing(
			storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
			, CompressedState terminalState
			, StateType stateId
			, std::function<StateType (CompressedState const&)> stateToIdCallback
		);
		/**
		* Builds transition matrix of truncated state space for the given program.
		*
		* @param transitionMatrixBuilder The builder of the transition matrix.
		* @param rewardModelBuilders The builders for the selected reward models.
		* @param choiceInformationBuilder The builder for the requested information of the choices
		* @param markovianChoices is set to a bit vector storing whether a choice is Markovian (is only set if the model type requires this information).
		* @param stateValuationsBuilder if not boost::none, we insert valuations for the corresponding states
		* */
		void buildMatrices(
			storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
			, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
			, StateAndChoiceInformationBuilder& choiceInformationBuilder
			, boost::optional<storm::storage::BitVector>& markovianChoices
			, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
		);
		/**
		* Explores state space and truncates the model
		*
		* @return The components of the truncated model
		* */
		storm::storage::sparse::ModelComponents<ValueType, RewardModelType> buildModelComponents();
		/**
		* Builds state labeling for our program
		*
		* @return State labeling for our program
		* */
		storm::models::sparse::StateLabeling buildStateLabeling();
		/**
		 * Sets up the initial state in the transition matrix
		 * */
		void setUpAbsorbingState(
			storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
			, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
			, StateAndChoiceInformationBuilder& choiceInformationBuilder
			, boost::optional<storm::storage::BitVector>& markovianChoices
			, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
		);

	private:
		/* Data Members */
		storm::storage::sparse::StateStorage<StateType>& stateStorage;
		std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> generator;
		// std::deque<std::pair<CompressedState, StateType>> statesToExplore;
		StatePriorityQueue statesToExplore;
		boost::optional<std::vector<uint_fast64_t>> stateRemapping;
		std::unordered_set<StateType> exploredStates; // States that we have explored
		std::unordered_map<StateType, std::shared_ptr<ProbabilityState>> stateMap; // S in the QEST paper
		storm::storage::SparseMatrixBuilder<ValueType> transitionMatrixBuilder;
		std::shared_ptr<ProbabilityState> currentProbabilityState;
		CompressedState absorbingState;
		bool absorbingWasSetUp;
		bool isInit;
		bool fresh;
		bool firstIteration;
		double localKappa;
		bool isCtmc;
		uint64_t numberTerminal;
		uint_fast64_t currentRowGroup;
		uint_fast64_t currentRow;

	};

	// Helper method to find in unordered_set
	template <typename StateType>
	bool set_contains(std::unordered_set<StateType> current_set, StateType value);
}
#endif // STAMINAMODELBUILDER_H
