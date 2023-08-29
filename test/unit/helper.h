/**
 * STAMINA - the [ST]ochasic [A]pproximate [M]odel-checker for [IN]finite-state [A]nalysis
 * Copyright (C) 2023 Fluent Verification, Utah State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 *
 **/

#ifndef STAMINA_UNITTESTS_HELPER_H
#define STAMINA_UNITTESTS_HELPER_H

#include <csignal>
#include <iostream>

#include <stamina/core/Options.h>

namespace stamina_test {

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
	set_default_values() {
		stamina::core::Options::kappa = 1.0;
		stamina::core::Options::reduce_kappa = 1.25; // 2.0;
		stamina::core::Options::approx_factor = 2.0;
		stamina::core::Options::fudge_factor = 1.0;
		stamina::core::Options::prob_win = 1.0e-3;
		stamina::core::Options::max_approx_count = 10;
		stamina::core::Options::no_prop_refine = false;
		stamina::core::Options::cudd_max_mem = "1g";
		stamina::core::Options::export_trans = "";
		stamina::core::Options::rank_transitions = false;
		stamina::core::Options::max_iterations = 10000;
		stamina::core::Options::method = STAMINA_METHODS::ITERATIVE_METHOD;
		stamina::core::Options::threads = 1;
		stamina::core::Options::preterminate = false;
		stamina::core::Options::event = EVENTS::UNDEFINED;
		stamina::core::Options::distance_weight = 1.0;
		stamina::core::Options::quiet = false;
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
			return true;
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

} // namespace stamina_test

#endif // STAMINA_UNITTESTS_HELPER_H
