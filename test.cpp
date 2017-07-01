#include "input.hpp"
#include "utfstring.hpp"

bool isCombining(int cp) {
	return (
		(cp >= 0x0300 && cp < 0x036f) || // Combining Diacritical Marks
		(cp >= 0x1AB0 && cp < 0x1AFF) || // Combining Diacritical Marks Extended
		(cp >= 0x1DC0 && cp < 0x1DFF) || // Combining Diacritical Marks Supplement
		(cp >= 0x20D0 && cp < 0x20FF) || // Combining Diacritical Marks for Symbols
		(cp >= 0xFE20 && cp < 0xFE2F)    // Combining Half Marks
   );
}

int test() {
	/*
	Input input;
	input.run();
	*/

	/*
	const utfstring str("fåäbaröfåäbaröfåäbaröfåäbarö");
	std::cout << "str: " << str << std::endl;
	std::cout << "length(): " << str.length() << std::endl;
	std::cout << "substr(2, len - 3): " << str.substr(2, str.length() - 3) << std::endl;
	std::cout << "at(2): " << str.at(2) << std::endl;

	for (utfstring ch : str.chars()) {
		std::cout << "char: " << ch << std::endl;
	}
	*/

	std::cout << utfstring("fååbar").substr(2, 2) << std::endl;
	std::cout << utfstring("").substr(0, 0) << std::endl;

	std::string str = "Heåäöh̀̍͐̏e͂̐̔̍l̈́̉̌̈l̈́͌̏̿ō̐̈͠j";
	char* start = const_cast<char*>(str.c_str());
	char* end = start + str.length();
	char* prev = start;
	char* curr = start;

	while (int cp = utf8::next(curr, end)) {
		while (curr != end && isCombining(utf8::peek_next(curr, end))) {
			utf8::next(curr, end);
		}

		const std::string ch = str.substr(prev - start, curr - prev);
		std::cout << cp << " " << ch << std::endl;
		prev = curr;

		if (curr == end) {
			break;
		}
	}

	return 0;
}
