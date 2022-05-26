#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include "../../src/stamina/util/StateIndexArray.h"
#include "../../src/stamina/StaminaModelBuilder.h"
// #include <unordered_set>

#define NUM_TO_INSERT 50000000

bool isPrime(int n) {
    // Corner cases
    if (n <= 1) { return false; }
    if (n <= 3) { return true; }

    if (n % 2 == 0 || n % 3 == 0) { return false; }

    for (int i = 5; i * i <= n; i = i + 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
           return false;
		}
	}

    return true;
}

int nextPrime(int N) {

    // Base case
    if (N <= 1) {
        return 2;
	}

    int prime = N;
    bool found = false;

    // Loop continuously until isPrime returns
    // true for a number greater than n
    while (!found) {
        prime++;
        if (isPrime(prime)) {
            found = true;
		}
    }

    return prime;
}


int main(int argc, char ** argv) {
	int numToInsert = atoi(argv[1]);
	stamina::util::StateIndexArray<uint32_t, stamina::StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>::ProbabilityState> set;
	for (int i = 0; i < numToInsert; i++) {
		CompressedState cs(0);
		std::shared_ptr<stamina::StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>::ProbabilityState> state(
			cs
			, i
		);
		set.put(i, state);
		// auto result = set.find(i);
	}
	return 0;
}
