/**
 * A type of thread which maintains the following responsibilities:
 *     1. Keeps track of which thread owns a particular state
 *     2. Allows for asynchronous requests for ownership of new states
 *     3. Allows for unordered and asynchronous requests for transition matrix insertion
 *
 * Created by Josh Jeppson on Jul 8, 2022
 * */

#ifndef STAMINA_BUILDER_THREADS_CONTROLTHREAD_H
#define STAMINA_BUILDER_THREADS_CONTROLTHREAD_H

#include "BaseThread.h"

#include "stamina/builder/StaminaModelBuilder.h"

#include "storm/storage/BitVectorHashMap.h"

#include <deque>

namespace stamina {
	namespace builder {
		namespace threads {
			// The thread index of no exploration thread
			const uint8_t NO_THREAD = 0;

			// Forward-declare the exploration thread
			template <typename ValueType, typename RewardModelType, typename StateType>
			class ExplorationThread;

			// Forward declare StateAndProbability
			template <typename ValueType, typename RewardModelType, typename StateType>
			class StateAndProbability;

			// Forward declare StaminaTransitionInfo
			template <typename ValueType, typename RewardModelType, typename StateType>
			class StaminaTransitionInfo;

			template <typename ValueType, typename RewardModelType, typename StateType>
			class ControlThread : public BaseThread<ValueType, RewardModelType, StateType> {
			public:

				typedef StateAndProbability<ValueType, RewardModelType, StateType> StateProbability;
				typedef StaminaTransitionInfo<ValueType, RewardModelType, StateType> Transition;

				struct StateAndThreadIndex {
					StateAndThreadIndex(StateType state, uint8_t thread) : state(state), thread(thread) {
						// Intentionally left empty
					}
					StateType state; // State Index
					uint8_t thread; // Thread index
					// Cast to StateType
					operator StateType() const {
						return state;
					}
				};
				class LockableDeque {
				public:
					int size();
					/**
					 * Locks the queue and emplaces
					 * */
					void emplace_back(StateType from, StateType to, double rate);
					bool empty();
					void lockThread();
					void unlockThread();
					Transition top();
					void pop();
				private:
					std::deque<Transition> queue;
					std::shared_mutex lock;
				};

				/**
				* Constructor for ControlThread. Primarily just calls super class constructor
				*
				* @param parent The model builder who owns this thread
				* @param numberExplorationThreads The number of exploration threads who will
				* be using this worker thread.
				* */
				ControlThread(
					StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
					, uint8_t numberExplorationThreads
// 					, storm::storage::sparse::StateStorage<uint8_t>& stateThreadMap // State storage owned by parents
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
				* @param requestedId The (new) stateId that a thread can request we assign a state to
				* @return The thread who owns the state and the new state index
				* */
				std::pair<uint8_t, StateType> requestOwnership(CompressedState & state, uint8_t threadIndex, StateType requestedId = 0);
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
				 * Gets the index of a state which already exists. If the state does not
				 * exist, returns 0.
				 *
				 * @param state The state to look up
				 * @return The state index
				 * */
				StateType whatIsIndex(CompressedState & state);
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
				 * Requests cross exploration from a
				 *
				 * @param stateAndProbability A datastructure containing:
				 *     state The state to cross explore
				 *     deltaPi The difference in probability to add
				 *     stateIndex The state index we found
				 * @param threadIndex Thread index to request cross exploration from
				 * */
				void requestCrossExplorationFromThread(
					StateProbability stateAndProbability
					, double threadIndex
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
				storm::storage::sparse::StateStorage<StateAndThreadIndex>& getStateStorage();
				void terminate();
			private:
				std::vector<LockableDeque> transitionQueues;
				std::shared_mutex ownershipMutex;
				const uint8_t numberExplorationThreads;
				storm::storage::sparse::StateStorage<uint8_t>& stateThreadMap;
				const std::vector<ExplorationThread<ValueType, RewardModelType, StateType>> explorationThreads;
			};

		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_THREADS_CONTROLTHREAD_H
