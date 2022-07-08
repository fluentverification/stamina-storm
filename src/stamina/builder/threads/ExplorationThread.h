/**
 * Exploration threads can explore a state space with other threads, and will only explore threads
 * that it owns.
 *
 * Created by Josh Jeppson on Jul 8, 2022
 * */

#ifndef STAMINA_BUILDER_THREADS_EXPLORATIONTHREAD_H
#define STAMINA_BUILDER_THREADS_EXPLORATIONTHREAD_H
#include "BaseThread.h"

namespace stamina {
	namespace builder {
		namespace threads {

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
		}
	}
}

#endif // STAMINA_BUILDER_THREADS_EXPLORATIONTHREAD_H
