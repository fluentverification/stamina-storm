#ifndef STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H
#define STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H

#include "ExplorationThread.h"

namespace stamina {
	namespace builder {
		namespace threads {
			template<typename ValueType, typename RewardModelType, typename StateType>
			class IterativeExplorationThread : public ExplorationThread<ValueType, RewardModelType, StateType> {
			public:

			};
		} // namespace threads
	} // namespace builder
} // namespace stamina

#endif // STAMINA_BUILDER_THREADS_ITERATIVEEXPLORATIONTHREAD_H
