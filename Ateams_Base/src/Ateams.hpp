#include <bits/stdc++.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>

#include <type_traits>
#include <charconv>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <limits>
#include <cmath>

#include <algorithm>
#include <iterator>
#include <utility>
#include <sstream>
#include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <array>
#include <list>
#include <set>
#include <map>

#include <random>
#include <chrono>

#include "Constants.hpp"

#include "Random.hpp"
#include "PugiXML.hpp"
#include "Semaphore.hpp"

using namespace std;
using namespace chrono;

#ifndef _ATEAMS_
#define _ATEAMS_

#if IS_LINUX
  #define PTHREAD_NORMAL_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
  #define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
  #define PTHREAD_RECURSIVE_MUTEX_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#endif

#define sleep_ms(time) this_thread::sleep_for(chrono::milliseconds(time)) ;

#define fix_terminal() ios_base::sync_with_stdio(IS_LINUX) ;

#define previous_line() cout << ASCII_PREVIOUS_LINE << flush ;

#define clear_line() cout << ASCII_CLEAR_LINE << flush ;

#define beep() cout << CHAR_BEEP << flush ;

#define quote(var) #var

#define stream_formatter(width, fill) setw(width) << setfill(fill)

enum TerminationInfo {
	EXECUTING, FINISHED_NORMALLY, USER_SIGNALED, EXECUTION_TIMEOUT, LACK_OF_IMPROVEMENT, TOO_MANY_SOLUTIONS, RESULT_ACHIEVED
};

template<typename T>
struct PrimitiveWrapper {

	static_assert(!is_pointer<T>::value, "T can't be a pointer!");
	static_assert(is_arithmetic<T>::value, "T must be a primitive!");

	T value = 0;

	PrimitiveWrapper(T value) {
		this->value = value;
	}
};

struct HeuristicsCounter {

	unsigned int tabu = 0;
	unsigned int genetic = 0;
	unsigned int annealing = 0;
};


void internalSignalHandler(int signal);

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
	switch (info) {
		case EXECUTING:
			return "EXECUTING";
		case FINISHED_NORMALLY:
			return "FINISHED_NORMALLY";
		case USER_SIGNALED:
			return "USER_SIGNALED";
		case EXECUTION_TIMEOUT:
			return "EXECUTION_TIMEOUT";
		case LACK_OF_IMPROVEMENT:
			return "LACK_OF_IMPROVEMENT";
		case TOO_MANY_SOLUTIONS:
			return "TOO_MANY_SOLUTIONS";
		case RESULT_ACHIEVED:
			return "RESULT_ACHIEVED";
		default:
			return "UNKNOWN";
	}
}

inline string getExecutionTime(milliseconds duration) {
	return to_string(duration_cast<seconds>(duration).count()) + 's';
}

#endif
