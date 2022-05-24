#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <unordered_set>

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
	std::unordered_set<int> set;
	set.reserve(nextPrime(numToInsert * 2));
	for (int i = 0; i < numToInsert; i++) {
		set.insert(i);
		// auto result = set.find(i);
	}
	return 0;
}
