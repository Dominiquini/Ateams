#include <chrono>
#include <random>
#include <mutex>

#ifndef _RANDOM_
#define _RANDOM_

#if (RANDOM_TYPE > 0)
  static std::default_random_engine randomEngine(RANDOM_TYPE);
#elif (RANDOM_TYPE == -1)
  static std::default_random_engine randomEngine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
#elif (RANDOM_TYPE == -2)
  static std::mt19937 randomEngine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
#elif (RANDOM_TYPE == -3)
  static std::random_device randomEngine;
#endif

static std::mutex mutex_rand;	// Mutex que protege a geracao de numeros aleatorios

class Random {
public:

	inline static bool randomBool() {
		return randomNumber(0, 2) == 1;
	}

	inline static int randomPercentage() {
		return randomNumber(0, 101);
	}

	inline static int randomNumber() {
		return randomNumber(RAND_MAX);
	}

	inline static int randomNumber(int max) {
		return randomNumber(0, max);
	}

	inline static int randomNumber(int min, int max) {
		std::uniform_int_distribution<int> randomDistribution(min, max - 1);

#if RANDOM_THREAD_SAFE
		std::lock_guard<decltype(mutex_rand)> lock_info_start(mutex_rand);
#endif

		return randomDistribution(randomEngine);
	}
};

#endif
