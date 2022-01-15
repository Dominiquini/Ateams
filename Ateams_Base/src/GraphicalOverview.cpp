#include "GraphicalOverview.hpp"

GraphicalOverview::GraphicalOverview(Control *ctrl) {
	GraphicalOverview::ctrl = ctrl;

	if (GraphicalOverview::ctrl->showGraphicalOverview) {
		glutInit(GraphicalOverview::ctrl->argc, GraphicalOverview::ctrl->argv);
	}
}

GraphicalOverview::~GraphicalOverview() {
	if (GraphicalOverview::ctrl->showGraphicalOverview) {
		if (int window = glutGetWindow() != 0) {
			glutDestroyWindow(window);
		}
	}

	GraphicalOverview::ctrl = NULL;
}

void GraphicalOverview::run() {
	if (GraphicalOverview::ctrl->showGraphicalOverview) {
		thread threadAnimation(GraphicalOverview::asyncRun);

		threadAnimation.detach();
	}
}

void GraphicalOverview::asyncRun() {
	if (STATUS == EXECUTING) {
		GraphicalOverview::setup();

		/* Inicia loop principal da janela de informações */
		glutMainLoop();
	} else {
		/* Finaliza loop principal da janela de informações */
		glutLeaveMainLoop();
	}
}

void GraphicalOverview::setup() {
	strcpy(screen_title, FileUtils::getFileName(string(ctrl->argv[0])).c_str());

	/* Cria a tela */
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(0, 0);

	glutCreateWindow(screen_title);

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

	{
		scoped_lock<decltype(mutex_info)> lock_info(mutex_info);

		snprintf(graphical_buffer, GRAPHICAL_BUFFER_SIZE, "%s - Threads: %d | Iteration: %d", screen_title, ctrl->runningThreads, ctrl->executionCount);
		glutSetWindowTitle(graphical_buffer);

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

	glutSwapBuffers();

	if (STATUS != EXECUTING) {
		GraphicalOverview::asyncRun();
	}

	sleep_ms(WINDOW_ANIMATION_UPDATE_INTERVAL);
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
	if (glutGetWindow() == 0 || format == NULL) {
		return;
	}

	va_list args;
	char *s;

	va_start(args, format);
	vsnprintf(graphical_buffer, GRAPHICAL_BUFFER_SIZE, format, args);
	va_end(args);

	glRasterPos2f(x, y);

	for (s = graphical_buffer; *s; s++) {
		glutBitmapCharacter(font_style, *s);
	}
}

Control *GraphicalOverview::ctrl = NULL;

char GraphicalOverview::screen_title[SCREEN_TITLE_SIZE];
char GraphicalOverview::graphical_buffer[GRAPHICAL_BUFFER_SIZE];
