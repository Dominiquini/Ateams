#include <sys/types.h>
#include <sys/stat.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <pthread.h>
#include <semaphore.h>

#include <functional>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <vector>
#include <bitset>
#include <string>
#include <cctype>
#include <ctime>
#include <list>
#include <set>

#include "Utils.hpp"

using namespace std;

#ifndef _ATEAMS_
#define _ATEAMS_

enum Heuristics {
	SA, BT, AG
};

struct ExecHeuristicsInfo {
	bool tabu;
	bool genetic;
	bool annealing;
};

void terminate(int signal);

#endif
