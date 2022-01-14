#include <thread>
#include <mutex>
#include <condition_variable>

#ifndef _SEMAPHORE_
#define _SEMAPHORE_

template<typename Mutex, typename CondVar>
class basic_semaphore {
public:

	using native_handle_type = typename CondVar::native_handle_type;

	explicit basic_semaphore(unsigned int count = 0);
	basic_semaphore(const basic_semaphore&) = delete;
	basic_semaphore(basic_semaphore&&) = delete;
	basic_semaphore& operator=(const basic_semaphore&) = delete;
	basic_semaphore& operator=(basic_semaphore&&) = delete;

	unsigned int getCounter();
	bool isEnabled();
	void setup(int count);
	void notify();
	void wait();
	bool try_wait();
	template<class Rep, class Period>
	bool wait_for(const std::chrono::duration<Rep, Period> &d);
	template<class Clock, class Duration>
	bool wait_until(const std::chrono::time_point<Clock, Duration> &t);

	native_handle_type native_handle();

private:

	Mutex mMutex;
	CondVar mCondVar;

	unsigned int mCount = 0;
	bool mEnabled = false;
};

using semaphore = basic_semaphore<std::mutex, std::condition_variable>;

template<typename Mutex, typename CondVar>
basic_semaphore<Mutex, CondVar>::basic_semaphore(unsigned int count) : mCount { count }, mEnabled { true } {
}

template<typename Mutex, typename CondVar>
unsigned int basic_semaphore<Mutex, CondVar>::getCounter() {
	std::scoped_lock<Mutex> lock { mMutex };

	return mCount;
}

template<typename Mutex, typename CondVar>
bool basic_semaphore<Mutex, CondVar>::isEnabled() {
	std::scoped_lock<Mutex> lock { mMutex };

	return mEnabled;
}

template<typename Mutex, typename CondVar>
void basic_semaphore<Mutex, CondVar>::setup(int count) {
	std::scoped_lock<Mutex> lock { mMutex };

	mEnabled = count >= 0;
	mCount = std::max(0, count);

	mCondVar.notify_all();
}

template<typename Mutex, typename CondVar>
void basic_semaphore<Mutex, CondVar>::notify() {
	std::scoped_lock<Mutex> lock { mMutex };

	++mCount;

	mCondVar.notify_one();
}

template<typename Mutex, typename CondVar>
void basic_semaphore<Mutex, CondVar>::wait() {
	std::unique_lock<Mutex> lock { mMutex };

	mCondVar.wait(lock, [&] {
		return !mEnabled || mCount != 0;
	});

	--mCount;
}

template<typename Mutex, typename CondVar>
bool basic_semaphore<Mutex, CondVar>::try_wait() {
	std::scoped_lock<Mutex> lock { mMutex };

	if (!mEnabled || mCount != 0) {
		--mCount;

		return true;
	} else {
		return false;
	}
}

template<typename Mutex, typename CondVar>
template<class Rep, class Period>
bool basic_semaphore<Mutex, CondVar>::wait_for(const std::chrono::duration<Rep, Period> &d) {
	std::unique_lock<Mutex> lock { mMutex };

	auto finished = mCondVar.wait_for(lock, d, [&] {
		return !mEnabled || mCount != 0;
	});

	if (finished) {
		--mCount;
	}

	return finished;
}

template<typename Mutex, typename CondVar>
template<class Clock, class Duration>
bool basic_semaphore<Mutex, CondVar>::wait_until(const std::chrono::time_point<Clock, Duration> &t) {
	std::unique_lock<Mutex> lock { mMutex };

	auto finished = mCondVar.wait_until(lock, t, [&] {
		return !mEnabled || mCount != 0;
	});

	if (finished) {
		--mCount;
	}

	return finished;
}

template<typename Mutex, typename CondVar>
typename basic_semaphore<Mutex, CondVar>::native_handle_type basic_semaphore<Mutex, CondVar>::native_handle() {
	return mCondVar.native_handle();
}

#endif
