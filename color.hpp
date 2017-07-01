#ifndef COLOR_HPP
#define COLOR_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <cmath>

struct RealColor {
	enum TYPE {
		TrueColor,
		Color256,
		Color16
	} type;

	uint8_t r, g, b;

//	RealColor(const RealColor &other) : type(other.type), r(other.r), g(other.g), b(other.b) { }

	static RealColor white() {
		return {Color16, 0, 0, 7};
	}

	static RealColor black() {
		return {Color16, 0, 0, 0};
	}

	bool operator==(const RealColor &other) const {
		return type == other.type && b == other.b && g == other.g && r == other.r;
	}

	bool operator!=(const RealColor &other) const {
		return !(*this == other);
	}

	std::string fg() const {
		switch (this->type) {
			case TrueColor:
				return "\033[38;2;" + ansiTrueColor() + "m";
			case Color256:
				return "\033[38;5;" + std::to_string(this->b) + "m";
			case Color16:
				return color16(30);
		}
	}

	std::string bg() const {
		switch (this->type) {
			case TrueColor:
				return "\033[48;2;" + ansiTrueColor() + "m";
			case Color256:
				return "\033[48;5;" + std::to_string(this->b) + "m";
			case Color16:
				return color16(40);
		}
	}

private:
	std::string ansiTrueColor() const {
		return std::to_string(this->r) + ";" + std::to_string(this->g) + ";" + std::to_string(this->b);
	}

	std::string color16(uint16_t x) const {
		const uint16_t color = this->b;
		const uint16_t base = color < 8 ? x : x + 60;
		return "\033[" + std::to_string(base + (color % 8)) + "m";
	}
};

struct Color {
	Color(uint32_t rgb)
		: r((rgb >> 16) & 0xff)
		, g((rgb >>  8) & 0xff)
		, b((rgb >>  0) & 0xff) {}

	Color(uint8_t r, uint8_t g, uint8_t b)
		: r(r)
		, g(g)
		, b(b) {}

	static Color rgb(double i) {
		return Color(
			std::pow(std::sin(i + (0 / 3.0) * M_PI), 2) * 255,
			std::pow(std::sin(i + (1 / 3.0) * M_PI), 2) * 255,
			std::pow(std::sin(i + (2 / 3.0) * M_PI), 2) * 255
		);
	}

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
		virtual std::string fg(const Color &rgb) const = 0;
		virtual std::string bg(const Color &rgb) const = 0;
		virtual RealColor value(const Color &rgb) const = 0;

		virtual ~AbstractColor() {}
};

class TrueColor : public AbstractColor {
	public:
		std::string fg(const Color &rgb) const {
			return "\033[38;2;" + ansiTrueColor(rgb) + "m";
		}

		std::string bg(const Color &rgb) const {
			return "\033[48;2;" + ansiTrueColor(rgb) + "m";
		}

		RealColor value(const Color &rgb) const {
			return {RealColor::TrueColor, rgb.r, rgb.g, rgb.b};
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
		std::string fg(const Color &rgb) const {
			const uint16_t color = colorIndex(rgb);
			const uint16_t base = color < 8 ? 30 : 90;
			return "\033[" + std::to_string(base + (color % 8)) + "m";
		}

		std::string bg(const Color &rgb) const {
			const uint16_t color = colorIndex(rgb);
			const uint16_t base = color < 8 ? 40 : 100;
			return "\033[" + std::to_string(base + (color % 8)) + "m";
		}

		RealColor value(const Color &rgb) const {
			return {RealColor::Color16, 0, 0, colorIndex(rgb)};
		}

	private:
		uint8_t colorIndex(const Color& rgb) const {
			float distance = 9999.0;
			uint8_t index = 0;

			for (uint8_t i = 0; i < 16; i++) {
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
		std::string fg(const Color &rgb) const {
			return "\033[38;5;" + std::to_string(this->ansi256(rgb)) + "m";
		}

		std::string bg(const Color &rgb) const {
			return "\033[48;5;" + std::to_string(this->ansi256(rgb)) + "m";
		}

		RealColor value(const Color& rgb) const {
			return {RealColor::Color256, 0, 0, this->ansi256(rgb)};
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

		std::string fg(const Color &rgb) const { return _color->fg(rgb); }
		std::string bg(const Color &rgb) const { return _color->bg(rgb); }
		RealColor value(const Color &rgb) const { return _color->value(rgb); }

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
