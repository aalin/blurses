#include <iostream>
#include <sys/ioctl.h>
#include "utfstring.hpp"
#include "cell_attributes.hpp"
#include "blurses.hpp"
#include <stack>
#include <memory>

class Widget {
	public:
		virtual void draw(Display& display, uint16_t x, uint16_t y, bool active) = 0;
		virtual void handleKey(const Key&) {}
		virtual void reset() {}
		virtual utfstring getValue() const = 0;
		virtual int getCursorPosition() const { return 0; }
};

typedef std::shared_ptr<Widget> WidgetPtr;

class CheckboxField : public Widget {
	public:
		CheckboxField(utfstring label) : _checked(false), _label(label) {}

		utfstring getValue() const {
			return _checked ? "1" : "0";
		}

		void reset() {
			_checked = 0;
		}

		void handleKey(const Key& key) {
			if (key.type == Key::DATA && key.data == " ") {
				_checked = !_checked;
			}
		}

		void draw(Display& display, uint16_t x, uint16_t y, bool active) {
			CellAttributes attrs = display.attr().fg(0xffffff);
			
			if (active) {
				display.setCursorPosition(0, 0);
				attrs.bg(0xffffff).fg(0x000000);
			}

			if (_checked) {
				display.primitives().text(x, y, "☑ ", attrs);
			} else {
				display.primitives().text(x, y, "☐ ", attrs);
			}

			display.primitives().text(x + 2, y, _label, attrs);
		}

	private:
		bool _checked;
		utfstring _label;
};

class InputField : public Widget {
	public:
		InputField() : _cursor_position(0) { }

		utfstring getValue() const {
			return _text;
		}

		int getCursorPosition() const {
			return _cursor_position;
		}

		void reset() {
			_text = "";
			_cursor_position = 0;
		}

		void draw(Display& display, uint16_t x, uint16_t y, bool active) {
			if (active) {
				display.setCursorPosition(x + _cursor_position, y);
			}

			display.primitives().text(x, y, _text + " ", display.attr().fg(active ? 0xffffff : 0xcccccc));
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
				case Key::KEY_HOME:
					_cursor_position = 0;
					break;
				case Key::KEY_END:
					_cursor_position = _text.length();
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
				case Key::KEY_ESCAPE:
					_text = "";
					_cursor_position = 0;
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
			_widgets.push_back(std::make_shared<InputField>());
			_widgets.push_back(std::make_shared<InputField>());
			_widgets.push_back(std::make_shared<InputField>());
			_widgets.push_back(std::make_shared<CheckboxField>("foo"));
			_widgets.push_back(std::make_shared<CheckboxField>("bar"));
			_index = 0;
		}

		void handleKey(Display &display, const Key& key, unsigned long ticks) {
			switch (key.type) {
				case Key::KEY_RETURN:
					handleReturn();
					break;
				case Key::KEY_REDRAW:
					display.redraw();
					break;
				case Key::KEY_TAB:
					_index = (_index + 1) % _widgets.size();
					break;
				case Key::KEY_TAB_BACK:
					_index = (_index + _widgets.size() - 1) % _widgets.size();
					break;
				default:
					_widgets[_index]->handleKey(key);
			}
		}

		void update(unsigned long ticks) {
			_t = ticks;
		}

		void draw(Display& display) {
			auto attrs = display.attr().fg(0xcccccc);
			auto attrs2 = display.attr().fg(0xffffff);

			display.primitives().circle(100, 15, 15, display.attr().bg(Color::hsv(_t / 5.0, 1.0, 0.8)));
			display.primitives().filledRect(95, 5, 105, 15, display.attr().bg(Color::hsv(_t / 20.0, 1.0, 0.8)));
			display.primitives().rect(95, 5, 105, 15, display.attr().bg(Color::hsv(_t / 10.0, 1.0, 0.8)));

			for (size_t i = 0; i < _widgets.size(); i++) {
				display.primitives().text(0, 10 + i, "input: ", i == _index ? attrs : attrs2);
				_widgets[i]->draw(display, 7, 10 + i, i == _index);
			}

			auto textAttrs = display.attr()
				.fg(Color::hsv((_t + _index * 1000) / 10.0, 1.0, 1.0))
				.bg(Color::hsv((_t + _index * 1000) / 10.0, 1.0, 0.5));

			int j = 0;

			for (utfstring text : _texts) {
				display.primitives().text(20, j++, text, textAttrs);
			}

			auto textAttrs2 = display.attr().fg(0x0099cc);
			auto textAttrs3 = display.attr().fg(0xffcc00);

			utfstring text = _widgets[_index]->getValue();
			int pos = _widgets[_index]->getCursorPosition();
			int i = 0;
			display.primitives().text(50, 0, text.substr(0, pos), textAttrs2);
			display.primitives().text(50 + pos, 0, text.substr(pos, text.length() - pos), textAttrs3);

			display.primitives().text(50, 1, std::to_string(text.find_offset2(0)), textAttrs);
			display.primitives().text(50, 2, std::to_string(text.find_offset2(1)), textAttrs);
			display.primitives().text(50, 3, std::to_string(text.find_offset2(2)), textAttrs);

			for (utfstring ch : text.chars()) {
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
		std::vector<WidgetPtr> _widgets;
		unsigned long _t;
		unsigned int _index;
		std::list<utfstring> _texts;

		void handleReturn() {
			utfstring text = _widgets[_index]->getValue();

			if (text.length() > 0) {
				_texts.push_back(text);
			}

			_widgets[_index]->reset();
		}
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
