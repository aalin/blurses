#ifndef PRIMIVES_HPP
#define PRIMIVES_HPP

#include "buffer.hpp"
#include "utfstring.hpp"

class Primitives {
	public:
		Primitives(Display& buffer) : _display(buffer) { }

		void text(uint16_t x, uint16_t y, utfstring text, const CellAttributes &attrs) {
			if (y >= _display.height()) {
				return;
			}

			int i = 0;

			for (utfstring ch : text.chars()) {
				if (x + i >= _display.width()) {
					return;
				}

				Cell cell = _display.get(x + i, y);
				attrs.apply(cell);
				cell.data = ch.str();
				set(x + i, y, cell);
				i++;
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

		void rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const CellAttributes &attrs) {
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

		void filledRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const CellAttributes &attrs) {
			Cell cell(attrs.buildCell());

			if (x1 < x0) { std::swap(x0, x1); }
			if (y1 < y0) { std::swap(y0, y1); }

			for (uint16_t x = x0; x <= x1; x++) {
				for (uint16_t y = y0; y <= y1; y++) {
					set(x, y, cell);
				}
			}
		}

		void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const CellAttributes &attrs) {
			Cell cell(attrs.buildCell());

			bresenham(x0, y0, x1, y1, [&](uint16_t x, uint16_t y) {
				set(x, y, cell);
			});
		}

		void bresenham(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, std::function<void(uint16_t, uint16_t)> fn) {
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

			for (uint16_t x = x0; x <= x1; x++) {
				if (steep) {
					fn(y, x);
				} else {
					fn(x, y);
				}

				error += derror;

				if (error > dx) {
					y += y1 > y0 ? 1 : -1;
					error -= dx * 2;
				}
			}
		}

	private:
		Display& _display;

		void set(uint16_t x, uint16_t y, const Cell& cell) {
			_display.set(x, y, cell);
		}
};

#endif
