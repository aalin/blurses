#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vector>
#include "cell.hpp"
#include "utfstring.hpp"

class Buffer {
	public:
		Buffer(uint16_t width, uint16_t height)
			: _width(width)
			, _height(height)
			, _cursorX(0)
			, _cursorY(0) {
			std::cout << "\e[2J";
			_buffer.resize(width * height, Cell());
			_buffer2.resize(width * height, Cell());
		}

		Cell& get(uint16_t x, uint16_t y) {
			return _buffer[this->getIndex(x, y)];
		}

		Cell& get2(uint16_t x, uint16_t y) {
			return _buffer2[this->getIndex(x, y)];
		}

		void set(uint16_t x, uint16_t y, Cell cell) {
			if (outOfBounds(x, y)) {
				return;
			}

			const size_t index = this->getIndex(x, y);

			_buffer[index] = cell;
		}

		void text(uint16_t x, uint16_t y, utfstring text, Color fg = 0xffffff, Color bg = 0x000000) {
			if (y >= _height) {
				return;
			}

			int i = 0;

			for (utfstring ch : text.chars()) {
				if (x + i >= _width) {
					return;
				}

				Cell cell = this->get(x + i, y);
				cell.fg = fg;
				cell.bg = bg;
				cell.data = ch.str();
				this->set(x + i, y, cell);
				i++;
			}
		}

		void redraw() {
			_buffer.assign(_width * _height, Cell());
			_buffer2.assign(_width * _height, Cell());
			std::cout << "\e[2J";
			print();
		}

		void print() {
			std::string buf;

			for (uint16_t y = 0; y < _height; y++) {
				std::pair<bool, uint16_t> tainted = isRowTainted(y);

				if (!tainted.first) {
					continue;
				}

				const uint16_t min_x = tainted.second;
				const uint16_t max_x = getLastTaintedInRow(y);
				const size_t row_index = this->getIndex(0, y);
				std::string row_buf;

				const Cell *prev = 0;

				row_buf += "\e[0m";
				row_buf += "\033[" + std::to_string(y + 1) + ";" + std::to_string(min_x + 1) + "H";

				for (uint16_t x = min_x; x < max_x; x++) {
					const Cell &cell = _buffer[row_index + x];
					printCell(row_buf, cell, prev);
					prev = &(cell);
				}

				buf += row_buf;
			}

			if (buf.length() > 0) {
				std::cout << "\033[?25l";
				std::cout << buf;
				printCursor();
				std::cout << "\033[?25h" << std::flush;
				_buffer2.assign(_buffer.begin(), _buffer.end());
				_buffer.assign(_width * _height, Cell());
			}
		}

		void setCursorPosition(uint16_t x, uint16_t y) {
			_cursorX = x;
			_cursorY = y;
		}

	private:
		const uint16_t _width;
		const uint16_t _height;
		uint16_t _cursorX;
		uint16_t _cursorY;
		ColorWrapper _color;
		std::vector<Cell> _buffer, _buffer2;

		void printCell(std::string& str, const Cell& cell, const Cell* prev) const {
			toggle(str, cell.isItalic, prev && prev->isItalic, "\e[3m", "\e[23m");
			toggle(str, cell.isUnderline, prev && prev->isUnderline, "\e[4m", "\e[24m");

			std::string data = cell.data;

			if (prev == 0 || cell.bg != prev->bg) { data = _color.bg(cell.bg, data); }
			if (prev == 0 || cell.fg != prev->fg) { data = _color.fg(cell.fg, data); }

			str += data;
		}

		void printCursor() const {
			std::cout << "\033[" << std::to_string(_cursorY + 1) << ";" << std::to_string(_cursorX + 1) << "H";
		}

		void toggle(std::string &str, bool curr, bool prev, std::string enable_code, std::string disable_code) const {
			if (curr && !prev) {
				str += enable_code;
			} else if (!curr && prev) {
				str += disable_code;
			}
		}

		std::pair<bool, uint16_t> isRowTainted(uint16_t y) {
			const size_t min = y * _width;
			const size_t max = min + _width;

			for (size_t i = min; i < max; i++) {
				if (_buffer[i] != _buffer2[i]) {
					return {true, i - min};
				}
			}

			return {false, 0};
		}

		uint16_t getLastTaintedInRow(uint16_t y) {
			const size_t min = y * _width;
			const size_t max = min + _width;

			for (size_t i = max; i >= min; i--) {
				if (_buffer[i] != _buffer2[i]) {
					return i - min + 1;
				}
			}

			return 0;
		}

		bool outOfBounds(uint16_t x, uint16_t y) {
			if (x > _width) { return true; }
			if (y > _height) { return true; }
			return false;
		}

		uint32_t getIndex(uint16_t x, uint16_t y) {
			if (outOfBounds(x, y)) {
				std::cout << "OUT OF BOUNDS" << std::endl;
				throw "out of bounds";
			}
			return y * _width + x;
		}
};

#endif
