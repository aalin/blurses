#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vector>
#include <list>
#include "cell.hpp"
#include "cell_attributes.hpp"

class Buffer {
	typedef std::pair<uint16_t, uint16_t> Range;

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

		void redraw(bool showCursor) {
			_buffer.assign(_width * _height, Cell());
			_prev_buffer.assign(_width * _height, Cell());
			std::cout << "\e[2J";
			print(showCursor);
		}

		void print(bool showCursor) {
			std::string buf;

			for (uint16_t y = 0; y < _height; y++) {
				std::list<Range> ranges = getTaintedRanges(y);

				if (ranges.empty()) {
					continue;
				}

				const Cell *prev = 0;
				uint16_t prev_x = 0;
				std::string row_buf = "\033[0m";
				const size_t row_index = this->getIndex(0, y);
				row_buf += "\033[" + std::to_string(y + 1) + ";1H";

				for (const Range &range : ranges) {
					const uint16_t min_x = range.first;
					const uint16_t max_x = range.second;
					const uint16_t shift_right = min_x - prev_x;

					if (shift_right > 0) {
						row_buf += "\033[" + std::to_string(shift_right) + "C";
					}

					for (uint16_t x = min_x; x < max_x; x++) {
						const Cell &cell = _buffer[row_index + x];
						printCell(row_buf, cell, prev);
						prev = &(cell);
					}

					prev_x = max_x;
				}

				buf += row_buf;
			}

			if (buf.length() > 0) {
				std::cout << "\033[?25l";
				std::cout << buf;
				printCursor(showCursor);
				std::cout << std::flush;
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

		void printCursor(bool showCursor) const {
			std::cout << "\033[" << std::to_string(_cursorY + 1) << ";" << std::to_string(_cursorX + 1) << "H";

			if (showCursor) {
				std::cout << "\033[?25h";
			}
		}

		void toggle(std::string &str, bool curr, bool prev, std::string enable_code, std::string disable_code) const {
			if (curr && !prev) {
				str += enable_code;
			} else if (!curr && prev) {
				str += disable_code;
			}
		}

		std::list<Range> optimizeRanges(const std::list<Range> &ranges, const uint16_t threshold = 4) const {
			std::list<Range> results;
			Range *last = 0;

			for (Range range : ranges) {
				if (last && range.first - last->second <= threshold) {
					last->second = range.second;
				} else {
					results.push_back(range);
					last = &(results.back());
				}
			}

			return results;
		}

		std::list<Range> getTaintedRanges(const uint16_t y) const {
			const size_t min = y * _width;
			const size_t max = min + _width;

			std::list<Range> ranges;

			int16_t first = -1;
			int16_t last = -1;

			for (size_t i = min; i < max; i++) {
				if (_buffer[i] == _prev_buffer[i]) {
					if (first >= 0) {
						ranges.push_back({first, last});
						first = -1;
						last = -1;
					}
				} else {
					if (first < 0) {
						first = i - min;
					}

					last = i - min + 1;
				}
			}

			if (first >= 0) {
				ranges.push_back({first, _width});
			}

			return optimizeRanges(ranges);
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
