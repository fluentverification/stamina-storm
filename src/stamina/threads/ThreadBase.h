#ifndef STAMINA_THREADS_THREADBASE_H
#define STAMINA_THREADS_THREADBASE_H

#include <thread>
#include <unordered_set>

namespace stamina {
	namespace threads {
		typedef int sigid_t;
		enum SIGNAL_IDS {

		};
		class ThreadBase {
		public:
			ThreadBase(uint8_t threadNumber);

			void registerFunction(std::function & function, sigid_t signalId);
			/* Statically accessible members (all threads can access this */
			static bool threadNumberWasRegistered(uint8_t threadNumber);
			static bool interruptWasRegistered(sigid_t interrupt);
			static void registerAllSystemThreadNumbers();
		protected:
			static std::unordered_set<uint8_t> registeredNumbers;
			static std::unordered_set<sigid_t> registeredInterrupts;

			/* Data Members */
			uint8_t threadNumber;
		};
	}
}
#endif // STAMINA_THREADS_THREAD_H
