#include "Ateams.hpp"

#include "Control.hpp"

using namespace std;
using namespace chrono;

volatile TerminationInfo STATUS;

int main(int argc, char *argv[]) {

	fix_terminal();

	signal(SIGTERM, internalSignalHandler);		// Interrompe o programa ao ser requisitado externamente
	signal(SIGINT, internalSignalHandler);		// Interrompe o programa ao se pessionar 'CTRL-C'

	srand(unsigned(time(NULL)));

	try {
		Control *ctrl;

		ExecutionInfo *executionInfo;

		for (int i = 0; i < 1; i++) {
			ctrl = Control::getInstance(argc, argv);

			ctrl->start();

			ctrl->run();

			ctrl->finish();

			ctrl->printSolutionInfo();

			ctrl->printExecutionInfo();

			executionInfo = Control::terminate();

			cout << endl <<  COLOR_GREEN << "Termination Reason: " << getTerminationInfo(STATUS) << " (" << getExecutionTime(executionInfo->executionTime) << ") " << COLOR_DEFAULT << endl << endl;
		}

		delete executionInfo;

		return 0;
	} catch (...) {
		beep();

		exception_ptr exception = current_exception();

		cerr << endl << COLOR_RED << "Exception: " << getExceptionMessage(exception) << COLOR_DEFAULT << endl << endl;

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
			default:
				break;
		}
	} else {
		cerr << endl << endl << COLOR_RED << "Aborting..." << COLOR_DEFAULT << endl << endl;

		exit(1);
	}
}
