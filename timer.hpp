#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <thread>

namespace Blurses {
class Timer {
	public:
		Timer() : _start_at(std::chrono::system_clock::now()) { }

		unsigned long getTime() {
			auto now = std::chrono::system_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now - _start_at).count();
		}

		void update() const {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

	private:
		std::chrono::time_point<std::chrono::system_clock> _start_at;
};
};

#endif
