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

#include "stamina/builder/StateAndTransitions.h"

#include "stamina/builder/StaminaModelBuilder.h"
#include "stamina/builder/StateAndTransitions.h"

#include "storm/storage/BitVectorHashMap.h"

#include <deque>

namespace stamina {
	namespace builder {
		namespace threads {

			// Forward-declare the exploration thread
			template <typename ValueType, typename RewardModelType, typename StateType>
			class ExplorationThread;

			template <typename ValueType, typename RewardModelType, typename StateType>
			class ControlThread : public BaseThread<ValueType, RewardModelType, StateType> {
			public:

				typedef StateAndProbability<StateType> StateProbability;
				typedef StaminaTransitionInfo<StateType> Transition;
				typedef StaminaStateAndThreadIndex<StateType> StateAndThreadIndex;

				class LockableDeque {
				public:
					LockableDeque()
						// : guard(this->lock)
					{
						// Intentionally left empty
					}

					LockableDeque(const LockableDeque & other)
						// : guard(this->lock)
						: queue(other.queue)
					{
						// We don't need to copy the lock, just the queue
					}
					int size() const;
					/**
					 * Locks the queue and emplaces an element
					 *
					 * @param from The state we are going from
					 * @param to The state the transition goes to
					 * @param rate The transition rate of this transition
					 * */
					void emplace_back(StateType from, StateType to, double rate);
					/**
					 * Determines whether the internal queue is empty. This method is
					 * const as it does not change anything about the internals of this
					 * datastructure
					 *
					 * @return whether the internal queue is empty
					 * */
					bool empty() const;
					/**
					 * Locks the mutex with a std::lock_guard and leaves it locked
					 * */
					void lockThread();
					/**
					 * Unlocks the std::lock_guard guarding the mutex
					 * */
					void unlockThread();
					/**
					 * Gets the top element (or front element) of the internal queue.
					 *
					 * @return The first transition
					 * */
					Transition top() const;
					void pop();
				private:
					std::deque<Transition> queue;
					mutable std::shared_mutex lock;
					// std::lock_guard<std::shared_mutex> guard;
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
// 					, storm::storage::BitVectorHashMap<uint8_t>& stateThreadMap // State storage owned by parents
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
				std::pair<uint8_t, StateType> requestOwnership(CompressedState const & state, uint8_t threadIndex, StateType requestedId = 0);
				/**
				* Gets the owning thread of a particular state without locking the mutex.
				* This allows for threads to use the many-read, one-write idea put forth
				* in the paper.
				*
				* @param state The state who we wonder if owns
				* @return The thread who owns `state`
				* */
				uint8_t whoOwns(CompressedState const & state) const;
				/**
				 * Gets the index of a state which already exists. If the state does not
				 * exist, returns 0.
				 *
				 * @param state The state to look up
				 * @return The state index
				 * */
				StateType whatIsIndex(CompressedState const & state);
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
					, uint8_t threadIndex
					, StateType fromIndex
					, ValueType transitionRate
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
				void terminate();
			protected:
				void registerTransitions();
			private:
				std::vector<LockableDeque> transitionQueues;
				std::shared_mutex ownershipMutex;
				const uint8_t numberExplorationThreads;
				storm::storage::BitVectorHashMap<uint8_t, storm::storage::Murmur3BitVectorHash<StateType>>& stateThreadMap;
				std::vector<ExplorationThread<ValueType, RewardModelType, StateType>> explorationThreads;
			};

		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_THREADS_CONTROLTHREAD_H
