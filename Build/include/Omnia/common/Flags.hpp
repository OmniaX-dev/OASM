#ifndef __FLAGS__HPP__
#define __FLAGS__HPP__

#include "Defines.hpp"

namespace Omnia
{
	namespace common
	{
		class Flags
		{
		public:
			static void init(void);
			static void set(int flag);
			static void unset(int flag);
			static void value(int flag, bool value);
			static bool isset(int flag);
			static bool isunset(int flag);
			static void neg(int flag);

		private:
			inline static bool m_flags[AUTO__MAX_FLAGS];
		};
	}
}

#endif
