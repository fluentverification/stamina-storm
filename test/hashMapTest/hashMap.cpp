#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <unordered_set>

#define NUM_TO_INSERT 50000000

int main(int argc, char ** argv) {
	int numToInsert = atoi(argv[1]);
	std::unordered_set<int> set;
	set.reserve(numToInsert * 2);
	for (int i = 0; i < numToInsert; i++) {
		set.insert(i);
		// auto result = set.find(i);
	}
	return 0;
}
