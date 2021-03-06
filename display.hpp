#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "buffer.hpp"
#include "cell_attributes.hpp"

namespace Blurses {
class Primitives;

class Display {
	public:
		Display();
		~Display();

		void redraw() {
			if (_buffer) {
				_buffer->redraw(_showCursor);
			}
		}

		void draw() {
			if (_buffer) {
				_buffer->print(_showCursor);
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

		void setCursorPosition(uint16_t x, uint16_t y) {
			getBuffer().setCursorPosition(x, y);
		}

		const Primitives& primitives() {
			return *_primitives;
		}

		 CellAttributes attr() {
			 return CellAttributes(_color);
		 }

		 void showCursor() {
			 // TODO: _show_cursor is not used anywhere.
			 _showCursor = true;
		 }

		 void hideCursor() {
			 _showCursor = false;
		 }

		 RealColor color(const Color& rgb) const {
			 return _color.value(rgb);
		 }

	private:
		struct winsize _winsize;
		uint16_t _width;
		uint16_t _height;
		Buffer *_buffer;
		Primitives *_primitives;
		ColorWrapper _color;
		bool _showCursor;

		void resize(uint16_t width, uint16_t height) {
			_width = width;
			_height = height;

			if (this->_buffer) {
				delete this->_buffer;
			}

			this->_buffer = new Buffer(width, height);
		}

		Buffer& getBuffer() {
			return *_buffer;
		}
};
};

#include "primitives.hpp"

namespace Blurses {
Display::Display() : _width(0), _height(0), _buffer(0), _showCursor(true) {
	_primitives = new Blurses::Primitives(*this);
	std::cout << "\033[?1047h\033[H\033[J";
}

Display::~Display() {
	delete _primitives;

	if (_buffer) {
		delete _buffer;
	}

	std::cout << "\033[?25h\033[?1047l\033[2J" << std::flush;
}
};

#endif
