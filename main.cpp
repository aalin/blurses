#include <iostream>
#include <sys/ioctl.h>
#include "utfstring.hpp"
#include "cell_attributes.hpp"
#include "blurses.hpp"
#include <stack>

class InputField {
	public:
		InputField() : _cursor_position(0) { }

		utfstring getText() {
			return _text;
		}

		void reset() {
			_text = "";
			_cursor_position = 0;
		}

		void draw(Display& display, uint16_t x, uint16_t y, unsigned long t, bool active) {
			if (active) {
				display.setCursorPosition(x + _cursor_position, y);
			}
			display.primitives().text(x, y, _text + " ", display.attr().fg(Color::rgb(t / 1000.0)));
		}

		void handleKey(const Key& key) {
			switch (key.type) {
				case Key::DATA:
					_text = _text.substr(0, _cursor_position) +
						key.data +
						_text.substr(_cursor_position, _text.length() - _cursor_position);
					_cursor_position++;
					if (_cursor_position > _text.length()) {
						_cursor_position = _text.length();
					}
					break;
				case Key::KEY_LEFT:
					if (_cursor_position > 0) { _cursor_position--; }
					break;
				case Key::KEY_RIGHT:
					if (_cursor_position < _text.length()) { _cursor_position++; }
					break;
				case Key::KEY_BACKSPACE:
					if (_cursor_position > 0) {
						_text = _text.substr(0, _cursor_position - 1) +
							_text.substr(_cursor_position, _text.length() - _cursor_position);
						_cursor_position--;
					}
					break;
				case Key::KEY_DELETE:
					_text = _text.substr(0, _cursor_position) + _text.substr(_cursor_position + 1, _text.length() - _cursor_position);
					break;
			}
		}

	private:
		int _cursor_position;
		utfstring _text;
};

class State {
	public:
		State() {
			_inputs.assign(3, InputField());
			_index = 0;
		}

		void handleKey(Display &display, const Key& key, unsigned long ticks) {
			switch (key.type) {
				case Key::KEY_RETURN:
					_texts.push_back(_inputs[_index].getText());
					_inputs[_index].reset();
					break;
				case Key::KEY_REDRAW:
					display.redraw();
					break;
				case Key::KEY_TAB:
					_index = (_index + 1) % _inputs.size();
					break;
				case Key::KEY_TAB_BACK:
					if (_index > 0) {
						_index--;
					} else {
						_index = _inputs.size() - 1;
					}
					break;
				default:
					_inputs[_index].handleKey(key);
			}
		}

		void update(unsigned long ticks) {
			_t = ticks;
		}

		void draw(Display& display) {
			CellAttributes attrs;
			attrs.fg(0xcccccc);
			CellAttributes attrs2(attrs);
			attrs.fg(0xffffff).bg(0x666666);

			for (size_t i = 0; i < _inputs.size(); i++) {
				display.primitives().text(0, 10 + i, "input: ", i == _index ? attrs : attrs2);
				_inputs[i].draw(display, 7, 10 + i, _t + i * 1000, i == _index);
			}

			CellAttributes textAttrs;
			textAttrs.fg(Color::rgb((_t + _index * 1000) / 1000.0));

			int j = 0;

			for (utfstring text : _texts) {
				display.primitives().text(20, j++, text, textAttrs);
			}

			int i = 0;

			for (utfstring ch : _inputs[_index].getText().chars()) {
				int j = 0;

				for (char c : ch.str()) {
					const std::string s = std::to_string((int)c);
					display.primitives().text(30 + j, i, s, textAttrs);
					j += s.length() + 1;
				}

				i++;
			}

			display.primitives().text(30, i, std::string(20, ' '), textAttrs);
		}

	private:
		std::vector<InputField> _inputs;
		unsigned long _t;
		unsigned int _index;
		std::list<utfstring> _texts;
};

class Application {
	public:
		Application() {
			pushState(new State());
		}

		void run() {
			Blurses::start([&](Display &display, std::list<Key> keys, unsigned long ticks) -> bool {
				for (const Key &key : keys) {
					currentState().handleKey(display, key, ticks);
				}

				currentState().update(ticks);
				currentState().draw(display);

				return true;
			});
		}

		void pushState(State* state) {
			_states.push(state);
		}

		void popState() {
			_states.pop();
		}

	private:
		std::stack<State*> _states;

		State& currentState() {
			return *_states.top();
		}
};

int main() {
	Application app;
	app.run();
}

int main2() {
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
