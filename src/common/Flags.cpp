#include "Flags.hpp"

namespace Omnia
{
	namespace common
	{
		void Flags::init(void)
		{
			for (int i = 0; i < AUTO__MAX_FLAGS; i++)
				m_flags[i] = false;
		}

		void Flags::set(int flag)
		{
			if (flag >= 0 && flag < AUTO__MAX_FLAGS)
				m_flags[flag] = true;
		}

		void Flags::unset(int flag)
		{
			if (flag >= 0 && flag < AUTO__MAX_FLAGS)
				m_flags[flag] = false;
		}

		void Flags::value(int flag, bool value)
		{
			if (flag >= 0 && flag < AUTO__MAX_FLAGS)
				m_flags[flag] = value;
		}

		bool Flags::isset(int flag)
		{
			if (flag >= 0 && flag < AUTO__MAX_FLAGS)
				return m_flags[flag];
			//TODO: port: GameData::instance().errorQueue.push(Error::FLAG_INDEX_OUT_OF_BOUNDS, ERROR_DATA());
			return false;
		}

		bool Flags::isunset(int flag)
		{
			return !Flags::isset(flag);
		}

		void Flags::neg(int flag)
		{
			if (flag >= 0 && flag < AUTO__MAX_FLAGS)
				m_flags[flag] = !m_flags[flag];
		}
	}
}
