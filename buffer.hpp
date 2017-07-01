#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vector>
#include "cell.hpp"
#include "cell_attributes.hpp"

class Buffer {
	public:
		Buffer(uint16_t width, uint16_t height)
			: _width(width)
			, _height(height)
			, _cursorX(0)
			, _cursorY(0) {
			std::cout << "\e[2J";
			_buffer.resize(width * height, Cell());
			_prev_buffer.resize(width * height, Cell());
		}

		Cell& get(uint16_t x, uint16_t y) {
			return _buffer[this->getIndex(x, y)];
		}

		void set(uint16_t x, uint16_t y, Cell cell) {
			if (outOfBounds(x, y)) {
				return;
			}

			const size_t index = this->getIndex(x, y);

			_buffer.at(index) = cell;
		}

		void redraw() {
			_buffer.assign(_width * _height, Cell());
			_prev_buffer.assign(_width * _height, Cell());
			std::cout << "\e[2J";
			print();
		}

		void print() {
			std::string buf;

			for (uint16_t y = 0; y < _height; y++) {
				std::pair<uint16_t, uint16_t> tainted = getTaintedIndexes(y);

				if (tainted.first == _width) {
					continue;
				}

				const uint16_t min_x = tainted.first;
				const uint16_t max_x = tainted.second;
				const size_t row_index = this->getIndex(0, y);
				std::string row_buf;

				const Cell *prev = 0;

				row_buf += "\033[0m\033[" + std::to_string(y + 1) + ";" + std::to_string(min_x + 1) + "H";

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
				_prev_buffer.assign(_buffer.begin(), _buffer.end());
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
		std::vector<Cell> _buffer, _prev_buffer;

		void printCell(std::string& str, const Cell& cell, const Cell* prev) const {
			toggle(str, cell.isItalic, prev && prev->isItalic, "\033[3m", "\033[23m");
			toggle(str, cell.isUnderline, prev && prev->isUnderline, "\033[4m", "\033[24m");

			std::string data = cell.data;

			if (data.length() == 1 && data[0] < 32) {
				const char value = 0x80 + data[0];
				data = "\xe2\x90";
				data += value;
			}

			if (prev == 0 || cell.bg != prev->bg) { data = cell.bg.bg() + data; }
			if (prev == 0 || cell.fg != prev->fg) { data = cell.fg.fg() + data; }

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

		std::pair<uint16_t, uint16_t> getTaintedIndexes(const uint16_t y) const {
			const size_t min = y * _width;
			const size_t max = min + _width;

			uint16_t min_x = _width;
			uint16_t max_x = _width;

			for (size_t i = min; i < max; i++) {
				if (_buffer[i] != _prev_buffer[i]) {
					min_x = i - min;
					break;
				}
			}

			for (size_t i = max; i >= min; i--) {
				if (_buffer[i] != _prev_buffer[i]) {
					max_x = i - min + 1;
					break;
				}
			}

			return {min_x, max_x};
		}

		bool outOfBounds(uint16_t x, uint16_t y) {
			if (x < 0) { return true; }
			if (y < 0) { return true; }
			if (x >= _width) { return true; }
			if (y >= _height) { return true; }
			return false;
		}

		uint32_t getIndex(uint16_t x, uint16_t y) {
			if (outOfBounds(x, y)) {
				std::cout << "OUT OF BOUNDS" << std::endl;
				throw "out of bounds";
			}
			return y * _width + x;
		}

		uint16_t round(float x) {
			return static_cast<uint16_t>(x + 0.5);
		}
};

#endif
