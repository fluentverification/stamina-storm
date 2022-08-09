/**
* Stamina Model Builder Class
* Created by Josh Jeppson on 8/17/2021
*
* If you look closely, you'll see similarities to storm::builder::ExplicitModelBuilder.
* However, since STAMINA has grown and morphed, stamina::builder::StaminaModelBuilder and
* the STORM builder originally referenced, storm::builder::ExplicitModelBuilder have
* diverged substantially. The main difference now being that stamina::builder::StaminaModelBuilder,
* and all of its child classes, now support multithreading and asynchronous state exploration,
* in addition to their original purpose of conditional exploration based on estimated state
* reachability.
* */

#ifndef STAMINA_BUILDER_STAMINAMODELBUILDER_H
#define STAMINA_BUILDER_STAMINAMODELBUILDER_H

#include <memory>
#include <utility>
#include <vector>
#include <deque>
#include <queue>
#include <cstdint>
#include <functional>

#include "core/Options.h"
#include "core/StaminaMessages.h"

#include "util/StateIndexArray.h"
#include "util/StateMemoryPool.h"

#include "builder/threads/BaseThread.h"

#include "builder/ProbabilityState.h"
#include "builder/StateAndTransitions.h"

#include <boost/functional/hash.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>

#include "__storm_needed_for_builder.h"

// Frequency for info/debug messages in terms of number of states explored.
#define MSG_FREQUENCY 100000
// #define MSG_FREQUENCY 4000

namespace stamina {
	namespace builder {

		using namespace storm::builder;
		using namespace storm::utility::prism;
		using namespace storm::generator;

		using namespace core;

		typedef storm::models::sparse::Ctmc<double> Ctmc;
		typedef storm::modelchecker::SparseCtmcCslModelChecker<Ctmc> CtmcModelChecker;

		template <typename ValueType, typename RewardModelType, typename StateType>
		class StaminaModelBuilder {
		public:

			typedef StaminaStateAndThreadIndex<StateType> StateThreadIndex;
			typedef StaminaTransitionInfo<StateType> TransitionInfo;
			typedef StaminaTransitionInfoComparison<StateType> TransitionInfoComparison;
			/**
			* Constructs a StaminaModelBuilder with a given storm::generator::PrismNextStateGenerator
			*
			* @param generator The generator we are going to use.
			* */
			StaminaModelBuilder(
				std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
				, storm::prism::Program const& modulesFile
				, storm::generator::NextStateGeneratorOptions const & options
			);
			/**
			* Constructs a StaminaModelBuilder with a PRISM program and generatorOptions
			*
			* @param program The PRISM program we are going to use to build the model with.
			* @param generatorOptions Options for the storm::generator::PrismNextStateGenerator we are going to use.
			* */
			StaminaModelBuilder(
				storm::prism::Program const& program
				, storm::generator::NextStateGeneratorOptions const& generatorOptions
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
			void printStateSpaceInformation();
			storm::expressions::Expression * getPropertyExpression();
			/**
			* Sets the property formula for state space truncation optimization. Does not load
			* or create an expression from the formula.
			*
			* @param formula The formula to set
			* @param modulesFile The modules file which contains the expressionmanager
			* */
			void setPropertyFormula(
				std::shared_ptr<const storm::logic::Formula> formula
				, const storm::prism::Program & modulesFile
			);
			/**
			* Gets the state ID of a current state, or adds it to the internal state storage. Performs state exploration
			* and state space truncation from that state.
			*
			* @param state Pointer to the state we are looking it
			* @return A pair with the state id and whether or not it was already discovered
			* */
			virtual StateType getOrAddStateIndex(CompressedState const& state);
			/**
			* Alterate state ID grabber. Returns state ID if exists. If it does not, returns the absorbing state
			* This is used as an alternative callback function for terminal (perimeter) states
			* */
			StateType getStateIndexOrAbsorbing(CompressedState const& state);
			double getLocalKappa();
			uint8_t getIteration();
			util::StateMemoryPool<ProbabilityState<StateType>> & getMemoryPool();
			std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> getGenerator();
			storm::storage::sparse::StateStorage<StateType> getStateStorage() const;
			std::vector<std::shared_ptr<threads::ExplorationThread<ValueType, RewardModelType, StateType>>> const & getExplorationThreads() const;
			/**
			 * Inserts a TransitionInfo into transitionsToAdd. This method must NOT be called
			 * after flushToTransitionMatrix has cleared transitionsToAdd
			 * */
			void createTransition(StateType from, StateType to, ValueType probability);
			void createTransition(TransitionInfo transitionInfo);

		protected:
			/**
			* Creates and loads the property expression from the formula
			* */
			void loadPropertyExpressionFromFormula();
			/**
			* Connects all terminal states to the absorbing state
			* */
			void connectTerminalStatesToAbsorbing(
				storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
				, CompressedState & terminalState
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
			virtual void buildMatrices(
				storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
				, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
				, StateAndChoiceInformationBuilder& choiceInformationBuilder
				, boost::optional<storm::storage::BitVector>& markovianChoices
				, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
			) = 0;
			/**
			 * Flushes the elements in transitionsToAdd into the transition matrix
			 *
			 * @param transitionMatrixBuilder The transition matrix builder
			 * */
			void flushToTransitionMatrix(storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder);
			/**
			* Explores state space and truncates the model
			*
			* @return The components of the truncated model
			* */
			virtual storm::storage::sparse::ModelComponents<ValueType, RewardModelType> buildModelComponents() = 0;
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

			/* Data Members */
			std::shared_ptr<threads::ControlThread<ValueType, RewardModelType, StateType>> controlThread;
			std::vector<std::shared_ptr<threads::ExplorationThread<ValueType, RewardModelType, StateType>>> explorationThreads;

			std::function<StateType (CompressedState const&)> terminalStateToIdCallback;

			storm::expressions::Expression * propertyExpression;
			storm::expressions::ExpressionManager * expressionManager;
			std::shared_ptr<const storm::logic::Formula> propertyFormula;

			std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> generator;

			util::StateMemoryPool<ProbabilityState<StateType>> memoryPool;

			std::deque<std::pair<ProbabilityState<StateType> *, CompressedState> > statesToExplore;

			// The following data members must be accessible to threads
			util::StateIndexArray<StateType, ProbabilityState<StateType>> stateMap;
			storm::storage::sparse::StateStorage<
				StateType
			>& stateStorage;

			// Remapping (not used by STAMINA)
			boost::optional<std::vector<uint_fast64_t>> stateRemapping;

			// Transitions which we must add
			std::vector<std::vector<TransitionInfo>> transitionsToAdd;
			// Options for next state generators
			storm::generator::NextStateGeneratorOptions const & options;
			// The model builder must have access to this to create a fresh next state generator each iteration
			storm::prism::Program const& modulesFile;
			ProbabilityState<StateType> * currentProbabilityState;
			CompressedState absorbingState;
			bool absorbingWasSetUp;
			bool isInit;
			bool fresh;
			uint8_t iteration;
			bool firstIteration;
			double localKappa;
			bool isCtmc;
			bool formulaMatchesExpression;
			uint64_t numberTerminal;
			uint64_t numberStates;
			uint64_t numberTransitions;
			uint_fast64_t currentRowGroup;
			uint_fast64_t currentRow;

		};

		// Helper method to find in unordered_set
		template <typename StateType>
		bool set_contains(std::unordered_set<StateType> current_set, StateType value);
	}
}
#endif // STAMINA_BUILDER_STAMINAMODELBUILDER_H
