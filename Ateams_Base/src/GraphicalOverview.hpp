#include <GL/freeglut.h>

#include "Control.hpp"

#ifndef _GRAPHICAL_OVERVIEW_
#define _GRAPHICAL_OVERVIEW_

#define GRAPHICAL_BUFFER_SIZE 64

#define WINDOW_WIDTH 1250
#define WINDOW_HEIGHT 500
#define WINDOW_ANIMATION_UPDATE_INTERVAL 100

extern mutex mutex_info;

extern volatile TerminationInfo STATUS;

class Control;

class GraphicalOverview {
private:

	static char graphical_buffer[GRAPHICAL_BUFFER_SIZE];

	static void asyncRun(Control*);

	static void setup(Control *);											// Cria e configura a tela
	static void display();                                      		// Desenha a tela
	static void reshape(GLint, GLint);                             	 	// Redesenha a tela
	static void drawstr(GLfloat, GLfloat, GLvoid*, const char*, ...); 	// Desenha uma string na tela

private:

	Control *ctrl;

public:

	GraphicalOverview(Control*);
	~GraphicalOverview();

	void run();
};

#endif
