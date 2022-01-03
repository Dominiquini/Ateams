#include "GraphicalOverview.hpp"

using namespace this_thread;

GraphicalOverview::GraphicalOverview(int *argc, char **argv) {
	this->argc = argc;
	this->argv = argv;

	glutInit(this->argc, this->argv);
}

GraphicalOverview::~GraphicalOverview() {
	int window = glutGetWindow();
	if (window != 0)
		glutDestroyWindow(window);
}

void GraphicalOverview::run() {
	pthread_t threadAnimation;

	pthread_attr_t attrDetached;
	pthread_attr_init(&attrDetached);
	pthread_attr_setdetachstate(&attrDetached, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&threadAnimation, &attrDetached, GraphicalOverview::asyncRun, (void*) this->argv[0]) != 0)
		throw "Thread Creation Error! (GraphicalOverview::asyncRun)";

	pthread_attr_destroy(&attrDetached);
}

void* GraphicalOverview::asyncRun(void *title) {
	if (STATUS == EXECUTING) {
		/* Cria a tela */
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
		glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		glutInitWindowPosition(0, 0);

		glutCreateWindow((char*) title);

		/* Define as funcoes de desenho */
		glutDisplayFunc(GraphicalOverview::display);
		glutIdleFunc(GraphicalOverview::display);
		glutReshapeFunc(GraphicalOverview::reshape);

		glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glLineWidth(2.0);

		/* Inicia loop principal da janela de informações */
		glutMainLoop();
	} else {
		/* Finaliza loop principal da janela de informações */
		glutLeaveMainLoop();
	}

	pthread_return(NULL);
}

void GraphicalOverview::display() {
	if (glutGetWindow() == 0) {
		return;
	}

	/* Limpa buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Reinicia o sistema de coordenadas */
	glLoadIdentity();

	/* Restaura a posicao da camera */
	gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);

	/* Desenha as informacoes na tela */
	float linha = 1.4;
	float coluna = -5;

	pthread_mutex_lock(&mutex_info);
	{
		for (list<HeuristicExecutionInfo*>::const_iterator iter = Heuristic::runningHeuristics->cbegin(); iter != Heuristic::runningHeuristics->cend() && STATUS == EXECUTING; iter++) {
			glColor3f(1.0f, 0.0f, 0.0f);
			GraphicalOverview::drawstr(coluna, linha + 0.4, GLUT_BITMAP_TIMES_ROMAN_24, "%s -> STATUS: %.2f %\n", (*iter)->heuristicInfo, (*iter)->status);

			glColor3f(0.0f, 1.0f, 0.0f);
			GraphicalOverview::drawstr(coluna, linha + 0.2, GLUT_BITMAP_HELVETICA_12, "Best Initial Solution: %.0f\t | \tBest Current Solution: %.0f\n\n", (*iter)->bestInitialFitness, (*iter)->bestActualFitness);

			glColor3f(0.0f, 0.0f, 1.0f);
			GraphicalOverview::drawstr(coluna, linha, GLUT_BITMAP_9_BY_15, (*iter)->execInfo);

			coluna += 3.4;

			if (coluna > 1.8) {
				coluna = -5;
				linha -= 1;
			}
		}
	}
	pthread_mutex_unlock(&mutex_info);

	glutSwapBuffers();

	if (STATUS != EXECUTING) {
		GraphicalOverview::asyncRun(NULL);
	}

	sleep_for(chrono::milliseconds(WINDOW_ANIMATION_UPDATE_INTERVAL));
}

void GraphicalOverview::reshape(GLint width, GLint height) {
	if (glutGetWindow() == 0) {
		return;
	}

	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float) width / height, 0.025, 25.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
}

void GraphicalOverview::drawstr(GLfloat x, GLfloat y, GLvoid *font_style, const char *format, ...) {
	if (glutGetWindow() == 0) {
		return;
	}

	if (format == NULL) {
		return;
	}

	va_list args;
	char buffer[512], *s;

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	glRasterPos2f(x, y);

	for (s = buffer; *s; s++) {
		glutBitmapCharacter(font_style, *s);
	}
}
