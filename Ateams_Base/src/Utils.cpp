#include "Utils.hpp"

#include "Control.hpp"

using namespace std;

#if (RANDOM_TYPE > 0)
  static default_random_engine randomEngine(RANDOM_TYPE);
#elif (RANDOM_TYPE == -1)
  static default_random_engine randomEngine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
#elif (RANDOM_TYPE == -2)
  static mt19937 randomEngine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
#elif (RANDOM_TYPE == -3)
  static random_device randomEngine;
#endif

int xRand() {
	return xRand(RAND_MAX);
}

int xRand(int max) {
	return xRand(0, max);
}

int xRand(int min, int max) {
	uniform_int_distribution<int> randomDistribution(min, max - 1);

	return randomDistribution(randomEngine);
}
