#ifndef CELL_ATTRIBUTES_HPP
#define CELL_ATTRIBUTES_HPP

#include "color.hpp"
#include "cell.hpp"

class CellAttributes {
	public:
		CellAttributes(const ColorWrapper &color) : _color(color), _fg(0xffffff), _bg(0x000000), _is_italic(false), _is_underline(false) {}

		CellAttributes(const CellAttributes &other)
			: _color(other._color)
			, _fg(other._fg)
			, _bg(other._bg)
			, _is_italic(other._is_italic)
			, _is_underline(other._is_underline)
			, _isset_fg(other._isset_fg)
			, _isset_bg(other._isset_bg)
			, _isset_is_italic(other._isset_is_italic)
			, _isset_is_underline(other._isset_is_underline) {}

		CellAttributes& fg(const Color &color) {
			_fg = color;
			_isset_fg = true;
			return *this;
		}

		CellAttributes& bg(const Color &color) {
			_bg = color;
			_isset_bg = true;
			return *this;
		}

		CellAttributes& italic(bool value) {
			_is_italic = value;
			_isset_is_italic = true;
			return *this;
		}

		CellAttributes& underline(bool value) {
			_is_underline = value;
			_isset_is_underline = true;
			return *this;
		}

		Cell buildCell() const {
			Cell cell;
			apply(cell);
			return cell;
		}

		Cell& apply(Cell& cell) const {
			if (_isset_fg) { cell.fg = _color.value(_fg); }
			if (_isset_bg) { cell.bg = _color.value(_bg); }
			if (_isset_is_italic) { cell.isItalic = _is_italic; }
			if (_isset_is_underline) { cell.isUnderline = _is_underline; }
			return cell;
		}

	private:
		const ColorWrapper& _color;
		Color _fg;
		Color _bg;
		bool _is_italic;
		bool _is_underline;
		bool _isset_fg;
		bool _isset_bg;
		bool _isset_is_italic;
		bool _isset_is_underline;
};

#endif
