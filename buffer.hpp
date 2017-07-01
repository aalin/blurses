#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vector>
#include "cell.hpp"
#include "utfstring.hpp"
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

			_buffer[index] = cell;
		}

		void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const CellAttributes &attrs) {
			bool steep = false;

			if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
				std::swap(x0, y0);
				std::swap(x1, y1);
				steep = true;
			}

			if (x0 > x1) {
				std::swap(x0, x1);
				std::swap(y0, y1);
			}

			int dx = x1 - x0;
			int dy = y1 - y0;

			int derror = std::abs(dy) * 2;
			int error = 0;
			uint16_t y = y0;

			Cell cell(attrs.buildCell());

			for (uint16_t x = x0; x <= x1; x++) {
				if (steep) {
					set(y, x, cell);
				} else {
					set(x, y, cell);
				}

				error += derror;

				if (error > dx) {
					y += y1 > y0 ? 1 : -1;
					error -= dx * 2;
				}
			}
		}

		void circle(uint16_t cx, uint16_t cy, float radius, const CellAttributes &attrs) {
			float detail = 32;

			for (uint16_t i = 0; i < detail; i++) {
				float a0 = i / detail;
				float x0 = cx + std::cos(a0 * M_PI * 2) * radius;
				float y0 = cy + std::sin(a0 * M_PI * 2) * radius / 2.0;
				float a1 = (i + 1) / detail;
				float x1 = cx + std::cos(a1 * M_PI * 2) * radius;
				float y1 = cy + std::sin(a1 * M_PI * 2) * radius / 2.0;
				line(round(x0), round(y0), round(x1), round(y1), attrs);
			}
		}

		void text(uint16_t x, uint16_t y, utfstring text, const CellAttributes &attrs) {
			if (y >= _height) {
				return;
			}

			int i = 0;

			for (utfstring ch : text.chars()) {
				if (x + i >= _width) {
					return;
				}

				Cell cell = this->get(x + i, y);
				attrs.apply(cell);
				cell.data = ch.str();
				this->set(x + i, y, cell);
				i++;
			}
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
				std::pair<bool, uint16_t> tainted = isRowTainted(y);

				if (!tainted.first) {
					continue;
				}

				const uint16_t min_x = tainted.second;
				const uint16_t max_x = getLastTaintedInRow(y);
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
		ColorWrapper _color;
		std::vector<Cell> _buffer, _prev_buffer;

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
				if (_buffer[i] != _prev_buffer[i]) {
					return {true, i - min};
				}
			}

			return {false, 0};
		}

		uint16_t getLastTaintedInRow(uint16_t y) {
			const size_t min = y * _width;
			const size_t max = min + _width;

			for (size_t i = max; i >= min; i--) {
				if (_buffer[i] != _prev_buffer[i]) {
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

		uint16_t round(float x) {
			return static_cast<uint16_t>(x + 0.5);
		}
};

#endif
