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
	EXECUTING, FINISHED_NORMALLY, INCOMPLETE, USER_SIGNALED, EXECUTION_TIMEOUT, LACK_OF_IMPROVEMENT, RESULT_ACHIEVED
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
		case RESULT_ACHIEVED:
			return "RESULT_ACHIEVED";
		default:
			return "UNKNOWN";
	}
}

class ProgressBar {
public:

	ProgressBar(int maxProgress, string label, string color, int progressBarLength) {
		this->label = label;
		this->color = color;
		this->maxProgress = maxProgress;
		this->progressBarLength = progressBarLength - label.size();

		reset();
	}
	ProgressBar(int maxProgress, string label) : ProgressBar(maxProgress, label, DEFAULT_PROGRESS_COLOR, DEFAULT_PROGRESS_SIZE) {
	}
	ProgressBar(int maxProgress) : ProgressBar(maxProgress, "", DEFAULT_PROGRESS_COLOR, DEFAULT_PROGRESS_SIZE) {
	}
	~ProgressBar() {
		reset();
	}

	void reset() {
		this->currentProgress = 0;
		this->amountOfFiller = 0;
		this->currUpdateVal = 0;
	}

	void init() {
		cout << endl << color;
	}

	void end() {
		cout << endl << COLOR_DEFAULT;
	}

	void update(int newProgress) {
		int oldProgress = currentProgress;
		currentProgress = min(maxProgress, newProgress);

		if (oldProgress != currentProgress) {
			amountOfFiller = (int) (progress() * (double) progressBarLength);

			print();
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
		cout << lastPartOfpBar << " (" << (int) (progress() * 100) << "%)";

		currUpdateVal += 1;

		cout << flush;
	}

	double progress() {
		return (double) currentProgress / (double) maxProgress;
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
