#ifndef STAMINA_UNITTESTS_HELPER_H
#define STAMINA_UNITTESTS_HELPER_H

#include <csignal>
#include <iostream>

#ifdef WINDOWS
#include <Windows.h>
#else // WINDOWS not defined
static bool received_segfault = false;

void
handle_signals(int sig) {
	// Test to see if we received
	if (sig == SIGSEGV) {
		received_segfault = true;
		std::cerr << "[ERROR (Unit Tests)] Received segmentation fault! (Signum " << sig << ")" << std::endl;
		// exit gracefully
		std::cerr << "Unit tests cannot continue after this. Please run verbosely to see where unit tests failed" << std::endl;
		std::exit(1);
	}
	else {
		std::cerr << "[NOTE (Unit Tests)] Received signal " << sig << "!" << std::endl;
	}
}

void
setup_signal_handling() {
	signal(SIGSEGV, handle_signals);
}

#endif // WINDOWS

template <typename T>
bool
pointer_is_valid(T * pointer) {
#ifdef WINDOWS
	__try {
		auto val = *pointer;
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
#else // WINDOWS not defined
	/* Need to scope it so it's not in the same scope as the rest of the function */
	auto f = [pointer]() {
		auto val = *pointer;
	};
	f();
	bool bad_pointer = received_segfault;
	received_segfault = false;
	return !bad_pointer;
#endif // WINDOWS
}

#endif // STAMINA_UNITTESTS_HELPER_H
