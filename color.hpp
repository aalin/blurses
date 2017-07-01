#ifndef COLOR_HPP
#define COLOR_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <cmath>

struct Color {
	Color(uint32_t rgb)
		: r((rgb >> 16) & 0xff)
		, g((rgb >>  8) & 0xff)
		, b((rgb >>  0) & 0xff) {}

	Color(uint8_t r, uint8_t g, uint8_t b)
		: r(r)
		, g(g)
		, b(b) {}

	bool operator==(const Color &other) const {
		return other.r == r && other.g == g && other.b == b;
	}

	bool operator!=(const Color &other) const {
		return !(*this == other);
	}

	float distance(const Color &c) const {
		return std::sqrt(
			std::pow((r - c.r), 2) +
			std::pow((g - c.g), 2) +
			std::pow((b - c.b), 2)
		);
	}

	uint8_t r;
	uint8_t g;
	uint8_t b;
};

class AbstractColor {
	public:
		virtual std::string fg(const Color &rgb, const std::string &str) const = 0;
		virtual std::string bg(const Color &rgb, const std::string &str) const = 0;

		virtual ~AbstractColor() {}
};

class TrueColor : public AbstractColor {
	public:
		std::string fg(const Color &rgb, const std::string &str) const {
			return "\033[38;2;" + ansiTrueColor(rgb) + "m" + str;
		}

		std::string bg(const Color &rgb, const std::string &str) const {
			return "\033[48;2;" + ansiTrueColor(rgb) + "m" + str;
		}

	private:
		std::string ansiTrueColor(const Color &rgb) const {
			return std::to_string(rgb.r) + ";" + std::to_string(rgb.g) + ";" + std::to_string(rgb.b);
		}
};

const Color COLORS[] = {
	0x000000,
	0xaa0000,
	0x00aa00,
	0xaa5500,
	0x0000aa,
	0xaa00aa,
	0x00aaaa,
	0xaaaaaa,
	0x555555,
	0xff5555,
	0x55ff55,
	0xffff55,
	0x5555ff,
	0xff55ff,
	0x55ffff,
	0xffffff
};

class Color16 : public AbstractColor {
	public:
		std::string fg(const Color &rgb, const std::string &str) const {
			const uint16_t color = colorIndex(rgb);
			return intensify("\033[" + std::to_string(30 + (color % 8)) + "m" + str, color);
		}

		std::string bg(const Color &rgb, const std::string &str) const {
			const uint16_t color = colorIndex(rgb);
			return "\033[" + std::to_string(40 + (color % 8)) + "m" + str;
		}

	private:
		std::string intensify(std::string str, uint16_t color) const {
			if (color < 8) {
				return str;
			}

			return "\033[1m" + str + "\033[22m";
		}

		uint16_t colorIndex(const Color& rgb) const {
			float distance = 9999.0;
			uint16_t index = 0;

			for (int i = 0; i < 16; i++) {
				const float dist = rgb.distance(COLORS[i]);

				if (dist < distance) {
					distance = dist;
					index = i;
				}
			}

			return index;
		}
};

class Color256 : public AbstractColor {
	public:
		std::string fg(const Color &rgb, const std::string &str) const {
			return "\033[38;5;" + std::to_string(this->ansi256(rgb)) + "m" + str;
		}

		std::string bg(const Color &rgb, const std::string &str) const {
			return "\033[48;5;" + std::to_string(this->ansi256(rgb)) + "m" + str;
		}

	private:
		uint8_t ansi256(const Color &rgb) const {
			if (rgb.r == 0 && rgb.g == 0 && rgb.b == 0) {
				return 0;
			}

			if (this->isGray(rgb.r, rgb.g, rgb.b)) {
				return 232 + ((rgb.r + rgb.g + rgb.b) / 33.0);
			}

			return 16 +
				(int)(6 * rgb.r / 256.0) * 36 +
				(int)(6 * rgb.g / 256.0) * 6 +
				(int)(6 * rgb.b / 256.0);
		}

		bool isGray(uint8_t r, uint8_t g, uint8_t b, float sep = 0.0) const {
			if (r < sep || g < sep || b < sep) {
				return (r < sep && g < sep && b < sep);
			}

			return isGray(r, g, b, sep + 42.5);
		}
};

class ColorWrapper {
	public:
		ColorWrapper() {
			if (std::string(std::getenv("TERM_PROGRAM")) == "iTerm.app") {
				_color = new TrueColor();
			} else if (ends_with(std::getenv("TERM"), "256color")) {
				_color = new Color256();
			} else {
				_color = new Color16();
			}
		}

		~ColorWrapper() {
			delete _color;
		}

		std::string fg(const Color &rgb, const std::string &str) const { return _color->fg(rgb, str); }
		std::string bg(const Color &rgb, const std::string &str) const { return _color->bg(rgb, str); }

	private:
		AbstractColor *_color;

		bool ends_with(const std::string &value, const std::string &ending) {
			if (ending.size() > value.size()) {
				return false;
			}

			return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
		}

};

#endif
