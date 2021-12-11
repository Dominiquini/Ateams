#include "Ateams.hpp"

#include "Control.hpp"
#include "Problem.hpp"

using namespace std;

volatile bool TERMINATE = false;

int main(int argc, char *argv[]) {

	/* Interrompe o programa ao se pessionar 'ctrl-c' */
	signal(SIGINT, terminate);

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

		cout << endl << endl << "Worst Final Solution: " << Problem::worst << endl;
		cout << endl << "Best Final Solution: " << Problem::best << endl;

		ExecutionInfo info = ctrl->getExecutionInfo();

		/* Escreve solucao em arquivo no disco */
		Problem::writeResultInFile(ctrl->getInputDataFile(), ctrl->getInputParameters(), &info, ctrl->getOutputResultFile());

		cout << endl << endl << "Solution:" << endl << endl;

		ctrl->printSolution(bestSolution);

		delete bestSolution;

		cout << endl << endl << "Explored Solutions: " << Problem::totalNumInst << endl;

		cout << endl << endl << "Executions: " << info.executionCount << endl << endl;

		ctrl->checkSolutions();

		Control::terminate();

		Problem::deallocateMemory();

		if (Problem::numInst != 0)
			throw "Memory Leak!";

		cout << endl;
	} catch (...) {
		exception_ptr exception = current_exception();

		cout << endl << "Error During Execution: ";
		cerr << getExceptionMessage(exception) << endl << endl;

		exit(1);
	}

	return 0;
}

void terminate(int signal) {
	TERMINATE = true;
}
