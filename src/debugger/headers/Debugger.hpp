#ifndef __DEBUGGER_HPP__
#define __DEBUGGER_HPP__

#include "Common.hpp"
#include "IOManager.hpp"

namespace Omnia
{
	namespace oasm
	{
		class Debugger : public IOReciever, public ErrorReciever
		{
			public:
				inline static Debugger& instance(void) { return *Debugger::s_instance; }
				int64 run(int argc, char** argv);

			private:
				std::map<_string, MemAddress> m_labels;
				
				static Debugger* s_instance;
		};

		inline Debugger* Debugger::s_instance = new Debugger();
	}
}

#endif