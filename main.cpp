#include <iostream>
#include <sys/ioctl.h>
#include "utfstring.hpp"
#include "cell_attributes.hpp"
#include "blurses.hpp"

int main() {
	utfstring text("");
	unsigned int cursor_position = 0;

	Blurses::start([&](Display &display, std::list<Key> keys, unsigned long ticks) -> bool {
		for (Key &key : keys) {
			switch (key.type) {
				case Key::DATA:
					text = text.substr(0, cursor_position) + key.data + text.substr(cursor_position, text.length() - cursor_position);
					cursor_position++;
					break;
				case Key::KEY_RETURN:
					text = "";
					cursor_position = 0;
					break;
				case Key::KEY_LEFT:
					if (cursor_position > 0) {
						cursor_position--;
					}
					break;
				case Key::KEY_RIGHT:
					if (cursor_position < text.length()) {
						cursor_position++;
					}
					break;
				case Key::KEY_BACKSPACE:
					if (cursor_position > 0) {
						text = text.substr(0, cursor_position - 1) + text.substr(cursor_position, text.length() - cursor_position);
						cursor_position--;
					}
					break;
				case Key::KEY_DELETE:
					text = text.substr(0, cursor_position) + text.substr(cursor_position + 1, text.length() - cursor_position);
					break;
				case Key::KEY_REDRAW:
					display.redraw();
					break;
			}
		}

		display.setCursorPosition(5 + cursor_position, 10);

		Primitives primitives = display.primitives();

		for (int lol = 5; lol < 15; lol++) { 
			primitives.line(lol, 5, lol + 10, 20, display.attr().bg(Color::rgb(ticks / 500.0 + lol / 5.0)));
		}

		for (float lol = 1; lol < 20; lol += 3) {
			primitives.circle(100, 10, lol, display.attr().bg(Color::rgb(ticks / (lol * 200.0))));
		}
		primitives.filledRect(95, 5, 105, 15, display.attr().bg(Color::rgb(ticks / 2000.0)));
		primitives.rect(95, 5, 105, 15, display.attr().bg(Color::rgb(ticks / 5000.0)));

		CellAttributes textAttrs(display.attr());
		textAttrs.fg(0xffffff).bg(0x000000);

		primitives.text(5, 10, text, textAttrs);
		primitives.text(5, 11, std::to_string(text.length()) + " ", CellAttributes(textAttrs).fg(Color::rgb(ticks / 200.0)));
		primitives.text(5, 12, text.substr(0, text.length()), CellAttributes(textAttrs).fg(Color::rgb(ticks / 400.0)).bg(Color::rgb(ticks / 800.0)));
		primitives.text(5, 13, std::to_string(ticks), textAttrs);

		CellAttributes textAttrs2(textAttrs);
		textAttrs2.fg(Color::rgb(ticks / 1000.0));

		int i = 0;

		for (utfstring ch : text.chars()) {
			int j = 0;

			for (char c : ch.str()) {
				const std::string s = std::to_string((int)c);
				primitives.text(30 + j, i, s, textAttrs2);
				j += s.length() + 1;
			}

			i++;
		}

		primitives.text(30, i, std::string(20, ' '), textAttrs);

		return true;
	});
}
