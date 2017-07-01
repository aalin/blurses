#include <string>
#include <vector>
#include <iostream>
#include <locale>
#include <codecvt>
#include <sys/ioctl.h>
#include <chrono>
#include <thread>
#include <csignal>
#include <functional>
#include "input.hpp"
#include "utfstring.hpp"
#include "display.hpp"
#include <cmath>

bool running = true;

void handleSigint(int signum __attribute__((unused))) {
	running = false;
}

const char* BACKSPACE = "\x7f";
const char* REDRAW = "\x0c";

Color rgb(float i) {
	return Color(
		std::pow(std::sin(i + (0 / 3.0) * M_PI), 2) * 255,
		std::pow(std::sin(i + (1 / 3.0) * M_PI), 2) * 255,
		std::pow(std::sin(i + (2 / 3.0) * M_PI), 2) * 255
	);
}

int main() {
	Display display;
	Input input;
	input.run();

	std::signal(SIGINT, &handleSigint);
	utfstring text("");
	unsigned int count = 0;
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

		display.update();
		Buffer& buffer = display.getBuffer();

		buffer.setCursorPosition(5 + cursor_position, 10);

		buffer.text(5, 10, std::string(text.length() + 1, ' '), 0x000000);
		buffer.text(5, 10, text, 0x555555);
		buffer.text(5, 11, std::string(text.length() + 1, ' '), 0x000000);
		buffer.text(5, 11, std::to_string(text.length()) + " ", rgb(count / 20.0));
		buffer.text(5, 12, std::string(text.length() + 1, ' '), 0x000000);
		buffer.text(5, 12, text.substr(0, text.length()), rgb(count / 50.0), rgb(count / 50.0 + 1.0));
		count++;

		int i = 0;

		for (int x = 0; x < display.height(); x++) {
			buffer.text(30, x, std::string(20, ' '), 0x000000);
		}

		for (utfstring ch : text.chars()) {
			int j = 0;

			for (char c : ch.str()) {
				const std::string s = std::to_string((int)c);
				buffer.text(30 + j, i, s, rgb(count / 100.0));
				j += s.length() + 1;
			}

			i++;
		}

		buffer.print();

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}
