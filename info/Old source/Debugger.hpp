#ifndef __DEBUGGER_HPP__
#define __DEBUGGER_HPP__

#include "ProcessManager.hpp"

namespace Omnia
{
	namespace oasm
	{
		class Debugger : public IOReciever
		{
			public:
				static inline Debugger& instance(void) { return s_instance; }
				bool run(String oex, DebugInfo dbg, std::vector<_int32> args = std::vector<_int32>());
				inline void addBreakPoint(MemAddress addr) { m_breaks.push_back(addr); }

			private:
				inline Debugger(void) : m_vmh(ProcessManager::instance()) {  }
				bool isBreakPoint(Process& proc, MemAddress inst);
				void printLastInst(void);

			private:
				ProcessManager& m_vmh;
				std::vector<MemAddress> m_breaks;

				static Debugger s_instance;
		};

		inline Debugger Debugger::s_instance;
	}
}


#endif