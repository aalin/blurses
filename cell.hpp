#ifndef CELL_HPP
#define CELL_HPP

#include <string>
#include "color.hpp"

namespace Blurses {
struct Cell {
	Cell() : Cell(RealColor::off(), RealColor::off(), " ", false, false) { }

	Cell(RealColor fg, RealColor bg, std::string data, bool isItalic, bool isUnderline)
		: fg(fg)
		, bg(bg)
		, data(data)
		, isItalic(isItalic),
		isUnderline(isUnderline) { }

	RealColor fg;
	RealColor bg;
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
};

#endif
