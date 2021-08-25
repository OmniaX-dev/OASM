#include "Debugger.hpp"

namespace Omnia
{
	namespace oasm
	{
		bool Debugger::isBreakPoint(Process& proc, MemAddress inst)
		{
			if (m_breaks.size() == 0) return false;
			for (auto& br : m_breaks)
			{
				if (m_vmh.offsetCode(proc, br) == inst) return true;
			}
			return false;
		}

		void Debugger::printLastInst(void)
		{
			std::vector<_int32> li = m_vmh.m_lastInst;
			if (li.size() == 0) return;
			String inst = Utils::instFromCode(li[0]);
			m_out->print("Last Instrution: ").newLine().tab().print(inst);
			for (_uint16 i = 1; i < li.size(); i++)
				m_out->print(", ").print(Utils::intToHexStr(li[i], true));
			m_out->newLine();
		}

		bool Debugger::run(String oex, DebugInfo dbg, std::vector<_int32> args)
		{
			BufferedOutput bout;
			m_vmh.setOutputHandler(&bout);
			m_out->newLine().print("Debug output");
			m_out->newLine().print("------------------------------------------------------------------").newLine();
			_uint16 ipu = m_vmh.m_ipu;
			m_vmh.m_ipu = 1;
			if (!m_vmh.runFromFile(oex, args))
			{
				m_out->print("Error running process.").newLine();
				return 2;
			}
			if (m_vmh.getRunningProcess() == nullptr)
			{
				m_out->print("Null running proc.").newLine();
				return 1;
			}
			Process& proc = *m_vmh.getRunningProcess();
			proc.m_debug = dbg;
			String input = "";
			bool running = true;
			while (true)
			{
				m_out->print(" > ");
				m_in->read(input);
				input = input.trim();
				if (input == "") continue;
				else if (input == "run")
				{
					input = "";
					break;
				}
				else if (input.startsWith("break "))
				{
					input = input.substr(6).trim();
					if (!Utils::isInt(input))
					{
						m_out->print("Error: code address expected").newLine();
						continue;
					}
					addBreakPoint((MemAddress)Utils::strToInt(input));
					continue;
				}
			}
			
			while (running)
			{
				for (_uint16 i = 0; i < ipu; i++)
				{
					MemAddress inst = m_vmh.mem((MemAddress)eRegisters::IP, false);
					if (isBreakPoint(proc, inst) || proc.m_debug.stepExecution)
					{
						m_out->print(" #> ");
						m_in->read(input);
					}
					std::vector<String> pout;
					if (input.trim() == "")
					{
						running = m_vmh.tick();
						pout = bout.flush();
						if (pout.size() == 0)
						{
							input = "";
							continue;
						}
						m_out->newLine().print("[script]#> ");
						for (auto& o : pout)
							m_out->print(o);
						input = "";
						continue;
					}
					if (input.trim() == "next")
					{
						running = m_vmh.tick();
						printLastInst();
						pout = bout.flush();
						if (pout.size() == 0)
						{
							input = "";
							continue;
						}
						m_out->print("[script]#> ");
						for (auto& o : pout)
							m_out->print(o);
					}
					else if (input.trim() == "resume")
					{
						proc.m_debug.setAll(false);
						m_breaks.clear();
					}
					else if (input.trim().startsWith("print "))
					{
						input = input.substr(6).trim();
						if (!Utils::isInt(input))
						{
							m_out->print("Integer expected.").newLine();
						}
						else
						{
							m_out->print("Value: ").print(m_vmh.mem(Utils::strToInt(input), false)).newLine();
						}
					}
					else if (input.trim() == "last_inst")
					{
						printLastInst();
					}
					else
					{
						m_out->print("Unknown debug command.").newLine();
					}
					input = "";

				}
			}
			m_out->newLine().print("------------------------------------------------------------------").newLine();
			m_out->print("Total Memory used: ").print(proc.totalMemoryUsed() * 4).print(" bytes");
			m_out->print(" (").print(proc.totalMemoryUsed()).print(" Cells)").newLine();
			return true;
		}
	}
}