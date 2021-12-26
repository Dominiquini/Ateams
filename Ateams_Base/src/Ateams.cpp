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

		/* Adiciona as heuristicas selecionadas */
		Control *ctrl = Control::getInstance(argc, argv);

		/* Leitura dos dados passados por arquivos */
		Problem::readProblemFromFile(ctrl->getInputDataFile());

		/* Le memoria prinipal do disco, se especificado */
		list<Problem*> *popInicial = Problem::readPopulationFromLog(ctrl->getOutputLogFile());

		/* Inicia a execucao */
		Problem *bestSolution = ctrl->start(popInicial);

		if (popInicial != NULL) {
			popInicial->clear();
			delete popInicial;
		}

		list<Problem*> *solutions = ctrl->getSolutions();

		/* Escreve memoria principal no disco */
		Problem::writeCurrentPopulationInLog(ctrl->getOutputLogFile(), solutions);

		delete solutions;

		ExecutionInfo info = ctrl->getExecutionInfo();

		/* Escreve solucao em arquivo no disco */
		Problem::writeResultInFile(ctrl->getInputDataFile(), ctrl->getInputParameters(), &info, ctrl->getOutputResultFile());

		ctrl->printSolution(false);

		ctrl->printExecution();

		delete bestSolution;

		ctrl->checkSolutions();

		Control::terminate();

		Problem::deallocateMemory();

		if (Problem::numInst != 0)
			throw "Memory Leak!";
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
