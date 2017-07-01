#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "buffer.hpp"

class Display {
	public:
		Display() : _width(0), _height(0), _buffer(0) {
			std::cout << "\033[?1047h\033[H\033[J";
		}

		~Display() {
			if (_buffer) {
				delete _buffer;
			}

			std::cout << "\033[?25h\033[?1047l";
		}

		void redraw() {
			if (_buffer) {
				_buffer->redraw();
			}
		}

		void update() {
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &_winsize);

			const bool width_changed = _width != _winsize.ws_col;
			const bool height_changed = _height != _winsize.ws_row;

			if (width_changed || height_changed) {
				this->resize(_winsize.ws_col, _winsize.ws_row);
			}
		}

		uint16_t height() const {
			return _height;
		}

		Buffer& getBuffer() {
			return *_buffer;
		}

	private:
		struct winsize _winsize;
		uint16_t _width;
		uint16_t _height;
		Buffer *_buffer;

		void resize(uint16_t width, uint16_t height) {
			_width = width;
			_height = height;

			if (this->_buffer) {
				delete this->_buffer;
			}

			this->_buffer = new Buffer(width, height);
		}
};

#endif
