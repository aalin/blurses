#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <functional>
#include <cmath>

namespace Graphics {
	typedef std::function<void(uint16_t, uint16_t)> callback_func;

	void bresenham(int x0, int y0, int x1, int y1, callback_func fn) {
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
		int y = y0;

		for (int x = x0; x <= x1; x++) {
			if (x >= 0 && y >= 0) {
				if (steep) {
					fn(y, x);
				} else {
					fn(x, y);
				}
			}

			error += derror;

			if (error > dx) {
				y += y1 > y0 ? 1 : -1;
				error -= dx * 2;
			}
		}
	}

	static void circle(uint16_t cx, uint16_t cy, float radius, float detail, callback_func fn, float y_multiply = 0.5) {
		for (uint16_t i = 0; i < detail; i++) {
			float a0 = i / detail;
			float x0 = cx + std::cos(a0 * M_PI * 2) * radius;
			float y0 = cy + std::sin(a0 * M_PI * 2) * radius * y_multiply;
			float a1 = (i + 1) / detail;
			float x1 = cx + std::cos(a1 * M_PI * 2) * radius;
			float y1 = cy + std::sin(a1 * M_PI * 2) * radius * y_multiply;
			bresenham(round(x0), round(y0), round(x1), round(y1), fn);
		}
	}
};

#endif
