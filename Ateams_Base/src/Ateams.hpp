#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <functional>
#include <exception>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
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

#ifdef _WIN32
  #define fix_terminal() ios_base::sync_with_stdio(false);
#else
  #define fix_terminal() ios_base::sync_with_stdio(true);
#endif

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

#define DEFAULT_PROGRESS_SIZE 100
#define DEFAULT_PROGRESS_COLOR COLOR_RED

#define COLOR_BLACK "[30m"
#define COLOR_RED "[31m"
#define COLOR_GREEN "[32m"
#define COLOR_YELLOW "[33m"
#define COLOR_BLUE "[34m"
#define COLOR_MAGENTA "[35m"
#define COLOR_CYAN "[36m"
#define COLOR_WHITE "[37m"
#define COLOR_DEFAULT "[39m"

#define PREVIOUS_LINE "\033[F"

enum TerminationInfo {
	EXECUTING, FINISHED_NORMALLY, INCOMPLETE, USER_SIGNALED, EXECUTION_TIMEOUT, LACK_OF_IMPROVEMENT, TOO_MANY_SOLUTIONS, RESULT_ACHIEVED
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

struct ExecHeuristicsInfo {
	unsigned int tabu = 0;
	unsigned int genetic = 0;
	unsigned int annealing = 0;
};

int xRand();

int xRand(int max);

int xRand(int min, int max);

void SignalHandler(int signal);

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
		case INCOMPLETE:
			return "INCOMPLETE";
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

class ProgressBar {
public:

	ProgressBar(string label, string color, int progressBarLength) {
		this->label = label;
		this->color = color;
		this->progressBarLength = progressBarLength - label.size();

		this->maxProgress = -1;
		this->currentProgress = -1;

		this->amountOfFiller = 0;
		this->currUpdateVal = 0;
	}
	ProgressBar(string label) : ProgressBar(label, DEFAULT_PROGRESS_COLOR, DEFAULT_PROGRESS_SIZE) {
	}
	ProgressBar() : ProgressBar("", DEFAULT_PROGRESS_COLOR, DEFAULT_PROGRESS_SIZE) {
	}
	~ProgressBar() {
	}

	void init(int maxProgress) {
		this->maxProgress = maxProgress;

		cout << endl << color;

		update(0);
	}

	void end() {
		this->maxProgress = -1;

		update(currentProgress);

		cout << endl << COLOR_DEFAULT;
	}

	void update(int newProgress) {
		if (maxProgress != -1) {
			int oldProgress = currentProgress;
			currentProgress = min(maxProgress, newProgress);

			if (oldProgress != currentProgress) {
				amountOfFiller = (int) (progress() * (double) progressBarLength);

				print();
			}
		}
	}

	void print() {
		currUpdateVal %= pBarUpdater.length();

		cout << "\r" << label << firstPartOfpBar;
		for (int a = 0; a < amountOfFiller; a++) {
			cout << pBarFiller;
		}
		cout << pBarUpdater[currUpdateVal];
		for (int b = 0; b < progressBarLength - amountOfFiller; b++) {
			cout << " ";
		}
		cout << lastPartOfpBar << " (" << percentage() << "%)";

		currUpdateVal += 1;

		cout << flush;
	}

	float progress() {
		return (float) currentProgress / (float) maxProgress;
	}

	int percentage() {
		return progress() * 100;
	}

private:

	const string firstPartOfpBar = "[", lastPartOfpBar = "]", pBarFiller = "|", pBarUpdater = "|";

	string label;
	string color;
	int maxProgress;
	int progressBarLength;

	int currentProgress;
	int amountOfFiller;
	int currUpdateVal;

};

#endif
