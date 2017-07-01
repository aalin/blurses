#include <string>
#include <vector>
#include <iostream>
#include <locale>
#include <codecvt>
#include <sys/ioctl.h>
#include <thread>
#include <csignal>
#include <functional>
#include "input.hpp"
#include "utfstring.hpp"
#include "display.hpp"
#include "cell_attributes.hpp"
#include "timer.hpp"
#include <cmath>

bool running = true;

void handleSigint(int signum __attribute__((unused))) {
	running = false;
}

const char* BACKSPACE = "\x7f";
const char* REDRAW = "\x0c";

int main() {
	Display display;
	Input input;
	Timer timer;
	input.run();

	std::signal(SIGINT, &handleSigint);

	utfstring text("");
	unsigned int cursor_position = 0;

	while (running) {
		for (Key &key : input.getBuffer()) {
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
					text = text.substr(0, cursor_position - 1) + text.substr(cursor_position, text.length() - cursor_position);
					if (cursor_position > 0) {
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

		unsigned long time = timer.getTime();

		display.update();
		Buffer& buffer = display.getBuffer();

		buffer.setCursorPosition(5 + cursor_position, 10);

		for (int lol = 5; lol < 15; lol++) { 
			buffer.line(lol, 5, lol + 10, 50, CellAttributes().bg(Color::rgb(time / 500.0 + lol / 5.0)));
		}

		buffer.circle(100, 10, 20, CellAttributes().bg(Color::rgb(time / 1000.0)));

		CellAttributes textAttrs;
		textAttrs.fg(0xffffff).bg(0x000000);

		buffer.text(5, 10, text, textAttrs);
		buffer.text(5, 11, std::to_string(text.length()) + " ", CellAttributes(textAttrs).fg(Color::rgb(time / 200.0)));
		buffer.text(5, 12, text.substr(0, text.length()), CellAttributes(textAttrs).fg(Color::rgb(time / 400.0)).bg(Color::rgb(time / 800.0)));
		buffer.text(5, 13, std::to_string(time), textAttrs);

		CellAttributes textAttrs2(textAttrs);
		textAttrs2.fg(Color::rgb(time / 1000.0));

		int i = 0;

		for (utfstring ch : text.chars()) {
			int j = 0;

			for (char c : ch.str()) {
				const std::string s = std::to_string((int)c);
				buffer.text(30 + j, i, s, textAttrs2);
				j += s.length() + 1;
			}

			i++;
		}

		buffer.text(30, i, std::string(20, ' '), textAttrs);

		buffer.print();
		timer.update();
	}
}
