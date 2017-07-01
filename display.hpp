#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "buffer.hpp"

class Primitives;

class Display {
	public:
		Display();

		~Display() {
			delete _primitives;

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

		void draw() {
			if (_buffer) {
				_buffer->print();
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

		uint16_t width() const {
			return _width;
		}

		uint16_t height() const {
			return _height;
		}

		void set(uint16_t x, uint16_t y, const Cell& cell) {
			getBuffer().set(x, y, cell);
		}

		Cell get(uint16_t x, uint16_t y) {
			return getBuffer().get(x, y);
		}

		Buffer& getBuffer() {
			return *_buffer;
		}

	private:
		struct winsize _winsize;
		uint16_t _width;
		uint16_t _height;
		Buffer *_buffer;
		Primitives *_primitives;

		void resize(uint16_t width, uint16_t height) {
			_width = width;
			_height = height;

			if (this->_buffer) {
				delete this->_buffer;
			}

			this->_buffer = new Buffer(width, height);
		}
};

#include "primitives.hpp"

Display::Display() : _width(0), _height(0), _buffer(0) {
	_primitives = new Primitives(*this);
	std::cout << "\033[?1047h\033[H\033[J";
}

#endif
