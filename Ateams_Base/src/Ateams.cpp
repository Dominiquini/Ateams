#include "Ateams.hpp"

#include "Control.hpp"

using namespace std;

volatile TerminationInfo STATUS;

int main(int argc, char *argv[]) {

	fix_terminal();

	signal(SIGTERM, internalSignalHandler);		// Interrompe o programa ao ser requisitado externamente
	signal(SIGINT, internalSignalHandler);		// Interrompe o programa ao se pessionar 'CTRL-C'

	srand(unsigned(time(NULL)));

	try {
		cout << endl;

		Control *ctrl;

		ExecutionInfo executionTime;

		for (int i = 0; i < 1; i++) {
			ctrl = Control::getInstance(argc, argv);

			ctrl->init();

			ctrl->run();

			ctrl->finish();

			ctrl->printSolution();

			ctrl->printExecution();

			executionTime = Control::terminate();

			cout << endl << endl << COLOR_GREEN << "Termination Reason: " << getTerminationInfo(STATUS) << " (" <<  executionTime.executionTime.count() << "s) " << COLOR_DEFAULT << endl << endl;
		}

		return 0;
	} catch (...) {
		beep();

		exception_ptr exception = current_exception();

		cerr << endl << endl << COLOR_RED << "Exception: " << getExceptionMessage(exception) << COLOR_DEFAULT << endl << endl;

		return -1;
	}
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
		cerr << endl << endl << COLOR_RED << "Aborting..." << COLOR_DEFAULT << endl << endl;

		exit(1);
	}
}
