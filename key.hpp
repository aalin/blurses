#ifndef KEY_HPP
#define KEY_HPP

#include <string>

namespace Blurses {
struct Key {
	enum TYPE {
		DATA,
		KEY_UP,
		KEY_DOWN,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_BACKSPACE,
		KEY_DELETE,
		KEY_REDRAW,
		KEY_CANCEL,
		KEY_ESCAPE,
		KEY_RETURN,
		KEY_TAB,
		KEY_TAB_BACK,
		KEY_END,
		KEY_HOME,
	};

	Key(Key::TYPE type) : type(type) {}
	Key(std::string str) : type(Key::DATA), data(str) {}

	Key::TYPE type;
	std::string data;
};
};

#endif
