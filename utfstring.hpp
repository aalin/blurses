#ifndef UTFSTRING_HPP
#define UTFSTRING_HPP

#include <string>
#include <list>
#include <iterator>
#include "vendor/utfcpp/source/utf8.h"

class utfstring {
	public:
		utfstring() : _str("") { }
		utfstring(const char* str) : _str(str) { }
		utfstring(std::string str) : _str(str) { }

		static utfstring decode(uint32_t codepoint) {
			std::string str;
			utf8::append(codepoint, std::back_inserter(str));
			return str;
		}

		int length() const {
			return chars().size();
		}

		size_t find_offset2(size_t index) const {
			return find_offset(index);
		}

		std::list<utfstring> chars() const {
			std::list<utfstring> list;

			char* start = strptr();
			char* end = start + _str.length();

			if (start == end) {
				return list;
			}

			char* prev = start;
			char* curr = start;

			while (utf8::next(curr, end)) {
				while (curr != end && is_combining(utf8::peek_next(curr, end))) {
					utf8::next(curr, end);
				}

				list.push_back(_str.substr(prev - start, curr - prev));

				prev = curr;

				if (curr == end) {
					break;
				}
			}

			return list;
		}

		utfstring at(int pos) const {
			return substr(pos, 1);
		}

		utfstring substr(size_t pos, size_t len) const {
			if (_str.length() == 0) {
				return utfstring("");
			}

			const size_t u8_pos = find_offset(pos);

			if (u8_pos > _str.length()) {
				return utfstring("");
			}

			const size_t u8_len = find_offset(pos + len) - u8_pos;

			return utfstring(_str.substr(u8_pos, u8_len));
		}

		utfstring& operator=(const utfstring &other) {
			this->_str = other.str();
			return *this;
		}

		utfstring& operator+=(const utfstring &other) {
			this->_str += other.str();
			return *this;
		}

		utfstring operator+(const utfstring &other) const {
			return utfstring(this->str() + other.str());
		}

		utfstring operator+(const std::string &other) const {
			return utfstring(this->str() + other);
		}

		utfstring operator+(const char* other) const {
			return utfstring(this->str() + other);
		}

		bool operator==(const utfstring& other) const {
			return other.str() == str();
		}

		bool operator==(const std::string& other) const {
			return other == str();
		}

		bool operator!=(const utfstring& other) const {
			return !(*this == other);
		}

		bool operator!=(const std::string& other) const {
			return !(*this == other);
		}

		std::string str() const {
			return _str;
		}

		size_t find_offset(size_t index) const {
			char* start = strptr();
			char* end = start + _str.length();
			char* prev = start;
			char* curr = start;

			if (start == end) {
				return 0;
			}

			size_t i = 0;

			while (utf8::next(curr, end)) {
				if (i++ >= index) {
					break;
				}

				while (curr != end && is_combining(utf8::peek_next(curr, end))) {
					utf8::next(curr, end);
				}

				prev = curr;

				if (curr == end) {
					break;
				}
			}

			return prev - start;
		}

		static bool is_valid(const std::string& str) {
			const char *start = const_cast<char*>(str.c_str());
			const char *end = start + str.length();

			return utf8::is_valid(start, end);
		}

		static bool is_combining(int cp) {
			return (
				(cp >= 0x0300 && cp < 0x036f) || // Combining Diacritical Marks
				(cp >= 0x1AB0 && cp < 0x1AFF) || // Combining Diacritical Marks Extended
				(cp >= 0x1DC0 && cp < 0x1DFF) || // Combining Diacritical Marks Supplement
				(cp >= 0x20D0 && cp < 0x20FF) || // Combining Diacritical Marks for Symbols
				(cp >= 0xFE20 && cp < 0xFE2F)    // Combining Half Marks
			);
		}

	private:
		std::string _str;

		char* strptr() const {
			return const_cast<char*>(_str.c_str());
		}


		friend std::ostream& operator<<(std::ostream& stream, const utfstring& str) {
			return stream << str.str();
        }
};

#endif
