#include "Ateams.hpp"

#include "Control.hpp"
#include "Problem.hpp"

using namespace std;

volatile TerminationInfo STATUS = EXECUTING;

int main(int argc, char *argv[]) {

#ifdef _WIN32
	ios_base::sync_with_stdio(false);
#endif

	signal(SIGTERM, SignalHandler);		// Interrompe o programa ao ser requisitado externamente
	signal(SIGINT, SignalHandler);		// Interrompe o programa ao se pessionar 'CTRL-C'

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

void SignalHandler(int signal) {
	switch (signal) {
		case SIGTERM:
		case SIGINT:
			STATUS = USER_SIGNALED;
			break;
	}
}
