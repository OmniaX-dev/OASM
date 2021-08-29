#include "OmniaString.hpp"
#include <math.h>

#include <algorithm>
#include <cctype>
#include <locale>
#include <string>
#include <sstream>
//#include <regex>
#include <iomanip>

namespace Omnia
{
	namespace common
	{
		OmniaString OmniaString::floatToStr(float n, int sigd)
		{
			int i = (int)(n);
			float f = n - i;
			int _f = (int)(f * pow(10, sigd));
			(_f < 0 ? _f *= -1 : _f = _f);
			OmniaString str = OmniaString("") + (i != 0 ? "" : (n < 0 ? "-" : "")) + OmniaString::intToStr(i) + "." + OmniaString::intToStr(_f);
			return str;
		}

		OmniaString OmniaString::intToStr(int n)
		{
			int nn = n;
			bool neg = false;
			if (nn < 0)
			{
				nn *= -1;
				neg = true;
			}
			int r = 0;
			OmniaString str = "";
			while (nn >= 10)
			{
				r = nn % 10;
				nn /= 10;
				str.add((char)(r + '0'));
			}
			str.add(nn + '0');
			return str.add((neg ? "-" : "")).reverse();
		}

		int OmniaString::indexOf(char c)
		{
			return indexOf(c, 0, length());
		}

		int OmniaString::indexOf(char c, int start)
		{
			return indexOf(c, start, length());
		}

		int OmniaString::indexOf(char c, int start, int end)
		{
			if (start < 0)
				start = 0;
			if (start >= length())
				start = length() - 1;
			if (end <= start)
				return -1;
			if (end > length())
				end = length();
			for (int i = start; i < end; i++)
			{
				if (at(i) == c)
					return i;
			}
			return -1;
		}

		int OmniaString::indexOf(OmniaString str)
		{
			return indexOf(str, 0, length());
		}

		int OmniaString::indexOf(OmniaString str, int start)
		{
			return indexOf(str, start, length());
		}

		int OmniaString::indexOf(OmniaString str, int start, int end)
		{
			if (start < 0)
				start = 0;
			if (start >= length())
				start = length() - 1;
			if (end <= start)
				return -1;
			if (end > length())
				end = length();
			if (end - start < str.length())
				return -1;
			bool done = false;
			for (int i = start; i < end; i++)
			{
				if (end - i < str.length())
					return -1;
				for (int j = i; j < i + str.length(); j++)
				{
					if (at(j) == str.at(j - i))
					{
						done = true;
						continue;
					}
					done = false;
					break;
				}
				if (done)
					return i;
			}
			return -1;
		}

		int OmniaString::lastIndexOf(char c)
		{
			_string tmp = string;
			string = reverse().cpp();
			int index = indexOf(c);
			string = tmp;
			return length() - index - 1;
		}

		int OmniaString::lastIndexOf(OmniaString str)
		{
			_string tmp = string;
			string = reverse().cpp();
			int index = indexOf(str.reverse());
			string = tmp;
			return length() - index - str.length();
		}

		OmniaString OmniaString::trim(void)
		{
			_string s = cpp();
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			}));
			s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(), s.end());
			return OmniaString(s).remove('\n');
		}

		int OmniaString::toInt(void)
		{
			if (!isNumeric())
				return -1;
			return std::atoi(trim().c_str());
		}

		float OmniaString::toFloat(void)
		{
			std::istringstream iss(cpp());
			float f = 0;
			iss >> std::noskipws >> f;
			if (iss.eof() && !iss.fail())
				return std::stof(cpp());
			return 0;
		}

		bool OmniaString::isNumeric(bool decimal)
		{
			int len = trim().length();
			if (len == 0) return false;
			bool dec = false;
			for (int i = 0; i < len; i++)
			{
				char c = at(i);
				if ((c >= '0' && c <= '9') || (i == 0 && c == '-') || (decimal && i < len - 1 && c == '.' && !dec))
				{
					if (c == '.')
						dec = true;
					continue;
				}
				return false;
			}
			return true;
		}

		OmniaString OmniaString::substr(int start)
		{
			return subString(start, length() - start);
		}

		OmniaString OmniaString::substr(int start, int end)
		{
			return subString(start, end - start);
		}

		OmniaString OmniaString::subString(int start, int nchars)
		{
			OmniaString res = "";
			if (start < 0 || nchars < 0 || start + nchars > length())
				return OmniaString();
			for (int i = start; i < start + nchars; i++)
				res.add(at(i));
			return res;
		}

		bool OmniaString::contains(char c)
		{
			return indexOf(c) >= 0;
		}

		bool OmniaString::contains(OmniaString str)
		{
			return indexOf(str) >= 0;
		}

		bool OmniaString::startsWith(OmniaString str)
		{
			return indexOf(str) == 0;
		}

		bool OmniaString::endsWith(OmniaString str)
		{
			int index = lastIndexOf(str);
			return index >= 0 && index == length() - str.length();
		}

		OmniaString OmniaString::reverse(void)
		{
			OmniaString str = "";
			for (int j = length() - 1; j >= 0; j--)
				str = str + at(j);
			return str;
		}

		OmniaString::StringTokens OmniaString::tokenize(OmniaString sep, bool trim)
		{
			std::vector<OmniaString> tokens;
			bool done = false;
			OmniaString token = "";
			for (int i = 0; i < length(); i++)
			{
				for (int j = i; j < i + sep.length(); j++)
				{
					if (at(j) == sep.at(j - i))
					{
						done = true;
						continue;
					}
					done = false;
					break;
				}
				if (done)
				{
					i += sep.length() - 1;
					tokens.push_back(token);
					token = "";
				}
				else
					token.add(at(i));
			}
			tokens.push_back(token);
			StringTokens t(tokens.size());
			for (unsigned int i = 0; i < tokens.size(); i++)
				if (trim && tokens[i].trim() != "")
					t.add(trim ? tokens[i].trim() : tokens[i]);
				else if (!trim)
					t.add(tokens[i]);
			return t;
		}

		void OmniaString::split(char c, OmniaString* part1, OmniaString* part2, bool trim)
		{
			if (indexOf(c) == -1)
				return;
			*part1 = substr(0, indexOf(c));
			*part2 = substr(indexOf(c) + 1);
			if (trim)
			{
				*part1 = part1->trim();
				*part2 = part2->trim();
			}
		}

		OmniaString OmniaString::toLowerCase(void)
		{
			OmniaString res = "";
			for (int i = 0; i < length(); i++)
			{
				if (at(i) >= 'A' && at(i) <= 'Z')
					res.add(at(i) + ('a' - 'A'));
				else
					res.add(at(i));
			}
			return res;
		}

		OmniaString OmniaString::toUpperCase(void)
		{
			OmniaString res = "";
			for (int i = 0; i < length(); i++)
			{
				if (at(i) >= 'a' && at(i) <= 'z')
					res.add(at(i) - ('a' - 'A'));
				else
					res.add(at(i));
			}
			return res;
		}

		OmniaString::StringTokens OmniaString::splitExc(char exc, bool trim)
		{
			std::istringstream iss(cpp());
			std::vector<OmniaString> v;
			std::string s;
			while (iss >> std::quoted(s, exc))
				v.push_back(OmniaString(s));
			StringTokens st(v.size());
			for (auto& ss : v)
			{
				if (trim)
					ss = ss.trim();
				st.add(ss);
			}
			return st;
		}

		bool OmniaString::equals(OmniaString str2, bool ignoreCase)
		{
			if (str2.length() != length())
				return false;
			OmniaString str1 = (ignoreCase ? toLowerCase() : cpp());
			if (ignoreCase)
				str2 = str2.toLowerCase();
			for (int i = 0; i < length(); i++)
			{
				if (str1.at(i) != str2.at(i))
					return false;
			}
			return true;
		}

		OmniaString OmniaString::remove(char c)
		{
			OmniaString res = "";
			for (int i = 0; i < length(); i++)
				if (at(i) != c)
					res.add(at(i));
			return res;
		}

		OmniaString OmniaString::replaceAll(OmniaString search, OmniaString replace)
		{
			_string s = string;
			for (size_t pos = 0; ; pos += replace.length())
			{
				pos = s.find(search.cpp(), pos);
				if (pos == std::string::npos) break;
				s.erase(pos, search.length());
				s.insert(pos, replace.cpp());
			}
			return OmniaString(s);
		}



		OmniaString::StringTokens::StringTokens(void)
		{
			create(10);
		}

		OmniaString::StringTokens::StringTokens(int length)
		{
			create(length);
		}

		void OmniaString::StringTokens::create(int length)
		{
			tokens.reserve(length);
			current = 0;
		}

		OmniaString OmniaString::StringTokens::next(void)
		{
			if (hasNext())
				return tokens[current++];
			return "";
		}

		OmniaString OmniaString::StringTokens::previous(void)
		{
			if (hasPrevious())
				return tokens[--current];
			return "";
		}

		int OmniaString::OmniaString::StringTokens::count(void)
		{
			return tokens.size();
		}

		void OmniaString::StringTokens::add(OmniaString token)
		{
			tokens.push_back(token);
		}

		bool OmniaString::StringTokens::hasNext(void)
		{
			return (unsigned)current < tokens.size();
		}

		bool OmniaString::StringTokens::hasPrevious(void)
		{
			return current > 0;
		}

		void OmniaString::StringTokens::cycle(void)
		{
			current = 0;
		}

		std::vector<OmniaString> OmniaString::StringTokens::array(void)
		{
			return tokens;
		}
	}
}
