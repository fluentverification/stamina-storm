#ifndef STAMINA_BUILDER_THREADS_BASETHREAD_H
#define STAMINA_BUILDER_THREADS_BASETHREAD_H

namespace stamina {
	namespace builder {
		namespace threads {
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
		}
	}
}


#endif // STAMINA_BUILDER_THREADS_BASETHREAD_H
