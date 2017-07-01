#ifndef INPUT_HPP
#define INPUT_HPP

#include <unistd.h>
#include <termios.h>
#include <list>
#include <thread>
#include <iostream>
#include <locale>
#include "utfstring.hpp"
#include "key.hpp"

class Input {
	public:
		Input() : _th(nullptr) {
			_running = false;
			tcgetattr(0, &this->_old_termios);
			termios settings = this->_old_termios;
			settings.c_lflag &= ~ICANON; // disable buffered io
			settings.c_lflag &= ~ECHO; // disable echo mode
			tcsetattr(0, TCSANOW, &settings);

			std::ios_base::sync_with_stdio(false);
			std::wcin.imbue(std::locale("en_US.UTF-8"));
			std::wcout.imbue(std::locale("en_US.UTF-8"));
		}

		~Input() {
			tcsetattr(0, TCSANOW, &this->_old_termios);

			if (_th == nullptr) {
				_running = false;
			}
		}

		void run() {
			_running = true;

			_th = new std::thread([this]() {
				std::string str;
				str.reserve(128);

				bool in_escape = false;
				bool in_bracket = false;

				while (_running) {
					const uint8_t c = read();

					if (c == 0x7e) {
						pushBuffer(Key(Key::KEY_DELETE));
						continue;
					}

					if (c == 0x7f) {
						pushBuffer(Key(Key::KEY_BACKSPACE));
						continue;
					}

					if (c == 0x0a) {
						pushBuffer(Key(Key::KEY_RETURN));
						continue;
					}

					if (c == 0x18) {
						pushBuffer(Key(Key::KEY_CANCEL));
						continue;
					}

					if (c == 0x0c) {
						pushBuffer(Key(Key::KEY_REDRAW));
						continue;
					}

					if (c == 0x1b) {
						in_escape = true;
						continue;
					}

					if (in_escape && c == '[') {
						in_bracket = true;
						continue;
					}

					if (in_escape) {
						if (in_bracket) {
							switch (c) {
								case 'A': pushBuffer(Key(Key::KEY_UP)); break;
								case 'B': pushBuffer(Key(Key::KEY_UP)); break;
								case 'D': pushBuffer(Key(Key::KEY_LEFT)); break;
								case 'C': pushBuffer(Key(Key::KEY_RIGHT)); break;
							}
						} else {
							pushBuffer(Key(Key::KEY_DELETE));
						}

						in_escape = false;
						in_bracket = false;
						continue;
					}

					in_escape = false;
					in_bracket = false;

					str += c;

					if (!utfstring::is_valid(str)) {
						continue;
					}

					for (utfstring &ch : utfstring(str).chars()) {
						pushBuffer(Key(ch.str()));
					}

					str = "";
				}
			});
		}

		std::list<Key> getBuffer() {
			std::lock_guard<std::mutex> guard(_buffer_mutex);
			std::list<Key> buffer = _buffer;
			_buffer.clear();
			return buffer;
		}

	private:
		termios _old_termios;
		std::list<Key> _buffer;
		std::mutex _buffer_mutex;
		std::thread *_th;
		bool _running;

		char read() {
			char c;
			::read(0, &c, 1);
			return c;
		}

		void pushBuffer(Key key) {
			std::lock_guard<std::mutex> guard(_buffer_mutex);
			_buffer.push_back(key);
		}
};

#endif
