#include "Ateams.hpp"

#include "Control.hpp"
#include "Problem.hpp"

using namespace std;

volatile TerminationInfo STATUS = EXECUTING;

int main(int argc, char *argv[]) {

	fix_terminal();

	signal(SIGTERM, internalSignalHandler);		// Interrompe o programa ao ser requisitado externamente
	signal(SIGINT, internalSignalHandler);		// Interrompe o programa ao se pessionar 'CTRL-C'

	srand(unsigned(time(NULL)));

	try {
		cout << endl;

		Control *ctrl = Control::getInstance(argc, argv);

		ctrl->init();

		ctrl->run();

		ctrl->finish();

		ctrl->printSolution();

		ctrl->printExecution();

		Control::terminate();

		cout << endl << endl << "Termination Reason: " << getTerminationInfo(STATUS) << endl << endl;

		return 0;
	} catch (...) {
		exception_ptr exception = current_exception();

		cerr << endl << endl << COLOR_RED << "Error During Execution: " << getExceptionMessage(exception) << COLOR_DEFAULT << endl << endl;

		return -1;
	}
}

int randomNumber() {
	return randomNumber(RAND_MAX);
}

int randomNumber(int max) {
	return randomNumber(0, max);
}

int randomNumber(int min, int max) {
	uniform_int_distribution<int> randomDistribution(min, max - 1);

	return randomDistribution(randomEngine);
}

void internalSignalHandler(int signal) {
	if (STATUS == EXECUTING) {
		switch (signal) {
			case SIGTERM:
			case SIGINT:
				STATUS = USER_SIGNALED;
				break;
		}
	} else {
		cerr << endl << endl << COLOR_RED << "Aborting..." << COLOR_DEFAULT << endl;

		exit(1);
	}
}
