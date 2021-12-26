#include <GL/freeglut.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <pthread.h>
#include <semaphore.h>

#include <functional>
#include <exception>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <vector>
#include <bitset>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <cctype>
#include <ctime>
#include <list>
#include <set>

using namespace std;

#ifndef _ATEAMS_
#define _ATEAMS_

#define RANDOM_TYPE -1

#if (RANDOM_TYPE > 0)
  static default_random_engine randomEngine(RANDOM_TYPE);
#elif (RANDOM_TYPE == -1)
  static default_random_engine randomEngine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
#elif (RANDOM_TYPE == -2)
  static mt19937 randomEngine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
#elif (RANDOM_TYPE == -3)
  static random_device randomEngine;
#endif

enum TerminationInfo {
	EXECUTING, FINISHED_NORMALLY, USER_SIGNALED, EXECUTION_TIMEOUT, LACK_OF_IMPROVEMENT, RESULT_ACHIEVED
};

struct ExecHeuristicsInfo {
	unsigned int tabu = 0;
	unsigned int genetic = 0;
	unsigned int annealing = 0;
};

int xRand();

int xRand(int max);

int xRand(int min, int max);

void SIGINTHandler(int signal);

inline string getExceptionMessage(exception_ptr &eptr) {
	try {
		rethrow_exception(eptr);
	} catch (const exception &e) {
		return e.what();
	} catch (const string &e) {
		return e;
	} catch (const char *e) {
		return e;
	} catch (...) {
		return (current_exception() ? current_exception().__cxa_exception_type()->name() : "Unexpected Exception!");
	}
}

inline string getTerminationInfo(TerminationInfo info) {
	switch(info) {
		case EXECUTING: return "EXECUTING";
		case FINISHED_NORMALLY: return "FINISHED_NORMALLY";
		case USER_SIGNALED: return "USER_SIGNALED";
		case EXECUTION_TIMEOUT: return "EXECUTION_TIMEOUT";
		case LACK_OF_IMPROVEMENT: return "LACK_OF_IMPROVEMENT";
		case RESULT_ACHIEVED: return "RESULT_ACHIEVED";
		default: return "UNKNOWN";
	}
}

#endif
