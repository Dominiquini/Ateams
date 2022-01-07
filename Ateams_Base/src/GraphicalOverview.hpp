#include <GL/freeglut.h>

#include "Control.hpp"

#ifndef _GRAPHICAL_OVERVIEW_
#define _GRAPHICAL_OVERVIEW_

#define GRAPHICAL_BUFFER_SIZE 64

extern pthread_mutex_t mutex_info;

extern volatile TerminationInfo STATUS;

class GraphicalOverview {
private:

	static char graphical_buffer[GRAPHICAL_BUFFER_SIZE];

	static void* asyncRun(void*);

	static void setup(char*);											// Cria e configura a tela
	static void display();                                      		// Desenha a tela
	static void reshape(GLint, GLint);                             	 	// Redesenha a tela
	static void drawstr(GLfloat, GLfloat, GLvoid*, const char*, ...); 	// Desenha uma string na tela

private:

	bool enabled;

	int *argc;
	char **argv;

public:

	GraphicalOverview(bool, int*, char**);
	~GraphicalOverview();

	void run();
};

#endif
