#include "ThreadBase.h"

namespace stamina {
namespace threads {

void
ThreadBase::registerAllSystemThreadNumbers() {
	// Register system interrupts
	sigid_t systemSignals[] = {
		SIGINT
		, SIGTERM
		, SIGBUS
		, SIGILL
		, SIGALRM
		, SIGABRT
		, SIGSTOP
		, SIGSEGV
		, SIGFPE
	};
	for (sigid_t sigId : systemSignals) {
		registeredInterrupts.insert(sigId);
	}
}

bool
threadNumberWasRegistered(uint8_t threadNumber) {
	return !registeredNumbers.find(threadNumber) == registeredNumbers.end();
}

bool
interruptWasRegistered(sigid_t interrupt) {
	return !registeredInterrupts.find(interrupt) == registeredInterrupts.end();
}

void
registerFunction(std::function & function, sigid_t signalId) {

}

} // namespace threads
} // namespace stamina
