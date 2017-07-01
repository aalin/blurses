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

namespace Blurses {
class Input {
	class InputState {
		public:
			InputState(Input& input) : _input(input) {
				reset();
			}

			bool handle(const char *str, size_t len) {
				const char c = str[0];

				if (c == 0x1b) {
					if (len > 1) {
						_escape_count++;
						return true;
					}

					reset();
					_input.pushBuffer(Key(Key::KEY_ESCAPE));
					return true;
				}

				if (_escape_count && _in_bracket) {
					if (c >= '0' && c <= '9') {
						_vt[strlen(_vt) % sizeof _vt] = c;
						return true;
					}

					if (c == '~') {
						switch (atoi(_vt)) {
							case 1: _input.pushBuffer(Key(Key::KEY_HOME)); break;
							case 3: _input.pushBuffer(Key(Key::KEY_DELETE)); break;
							case 4: _input.pushBuffer(Key(Key::KEY_END)); break;
						}
					} else {
						if (_escape_count == 2) {
								switch (c) {
									case 'D': _input.pushBuffer(Key(Key::KEY_HOME)); break;
									case 'C': _input.pushBuffer(Key(Key::KEY_END)); break;
								}
						} else {
							switch (c) {
								case 'A': _input.pushBuffer(Key(Key::KEY_UP)); break;
								case 'B': _input.pushBuffer(Key(Key::KEY_DOWN)); break;
								case 'D': _input.pushBuffer(Key(Key::KEY_LEFT)); break;
								case 'C': _input.pushBuffer(Key(Key::KEY_RIGHT)); break;
								case 'Z': _input.pushBuffer(Key(Key::KEY_TAB_BACK)); break;
							}
						}
					}

					reset();
					return true;
				}

				if (_escape_count && !_in_bracket && c == '[') {
					_in_bracket = true;
					return true;
				}

				reset();
				return false;
			}

			void reset() {
				_escape_count = 0;
				_in_bracket = false;
				std::memset(_vt, 0, sizeof _vt);
			}

		private:
			Input &_input;
			uint8_t _escape_count;
			bool _in_bracket;
			char _vt[3];
	};

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

				char buffer[32];
				size_t buflen = 0;

				InputState state(*this);

				while (_running) {
					buflen = ::read(0, &buffer, sizeof buffer);

					for (size_t i = 0; i < buflen; i++) {
						char c = buffer[i];

						if (handleAscii(c)) {
							continue;
						}

						if (state.handle(buffer + i, buflen - i)) {
							continue;
						}

						state.reset();

						str += c;

						if (!utfstring::is_valid(str)) {
							continue;
						}

						for (utfstring &ch : utfstring(str).chars()) {
							pushBuffer(Key(ch.str()));
						}

						str = "";
					}
				}
			});
		}

		std::list<Key> getBuffer() {
			std::lock_guard<std::mutex> guard(_buffer_mutex);
			std::list<Key> buffer = _buffer;
			_buffer.clear();
			return buffer;
		}

		void pushBuffer(Key key) {
			std::lock_guard<std::mutex> guard(_buffer_mutex);
			_buffer.push_back(key);
		}

	private:
		termios _old_termios;
		std::list<Key> _buffer;
		std::mutex _buffer_mutex;
		std::thread *_th;
		bool _running;

		bool handleAscii(const char c) {
			switch (c) {
				case 0x7f:
						pushBuffer(Key(Key::KEY_BACKSPACE));
						return true;
				case 0x0a:
						pushBuffer(Key(Key::KEY_RETURN));
						return true;
				case 0x09:
						pushBuffer(Key(Key::KEY_TAB));
						return true;
				case 0x18:
						pushBuffer(Key(Key::KEY_CANCEL));
						return true;
				case 0x0c:
						pushBuffer(Key(Key::KEY_REDRAW));
						return true;
				default:
						return false;
			}
		}
};
};

#endif
