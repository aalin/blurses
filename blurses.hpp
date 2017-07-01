#ifndef BLURSES_HPP
#define BLURSES_HPP

#include <csignal>
#include <functional>
#include "timer.hpp"
#include "display.hpp"
#include "input.hpp"

class Blurses {
	public:
		Blurses() {
			_running = false;
		}

		void run(std::function<bool(Display&, std::list<Key>, unsigned long)> fn) {
			_running = true;
			_input.run();

			while (_running) {
				_display.update();
				_running = fn(_display, _input.getBuffer(), _timer.getTime());
				_display.draw();
				_timer.update();
			}
		}

		void stop() {
			_running = false;
		}

		static void start(std::function<bool(Display&, std::list<Key>, unsigned long)> fn) {
			if (_instance) {
				throw "Already instantiated";
			}

			_instance = new Blurses();
			std::signal(SIGINT, &Blurses::handleSigint);
			_instance->run(fn);
		}

		static void handleSigint(int signum __attribute__((unused))) {
			if (_instance) {
				_instance->stop();
			}

			delete _instance;
		}

	private:
		static Blurses *_instance;
		bool _running;
		Display _display;
		Input _input;
		Timer _timer;
};

Blurses *Blurses::_instance;

#endif
