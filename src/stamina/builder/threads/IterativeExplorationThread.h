/**
 * An implementation of the ExplorationThread abstract class which uses the STAMINA iterative (2.0/2.5)
 * algorithm.
 *
 * Created by Josh Jeppson
 * */

#ifndef STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H
#define STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H

#include "ExplorationThread.h"

namespace stamina {
	namespace builder {
		namespace threads {
			template<typename ValueType, typename RewardModelType, typename StateType>
			class IterativeExplorationThread : public ExplorationThread<StateType, RewardModelType, ValueType> {
			public:
				/**
				* Constructor. Invokes super's constructor.
				*
				* @param parent The model builder who owns this thread
				* @param threadIndex The index of this thread
				* */
				IterativeExplorationThread(
					StaminaModelBuilder<ValueType, RewardModelType, StateType> * parent
					, uint8_t threadIndex
					, ControlThread<StateType, RewardModelType, ValueType> & controlThread
				);

			protected:
				virtual void exploreStates() override;
				virtual void exploreState(StateAndProbability & stateProbability) override;
			};
		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H
