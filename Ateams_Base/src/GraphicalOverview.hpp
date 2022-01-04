#include <GL/freeglut.h>

#include "Control.hpp"

#ifndef _GRAPHICAL_OVERVIEW_
#define _GRAPHICAL_OVERVIEW_

extern pthread_mutex_t mutex_info;

extern volatile TerminationInfo STATUS;

class GraphicalOverview {
private:

	static void* asyncRun(void*);

	static void display();                                      		//Desenha a tela
	static void reshape(GLint, GLint);                             	 	//Redesenha a tela
	static void drawstr(GLfloat, GLfloat, GLvoid*, const char*, ...); 	//Desenha uma string na tela

	int *argc;
	char **argv;

public:

	GraphicalOverview(int*, char**);
	~GraphicalOverview();

	void run();

};

#endif
