#ifndef STAMINA_BUILDER_MODELBUILDERMUTEX_H
#define STAMINA_BUILDER_MODELBUILDERMUTEX_H

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_set>

namespace stamina {
	namespace builder {
		template <typename StateType = uint32_t>
		class ModelBuilderMutex {
		public:
			ModelBuilderMutex();
			bool stateIsInUse(StateType stateId);
			void setStateInUse(StateType stateId, bool inUse = true);
		protected:
			std::unordered_set<StateType> inUseStates; // In theory, this should never exceed close to 1e3 states even on very strongly connected models
			std::shared_mutex write;
		};
	} // namespace builder
} // namespace stamina
#endif // STAMINA_BUILDER_MODELBUILDERMUTEX_H
