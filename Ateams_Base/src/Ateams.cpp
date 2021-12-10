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

		list<Problem*> *pop = ctrl->getPop();
		list<Problem*>::const_iterator iter1, iter2;

		/* Testa a memoria principal por solucoes repetidas ou fora de ordem */
		for (iter1 = pop->begin(); iter1 != pop->end(); iter1++)
			for (iter2 = iter1; iter2 != pop->end(); iter2++)
				if ((iter1 != iter2) && (fnequal1(*iter1, *iter2) || fncomp1(*iter2, *iter1)))
					cout << endl << endl << "Incorrect Main Memory!!!" << endl;

		/* Escreve memoria principal no disco */
		Problem::dumpCurrentPopulationInLog(ctrl->getOutputLogFile(), pop);

		delete pop;

		cout << endl << endl << "Worst Final Solution: " << Problem::worst << endl;
		cout << endl << "Best Final Solution: " << Problem::best << endl;

		ExecInfo info;
		ctrl->getInfo(&info);

		/* Escreve solucao em arquivo no disco */
		Problem::writeResultInFile(ctrl->getInputDataFile(), ctrl->getInputParameters(), &info, ctrl->getOutputResultFile());

		cout << endl << endl << "Solution:" << endl << endl;

		ctrl->printSolution(bestSolution);

		delete bestSolution;

		Control::terminate();

		Problem::deallocateMemory();

		cout << endl << endl << "Explored Solutions: " << Problem::totalNumInst << endl << endl;

		if (Problem::numInst != 0)
			cout << "Memory Leak! ( " << Problem::numInst << " )" << endl << endl;

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
