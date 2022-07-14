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
			template <typename StateType, typename RewardModelType, typename ValueType>
			class IterativeExplorationThread : public ExplorationThread<StateType, RewardModelType, ValueType> {
			public:
				typedef typename ExplorationThread<StateType, RewardModelType, ValueType>::StateAndProbability StateAndProbability;
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
					, uint32_t stateSize
					, util::StateIndexArray<StateType, ProbabilityState<StateType>> * stateMap
					, std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
					, std::function<StateType (CompressedState const&)> stateToIdCallback
				);


			protected:
				virtual void exploreStates() override;
				virtual void exploreState(StateAndProbability & stateProbability) override;
			private:
				uint32_t numberTerminal;
				bool isCtmc;
			};
		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H
