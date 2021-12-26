#include "Ateams.hpp"

#include "Control.hpp"
#include "Problem.hpp"

using namespace std;

volatile TerminationInfo STATUS = EXECUTING;

int main(int argc, char *argv[]) {

	/* Interrompe o programa ao se pessionar 'ctrl-c' */
	signal(SIGINT, SIGINTHandler);

	srand(unsigned(time(NULL)));

	try {
		cout << endl;

		Control *ctrl = Control::getInstance(argc, argv);

		ctrl->init();

		ctrl->run();

		ctrl->finish();

		ctrl->printSolution(false);

		ctrl->printExecution();

		ctrl->checkSolutions();

		Control::terminate();
	} catch (...) {
		exception_ptr exception = current_exception();

		cout << endl << "Error During Execution: ";
		cerr << getExceptionMessage(exception) << endl << endl;

		exit(1);
	}

	cout << endl << endl << "Termination Reason: " << getTerminationInfo(STATUS) << endl << endl;

	return 0;
}

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

void SIGINTHandler(int signal) {
	STATUS = USER_SIGNALED;
}
