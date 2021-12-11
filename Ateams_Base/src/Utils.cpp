#include "Utils.hpp"

#include "Control.hpp"

using namespace std;

static random_device randomDevice;

int xRand() {
	return xRand(0, RAND_MAX);
}

int xRand(int min, int max) {
	mt19937_64 randomNumberGenerator(randomDevice());
	uniform_int_distribution<int> randomDistribution(min, max - 1);

	return randomDistribution(randomNumberGenerator);
}

string getExceptionMessage(exception_ptr &eptr) {
	try {
		rethrow_exception(eptr);
	} catch (const exception &e) {
		return e.what();
	} catch (const string &e) {
		return e;
	} catch (const char *e) {
		return e;
	} catch (...) {
		return (current_exception() ? current_exception().__cxa_exception_type()->name() : "Unexpected Exception!");
	}
}
