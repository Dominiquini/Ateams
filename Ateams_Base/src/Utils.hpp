#include <exception>
#include <chrono>
#include <random>
#include <string>

using namespace std;

#define RANDOM_TYPE -1

#ifndef _Utils_
#define _Utils_

int xRand();

int xRand(int min, int max);

string getExceptionMessage(exception_ptr &eptr);

#endif
