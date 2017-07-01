#ifndef PRIMIVES_HPP
#define PRIMIVES_HPP

#include <functional>
#include "display.hpp"
#include "buffer.hpp"
#include "utfstring.hpp"
#include "graphics.hpp"

class Primitives {
	public:
		Primitives(Display& buffer) : _display(buffer) { }

		void text(uint16_t x, uint16_t y, utfstring text, const CellAttributes &attrs) const {
			if (y >= _display.height()) {
				return;
			}

			int i = 0;

			for (utfstring ch : text.chars()) {
				if (x + i >= _display.width()) {
					return;
				}

				putchar(x + i, y, ch.str(), attrs);
				i++;
			}
		}

		void putchar(uint16_t x, uint16_t y, std::string ch, const CellAttributes &attrs) const {
			if (x >= _display.width()) { return; }
			if (y >= _display.height()) { return; }

			Cell cell = _display.get(x, y);
			attrs.apply(cell);
			cell.data = ch;
			set(x, y, cell);
		}

		void circle(uint16_t cx, uint16_t cy, float radius, const CellAttributes &attrs) const {
			Cell cell(attrs.buildCell());

			Graphics::circle(cx, cy, radius, 16, [&](uint16_t x, uint16_t y) {
				set(x, y, cell);
			});
		}

		void rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const CellAttributes &attrs) const {
			Cell cell(attrs.buildCell());

			if (x1 < x0) { std::swap(x0, x1); }
			if (y1 < y0) { std::swap(y0, y1); }

			for (uint16_t x = x0; x <= x1; x++) {
				set(x, y0, cell);
				set(x, y1, cell);
			}

			for (uint16_t y = y0; y <= y1; y++) {
				set(x0, y, cell);
				set(x1, y, cell);
			}
		}

		void filledRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const CellAttributes &attrs) const {
			Cell cell(attrs.buildCell());

			if (x1 < x0) { std::swap(x0, x1); }
			if (y1 < y0) { std::swap(y0, y1); }

			for (uint16_t x = x0; x <= x1; x++) {
				for (uint16_t y = y0; y <= y1; y++) {
					set(x, y, cell);
				}
			}
		}

		void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const CellAttributes &attrs) const {
			Cell cell(attrs.buildCell());

			Graphics::bresenham(x0, y0, x1, y1, [&](uint16_t x, uint16_t y) {
				set(x, y, cell);
			});
		}

	private:
		Display& _display;

		void set(uint16_t x, uint16_t y, const Cell& cell) const {
			_display.set(x, y, cell);
		}
};

#endif
