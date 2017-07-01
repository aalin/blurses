#ifndef CELL_HPP
#define CELL_HPP

#include <string>
#include "color.hpp"

struct Cell {
	Cell() : Cell(0xffffff, 0x000000, " ", false, false) { }

	Cell(Color fg, Color bg, std::string data, bool isItalic, bool isUnderline)
		: fg(fg), bg(bg), data(data), isItalic(isItalic), isUnderline(isUnderline) {
	}

	Color fg;
	Color bg;
	std::string data;
	bool isItalic:1;
	bool isUnderline:2;

	Cell& operator=(const Cell &other) {
		this->fg = other.fg;
		this->bg = other.bg;
		this->data = other.data;
		this->isItalic = other.isItalic;
		this->isUnderline = other.isUnderline;
		return *this;
	}

	bool operator==(const Cell &other) const {
		return (
			this->fg == other.fg &&
			this->bg == other.bg &&
			this->data == other.data &&
			this->isItalic == other.isItalic &&
			this->isUnderline == other.isUnderline
	   );
	}

	bool operator!=(const Cell &other) const {
		return !(other == *this);
	}
};

#endif
