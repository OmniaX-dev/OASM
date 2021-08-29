#ifndef __STRING__HPP__
#define __STRING__HPP__

#include <iostream>
#include <vector>

namespace Omnia
{
	namespace common
	{
		typedef std::string _string;

		class StringTokens;
		class String
		{
		public: class StringTokens
		{
			public:
				StringTokens(void);
				StringTokens(int length);
				void create(int length);

				String next(void);
				String previous(void);
				int count(void);
				bool hasNext(void);
				bool hasPrevious(void);
				void cycle(void);

				std::vector<String> array(void);

			private:
				void add(String token);

			private:
				std::vector<String> tokens;
				int current;

				friend class String;
		};
		public:
			inline String(void) : string("") {}
			inline String(_string str) : string(str) {}
			inline String(const char str[]) {string.assign(str);}
			inline String operator=(_string str) {string = str; return *this;}
			inline String operator=(const char str[]) {string.assign(str); return *this;}
			inline String operator=(String str) {string = str.cpp(); return *this;}
			inline String operator+(String str2) {return String(string + str2.cpp());}
			inline String operator+(const char str2[]) {return String(string + str2);}
			inline String operator+(_string str2) {return String(string + str2);}
			inline String operator+(char c) {return String(string + c);}
			inline String operator+(int n) {return String(string + intToStr(n).cpp());}
			inline char operator[](int index) {return string[index];}
			inline bool operator==(String str2) {return equals(str2);}
			inline bool operator==(const char str2[]) {return equals( _string(str2));}
			inline bool operator==(_string str2) {return equals(str2);}
			inline bool operator!=(String str2) {return string != str2.cpp();}
			inline bool operator!=(const char str2[]) {return string != _string(str2);}
			inline bool operator!=(_string str2) {return string != str2;}
			inline void put(int index, char c) { string[index] = c; }
			static String intToStr(int n);
			static String floatToStr(float n, int sigd = 3);
			inline _string cpp(void) const {return string;}
			inline const char* c_str(void) const {return string.c_str();}
			inline String add(String str2) {string += str2.cpp(); return *this;}
			inline String add(const char str2[]) {string += str2; return *this;}
			inline String add(_string str2) {string += str2; return *this;}
			inline String add(char c) {string += c; return *this;}
			inline String addInt(int i) {string += String::intToStr(i).cpp(); return *this;}
			inline int length(void) {return (signed)string.length();}
			inline char at(int index) {return string[index];}
			int indexOf(char c);
			int indexOf(char c, int start);
			int indexOf(char c, int start, int end);
			int indexOf(String str);
			int indexOf(String str, int start);
			int indexOf(String str, int start, int end);
			int lastIndexOf(char c);
			int lastIndexOf(String str);
			String trim(void);
			int toInt(void);
			float toFloat(void);
			bool isNumeric(bool decimal = false);
			String substr(int start);
			String substr(int start, int end);
			String subString(int start, int nchars);
			bool contains(char c);
			bool contains(String str);
			bool startsWith(String str);
			bool endsWith(String str);
			String toLowerCase(void);
			String toUpperCase(void);
			String reverse(void);
			StringTokens tokenize(String sep = " ", bool trim = true);
			void split(char c, String* part1, String* part2, bool trim = true);
			StringTokens splitExc(char exc = '\"', bool trim = true);
			bool equals(String str2, bool ignoreCase = false);
			String remove(char c);
			String replaceAll(String search, String replace);
			inline void set(String str) { string = str.cpp(); }

		protected:
			_string string;
		};
	}
}
#endif
