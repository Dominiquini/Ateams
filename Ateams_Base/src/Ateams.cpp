#include "Ateams.hpp"

#include "Control.hpp"

using namespace std;

pthread_mutex_t mutex_rand = PTHREAD_MUTEX_INITIALIZER;	// Mutex que protege a geracao de numeros aleatorios

volatile TerminationInfo STATUS;

int main(int argc, char *argv[]) {

	fix_terminal();

	signal(SIGTERM, internalSignalHandler);		// Interrompe o programa ao ser requisitado externamente
	signal(SIGINT, internalSignalHandler);		// Interrompe o programa ao se pessionar 'CTRL-C'

	srand(unsigned(time(NULL)));

	try {
		cout << endl;

		Control *ctrl;

		for (int i = 0; i < 1; i++) {
			ctrl = Control::getInstance(argc, argv);

			ctrl->init();

			ctrl->run();

			ctrl->finish();

			ctrl->printSolution();

			ctrl->printExecution();

			Control::terminate();

			cout << endl << endl << COLOR_GREEN << "Termination Reason: " << getTerminationInfo(STATUS) << COLOR_DEFAULT << endl << endl;
		}

		return 0;
	} catch (...) {
		beep();

		exception_ptr exception = current_exception();

		cerr << endl << endl << COLOR_RED << "Exception: " << getExceptionMessage(exception) << COLOR_DEFAULT << endl << endl;

		return -1;
	}
}

bool randomBool() {
	return randomNumber(0, 2) == 1;
}

int randomPercentage() {
	return randomNumber(0, 101);
}

int randomNumber() {
	return randomNumber(RAND_MAX);
}

int randomNumber(int max) {
	return randomNumber(0, max);
}

int randomNumber(int min, int max) {
	uniform_int_distribution<int> randomDistribution(min, max - 1);

#if RANDOM_THREAD_SAFE
	pthread_lock(&mutex_rand);

	int rand = randomDistribution(randomEngine);

	pthread_unlock(&mutex_rand);
#else
	int rand = randomDistribution(randomEngine);
#endif

	return rand;
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
