/**
 * Exploration threads can explore a state space with other threads, and will only explore threads
 * that it owns.
 *
 * Created by Josh Jeppson on Jul 8, 2022
 * */

#ifndef STAMINA_BUILDER_THREADS_EXPLORATIONTHREAD_H
#define STAMINA_BUILDER_THREADS_EXPLORATIONTHREAD_H

#include "BaseThread.h"
#include "ControlThread.h"

namespace stamina {
	namespace builder {
		namespace threads {

			template <typename StateType, typename RewardModelType, typename ValueType>
			class ExplorationThread : public BaseThread<ValueType, RewardModelType, StateType> {
			public:
				typedef std::pair<CompressedState &, std::shared_ptr<StaminaModelBuilder<ValueType, RewardModelType, StateType>>> StateAndProbability;
				/**
				* Constructor. Invokes super's constructor and stores the
				* thread index which cannot change for the life of the thread
				*
				* @param parent The model builder who owns this thread
				* @param threadIndex The index of this thread
				* */
				ExplorationThread(
					StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
					, uint8_t threadIndex
					, ControlThread<ValueType, RewardModelType, StateType> & controlThread
				);
				uint8_t getIndex();
				uint32_t getNumberOfOwnedStates();
				bool isIdling();
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
				virtual void exploreStates() = 0;
				virtual void exploreState(StateAndProbability & stateProbability) = 0;
				// Weak priority on crossExplorationQueue (superseded by mutex lock)
				std::shared_mutex crossExplorationQueueMutex;
				std::deque<std::pair<CompressedState, double>> crossExplorationQueue;
				std::deque<StateAndProbability> mainExplorationQueue;
			private:
				const uint8_t threadIndex;
				uint32_t numberOfOwnedStates;
				bool idling;
				ControlThread<ValueType, RewardModelType, StateType> & controlThread;
			};
		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_THREADS_EXPLORATIONTHREAD_H
