#ifndef BRAILLE_BUFFER_HPP
#define BRAILLE_BUFFER_HPP

#include <vector>
#include <list>
#include <string>
#include <cmath>
#include "utfstring.hpp"
#include "graphics.hpp"

class BrailleBuffer {
	public:
		BrailleBuffer(uint16_t width, uint16_t height)
		: _width(width)
		, _height(height) {
			_buffer.resize(_width * _height, false);
		}

		BrailleBuffer& set(uint16_t x, uint16_t y, bool value) {
			if (x >= _width) { return *this; }
			if (y >= _height) { return *this; }
			_buffer[y * _width + x] = value;
			return *this;
		}

		void clear() {
			_buffer.assign(_width * _height, false);
		}

		void flip() {
			_buffer.flip();
		}

		void circle(uint16_t cx, uint16_t cy, float radius) {
			Graphics::circle(cx, cy, radius, 16, [&](uint16_t x, uint16_t y) {
				set(x, y, true);
			}, 1.0);
		}

		void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
			Graphics::bresenham(x0, y0, x1, y1, [&](uint16_t x, uint16_t y) {
				set(x, y, true);
			});
		}

		std::list<std::list<std::string> > lines() {
			std::list<std::list<std::string> > lines;

			for (size_t i = 0; i < std::ceil(_height / 4.0); i++) {
				std::vector<uint32_t> values(_width / 2, 0);

				for (size_t yc = 0; yc < 4; yc++) {
					const size_t y = i * 4 + yc;

					if (y >= _height) {
						break;
					}

					for (size_t x = 0; x < _width; x++) {
						if (_buffer.at(y * _width + x)) {
							if (yc == 3) {
								values[x / 2] |= 0x40 << (x % 2);
							} else {
								values[x / 2] |= (1 << (x % 2) * 3) << yc;
							}
						}
					}
				}

				std::list<std::string> line;

				for (uint32_t value : values) {
					line.push_back(utfstring::decode(0x2800 + value).str());
				}

				lines.push_back(line);
			}

			return lines;
		}

	private:
		const uint16_t _width;
		const uint16_t _height;
		std::vector<bool> _buffer;
};

#endif
