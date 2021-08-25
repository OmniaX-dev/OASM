#ifndef __STRING_TOKENS__H__
#define __STRING_TOKENS__H__

#include "String.hpp"

namespace Omnia
{
	class StringTokens
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
}
#endif
