#include "Utils.hpp"

using namespace std;

int xRand() {
	return xRand(0, RAND_MAX);
}

int xRand(int min, int max) {
	random_device rd;
	mt19937_64 gen(rd());
	uniform_int_distribution<int> distr(min, max - 1);

	return distr(gen);
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
