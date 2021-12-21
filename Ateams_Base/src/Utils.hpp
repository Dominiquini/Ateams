#include <exception>
#include <chrono>
#include <random>
#include <string>

using namespace std;

/*
 * +N -> default_random_engine(N)
 * -1 -> default_random_engine(%TIME%)
 * -2 -> mt19937(%TIME%)
 * -3 -> random_device
 */
#define RANDOM_TYPE -1

#ifndef _Utils_
#define _Utils_

int xRand();

int xRand(int min, int max);

string getExceptionMessage(exception_ptr &eptr);

#endif
