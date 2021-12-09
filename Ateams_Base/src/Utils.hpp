#include <exception>
#include <random>
#include <string>

using namespace std;

#ifndef _Utils_
#define _Utils_

int xRand();

int xRand(int min, int max);

string getExceptionMessage(exception_ptr &eptr);

#endif
