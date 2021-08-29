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
		class OmniaString
		{
		public: class StringTokens
		{
			public:
				StringTokens(void);
				StringTokens(int length);
				void create(int length);

				OmniaString next(void);
				OmniaString previous(void);
				int count(void);
				bool hasNext(void);
				bool hasPrevious(void);
				void cycle(void);

				std::vector<OmniaString> array(void);

			private:
				void add(OmniaString token);

			private:
				std::vector<OmniaString> tokens;
				int current;

				friend class OmniaString;
		};
		public:
			inline OmniaString(void) : string("") {}
			inline OmniaString(_string str) : string(str) {}
			inline OmniaString(const char str[]) {string.assign(str);}
			inline OmniaString operator=(_string str) {string = str; return *this;}
			inline OmniaString operator=(const char str[]) {string.assign(str); return *this;}
			inline OmniaString operator=(OmniaString str) {string = str.cpp(); return *this;}
			inline OmniaString operator+(OmniaString str2) {return OmniaString(string + str2.cpp());}
			inline OmniaString operator+(const char str2[]) {return OmniaString(string + str2);}
			inline OmniaString operator+(_string str2) {return OmniaString(string + str2);}
			inline OmniaString operator+(char c) {return OmniaString(string + c);}
			inline OmniaString operator+(int n) {return OmniaString(string + intToStr(n).cpp());}
			inline char operator[](int index) {return string[index];}
			inline bool operator==(OmniaString str2) {return equals(str2);}
			inline bool operator==(const char str2[]) {return equals( _string(str2));}
			inline bool operator==(_string str2) {return equals(str2);}
			inline bool operator!=(OmniaString str2) {return string != str2.cpp();}
			inline bool operator!=(const char str2[]) {return string != _string(str2);}
			inline bool operator!=(_string str2) {return string != str2;}
			inline void put(int index, char c) { string[index] = c; }
			static OmniaString intToStr(int n);
			static OmniaString floatToStr(float n, int sigd = 3);
			inline _string cpp(void) const {return string;}
			inline const char* c_str(void) const {return string.c_str();}
			inline OmniaString add(OmniaString str2) {string += str2.cpp(); return *this;}
			inline OmniaString add(const char str2[]) {string += str2; return *this;}
			inline OmniaString add(_string str2) {string += str2; return *this;}
			inline OmniaString add(char c) {string += c; return *this;}
			inline OmniaString addInt(int i) {string += OmniaString::intToStr(i).cpp(); return *this;}
			inline int length(void) {return (signed)string.length();}
			inline char at(int index) {return string[index];}
			int indexOf(char c);
			int indexOf(char c, int start);
			int indexOf(char c, int start, int end);
			int indexOf(OmniaString str);
			int indexOf(OmniaString str, int start);
			int indexOf(OmniaString str, int start, int end);
			int lastIndexOf(char c);
			int lastIndexOf(OmniaString str);
			OmniaString trim(void);
			int toInt(void);
			float toFloat(void);
			bool isNumeric(bool decimal = false);
			OmniaString substr(int start);
			OmniaString substr(int start, int end);
			OmniaString subString(int start, int nchars);
			bool contains(char c);
			bool contains(OmniaString str);
			bool startsWith(OmniaString str);
			bool endsWith(OmniaString str);
			OmniaString toLowerCase(void);
			OmniaString toUpperCase(void);
			OmniaString reverse(void);
			StringTokens tokenize(OmniaString sep = " ", bool trim = true);
			void split(char c, OmniaString* part1, OmniaString* part2, bool trim = true);
			StringTokens splitExc(char exc = '\"', bool trim = true);
			bool equals(OmniaString str2, bool ignoreCase = false);
			OmniaString remove(char c);
			OmniaString replaceAll(OmniaString search, OmniaString replace);
			inline void set(OmniaString str) { string = str.cpp(); }

		protected:
			_string string;
		};
	}
}
#endif
