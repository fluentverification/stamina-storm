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

#ifndef STAMINAMODELBUILDER_H
#define STAMINAMODELBUILDER_H

#include <memory>
#include <utility>
#include <vector>
#include <deque>
#include <queue>
#include <cstdint>
#include <functional>

#include "../Options.h"
#include "../StaminaMessages.h"
#include "../util/StateIndexArray.h"
#include "../util/StateMemoryPool.h"

#include <boost/functional/hash.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>

#include "__storm_needed_for_builder.h"

// Frequency for info/debug messages in terms of number of states explored.
#define MSG_FREQUENCY 100000
// #define MSG_FREQUENCY 4000

namespace stamina {
	namespace builder {

		// Forward-declare thread classes
		class BaseThread;
		class BookKeeperThread : public BaseThread;
		class ExplorationThread : public BaseThread;

		using namespace storm::builder;
		using namespace storm::utility::prism;
		using namespace storm::generator;

		typedef storm::models::sparse::Ctmc<double> Ctmc;
		typedef storm::modelchecker::SparseCtmcCslModelChecker<Ctmc> CtmcModelChecker;

		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		class StaminaModelBuilder {
		public:
// 			struct
			/* Sub-class for states with probabilities */
			class ProbabilityState {
			public:
				StateType index;
				bool assignedInRemapping;
				uint8_t iterationLastSeen;
				bool isNew;
				bool wasPutInTerminalQueue;
// 				ProbabilityState() : { /* Intentionally left empty */ }
				ProbabilityState(
					StateType index = 0
					, double pi = 0.0
					, bool terminal = true
					, uint8_t iterationLastSeen = 0
				) : index(index)
					, pi(pi)
					, terminal(terminal)
					, assignedInRemapping(false)
					, iterationLastSeen(iterationLastSeen)
					, isNew(true)
					, wasPutInTerminalQueue(false)
				{
					// Intentionally left empty
				}
				// Copy constructor
				ProbabilityState(const ProbabilityState & other)
					: index(other.index)
					, pi(other.pi)
					, terminal(other.terminal)
					, assignedInRemapping(other.assignedInRemapping)
					, isNew(other.isNew)
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
				double pi;
				bool terminal;

			};
			struct ProbabilityStateComparison {
				bool operator() (
					const ProbabilityState * first
					, const ProbabilityState * second
				) const {
					// Create a max heap on the reachability probability
					return first->pi < second->pi;
				}
			};

			class ProbabilityStatePair {
			public:
				ProbabilityState * first;
				CompressedState second;
			};

			struct ProbabilityStatePairComparison {
				bool operator() (
					const ProbabilityStatePair first
					, const ProbabilityStatePair second
				) const {
					// Create a max heap on the reachability probability
					return first.first->pi < second.first->pi;
				}
			};

			/**
			 * A basic struct for out of order transitions to insert into the transition matrix.
			 * This is faster than using the remapping and std::sort in the STORM API
			 * */
			class TransitionInfo {
			public:
				TransitionInfo(StateType to, ValueType transition) :
					to(to), transition(transition) { /* Intentionally left empty */ }
				StateType to;
				ValueType transition;
			};
			struct TransitionInfoComparison {
				bool operator() (
					const TransitionInfo * first
					, const TransitionInfo * second
				) const {
					return first->to > second->to;
				}
			};

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
			* Applies the remapping in the state remapping vector to the transition matrix
			*
			* @param transitionMatrixBuilder The transition matrix to apply it to.
			* */
			void remapStates(storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder);
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
			 * Inserts a TransitionInfo into transitionsToAdd. This method must NOT be called
			 * after flushToTransitionMatrix has cleared transitionsToAdd
			 * */
			void createTransition(StateType from, StateType to, ValueType probability);
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
			std::shared_ptr<BookKeeperThread> workerThread;
			std::vector<std::shared_ptr<ExplorationThread>> explorationThreads;

			std::function<StateType (CompressedState const&)> terminalStateToIdCallback;
			storm::expressions::Expression * propertyExpression;
			storm::expressions::ExpressionManager * expressionManager;
			std::shared_ptr<const storm::logic::Formula> propertyFormula;
			storm::storage::sparse::StateStorage<StateType>& stateStorage;
			std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> generator;
			util::StateMemoryPool<ProbabilityState> memoryPool;
			// StatePriorityQueue statesToExplore;
			std::deque<std::pair<ProbabilityState *, CompressedState> > statesToExplore;
			boost::optional<std::vector<uint_fast64_t>> stateRemapping;
			util::StateIndexArray<StateType, ProbabilityState> stateMap;
			// Transitions which we must add
			std::vector<std::vector<TransitionInfo>> transitionsToAdd;
			// Options for next state generators
			storm::generator::NextStateGeneratorOptions const & options;
			// The model builder must have access to this to create a fresh next state generator each iteration
			storm::prism::Program const& modulesFile;
			ProbabilityState * currentProbabilityState;
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

		/**
		 * Base class for all threads. Automatically constructs a thread which runs
		 * the mainLoop function.
		 * */
		template <typename StateType, typename ValueType>
		class BaseThread : public std::thread {
		public:
			/**
			 * Constructs a BaseThread
			 *
			 * @param parent The model builder who owns this thread
			 * */
			BaseThread(StaminaModelBuilder<ValueType, StateType=StateType> * parent);
			/**
			 * Pure virtual function for the main loop. When this function returns,
			 * the thread dies.
			 * */
			virtual void mainLoop() = 0;
			/**
			 * Creates and starts this thread in the background
			 * */
			void startThread();
			/**
			 * Gets the pointer to the model builder owning this thread
			 *
			 * @return This thread's parent
			 * */
			const StaminaModelBuilder<ValueType, StateType=StateType> * getParent();
		private:
			const StaminaModelBuilder<ValueType, StateType=StateType> * parent;
		};

		template <typename StateType, typename ValueType>
		class BookKeeperThread<StateType> : public BaseThread {
		public:
			/**
			 * Constructor for BookKeeperThread. Primarily just calls super class constructor
			 *
			 * @param parent The model builder who owns this thread
			 * @param numberExplorationThreads The number of exploration threads who will
			 * be using this worker thread.
			 * */
			BookKeeperThread(
				StaminaModelBuilder<ValueType, StateType=StateType> * parent
				, uint8_t numberExplorationThreads
			);
			/**
			 * Requests ownership of a state for a particular thread. This is intended
			 * to be called by the thread whose index matches the second parameter in
			 * this function. This function *locks the mutex* and so it should not be
			 * used if we just wish to ask who owns a particular state.
			 *
			 * If request ownership is successful, the return value is equal to
			 * the index of the state requesting ownership of the state. However, if it
			 * is not successful, then the return value gives the thread which potentially
			 * locked the mutex and got ownership first.
			 *
			 * @param state The state to request ownership for
			 * @param threadIndex The thread who wants ownership of the state.
			 * @return The thread who owns the state
			 * */
			uint8_t requestOwnership(CompressedState & state, uint8_t threadIndex);
			/**
			 * Gets the owning thread of a particular state without locking the mutex.
			 * This allows for threads to use the many-read, one-write idea put forth
			 * in the paper.
			 *
			 * @param state The state who we wonder if owns
			 * @return The thread who owns `state`
			 * */
			uint8_t whoOwns(CompressedState & state);
			/**
			 * Requests a transition to be inserted (not necessarily in order).
			 * These transitions are requested by the exploration threads and
			 * are flushed to the model builder's data structure on a "when available"
			 * basis, meaning that when this thread idles, it transfers these
			 * transitions. Additionally, it maintains mutexes for each thread and
			 * only locks the mutex for that particular thread to prevent mutex collisions
			 * on the different threads requesting transition insertion.
			 *
			 * @param thread The index of the thread making the request
			 * @param from The index of the state we are transitioning from
			 * @param to The index of the state we are transitioning to
			 * @param rate The transition rate (if CTMC) or transition probability (if DTMC)
			 * */
			void requestInsertTransition(
				uint8_t thread
				, StateType from
				, StateType to
				, double rate
			);
			/**
			 * This thread lives for the duration of all exploration threads. It waits for
			 * the exploration threads to all emit a "finished" signal, and then tells each
			 * exploration thread to die.
			 *
			 * The main loop for this thread also flushes things to the parents' transitionsToAdd,
			 * which is not locked or mutex'ed because there is only one worker thread to do that.
			 * */
			virtual void mainLoop() override;
			// TODO: Lockable queue
		private:
			std::shared_mutex ownershipMutex;
			const uint8_t numberExplorationThreads;
		};

		template <typename StateType, typename ValueType>
		class ExplorationThread<StateType, ValueType> : public BaseThread {
		public:
			typedef std::pair<CompressedState &, std::shared_ptr<StaminaModelBuilder<ValueType, StateType=StateType>> StateAndProbability;
			/**
			 * Constructor. Invokes super's constructor and stores the
			 * thread index which cannot change for the life of the thread
			 *
			 * @param parent The model builder who owns this thread
			 * @param index The index of this thread
			 * */
			ExplorationThread(
				StaminaModelBuilder<ValueType, StateType=StateType>> * parent
				, uint8_t index
			);
			uint8_t getIndex();
			uint32_t getNumberOfOwnedStates();
			bool isFinished();
			/**
			 * A function called by other threads to request cross exploration of
			 * states already explored but encountered by another thread.
			 *
			 * @param state The state to cross explore.
			 * @param deltaPi The difference in reachability to add to that
			 * state and push forward to its successors.
			 * */
			void requestCrossExploration(CompressedState & state, double deltaPi);
			/**
			 * Does state exploration or idles until worker thread asks to kill it.
			 * */
			virtual void mainLoop() override; // doExploration
		protected:
			virtual void exploreState(StateAndProbability & stateProbability) = 0;
			// Weak priority on crossExplorationQueue (superseded by mutex lock)
			std::shared_mutex crossExplorationQueueMutex;
			std::deque<std::pair<CompressedState, double deltaPi>> crossExplorationQueue;
			std::deque<StateAndProbability> mainExplorationQueue;
		private:
			const uint8_t index;
			uint32_t numberOfOwnedStates;
			bool finished;
		};

		// Helper method to find in unordered_set
		template <typename StateType>
		bool set_contains(std::unordered_set<StateType> current_set, StateType value);
	}
}
#endif // STAMINAMODELBUILDER_H
