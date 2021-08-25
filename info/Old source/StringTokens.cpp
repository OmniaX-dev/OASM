#include "StringTokens.hpp"

namespace Omnia
{
	StringTokens::StringTokens(void)
	{
		create(10);
	}

	StringTokens::StringTokens(int length)
	{
		create(length);
	}

	void StringTokens::create(int length)
	{
		tokens.reserve(length);
		current = 0;
	}

	String StringTokens::next(void)
	{
		if (hasNext())
			return tokens[current++];
		return "";
	}

	String StringTokens::previous(void)
	{
		if (hasPrevious())
			return tokens[--current];
		return "";
	}

	int StringTokens::count(void)
	{
		return tokens.size();
	}

	void StringTokens::add(String token)
	{
		tokens.push_back(token);
	}

	bool StringTokens::hasNext(void)
	{
		return current < tokens.size();
	}

	bool StringTokens::hasPrevious(void)
	{
		return current > 0;
	}

	void StringTokens::cycle(void)
	{
		current = 0;
	}

	std::vector<String> StringTokens::array(void)
	{
		return tokens;
	}
}
