#include <GL/freeglut.h>

#include "Control.hpp"

#ifndef _GRAPHICAL_OVERVIEW_
#define _GRAPHICAL_OVERVIEW_

#define SCREEN_TITLE_SIZE 64
#define GRAPHICAL_BUFFER_SIZE 512

#define WINDOW_WIDTH 1250
#define WINDOW_HEIGHT 500
#define WINDOW_ANIMATION_UPDATE_INTERVAL 100

extern mutex mutex_info;

class Control;

enum GraphicalOverviewStatus {
	STARTED, STOPPED
};

class GraphicalOverview {
private:

	static GraphicalOverview *instance;

	static char screen_title[SCREEN_TITLE_SIZE];

	static char graphical_buffer[GRAPHICAL_BUFFER_SIZE];

	static void asyncRun();

	static void setup();												// Cria e configura a tela
	static void display();                                      		// Desenha a tela
	static void reshape(GLint, GLint);                             	 	// Redesenha a tela
	static void drawstr(GLfloat, GLfloat, GLvoid*, const char*, ...); 	// Desenha uma string na tela

	GraphicalOverviewStatus status;

	Control *ctrl;

	GraphicalOverview(Control*);
	~GraphicalOverview();

public:

	static GraphicalOverview* getInstance(Control*);
	static void destroyInstance();

	void start();
	void stop();
};

#endif
