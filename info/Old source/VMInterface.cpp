#include "VMInterface.hpp"
#include "ProcessManager.hpp"
#include "Assembler.hpp"
#include "StringTokens.hpp"
#include "Debugger.hpp"

namespace Omnia
{
	namespace oasm
	{
		_int32 VMLoader::run(std::vector<_string>& args)
		{
			_int32 heap_print_size = 200, stack_print_size = 200;
			String pm_o_file = "";
			bool pm_o_registers = true, pm_o_process = true, pm_o_stack = true, pm_o_heap = true;
			bool o_compile = false, o_printMemory = false, o_printProcInfo = false;
			bool o_debugAll = false, o_debug = false;
			std::vector<String> argd;

			for (auto& o : args)
			{
				String so = String(o).trim();
				if (so == "-c")
				{
					o_compile = true;
					o = "";
				}
				else if (so.startsWith("--print-memory"))
				{
					o = "";
					o_printMemory = true;
					so = so.substr(14).trim();
					if (so.startsWith("[") && so.endsWith("]"))
					{
						so = so.substr(1, so.length() - 1).trim();
						StringTokens st = so.tokenize(",", true);
						while (st.hasNext())
						{
							String tok = st.next();
							if (!tok.contains("=")) continue; //TODO: Error
							String p = tok.substr(0, tok.indexOf("=")).trim();
							String d = tok.substr(tok.indexOf("=") + 1).trim();
							if (p == "stack")
							{
								if (Utils::isInt(d))
								{
									stack_print_size = Utils::strToInt(d);
									pm_o_stack = stack_print_size > 0;
								}//TODO: Else error
							}
							else if (p == "heap")
							{
								if (Utils::isInt(d))
								{
									heap_print_size = Utils::strToInt(d);
									pm_o_heap = heap_print_size > 0;
								}//TODO: Else error
							}
							else if (p == "regs")
							{
								if (d == "true") pm_o_registers = true;
								else if (d == "false") pm_o_registers = false;
								//TODO: Else error
							}
							else if (p == "proc")
							{
								if (d == "true") pm_o_process = true;
								else if (d == "false") pm_o_process = false;
								//TODO: Else error
							}
							else if (p == "file")
							{
								pm_o_file = d;
								//TODO: Else error
							}
							//TODO: Else error
						}
					}
				}
				else if (so == "--debug-all")
				{
					o_debugAll = true;
					o = "";
				}
				else if (so == "--show-frame")
				{
					o_printProcInfo = true;
					o = "";
				}

				if (o != "") argd.push_back(so);
			}

			if (o_compile && argd.size() > 1)
			{
				String out_file = "out.oex", in_file = "";
				for (_uint32 i = 0; i < argd.size(); i++)
				{
					String arg = argd[i];
					if (arg == "-o")
					{
						if (i < argd.size() - 1)
							out_file = argd[++i];
						//TODO: Else error
					}
					else if (arg == "-i")
					{
						if (i < argd.size() - 1)
							in_file = argd[++i];
						//TODO: Else error
					}    
				}
				Assembler::instance().assembleToFile(in_file, out_file);
			}
			else if (o_debugAll)
			{
				String oex = argd[0];
				DebugInfo dbg;
				//dbg.useAll();
				//TODO: Convert args to _int32 vector
				if (!Debugger::instance().run(oex, dbg)) //TODO: Pass args
				{
					m_out->print("Error debugging process.").newLine();
					return 10;
				}
			}
			else
			{
				String oex = argd[0];

				//TODO: Convert args to _int32 vector

				if (o_printProcInfo)
				{
					m_out->newLine().print("Script output");
					m_out->newLine().print("------------------------------------------------------------------").newLine();
				}
				if (!ProcessManager::instance().runFromFile(oex)) //TODO: Pass args
				{
					m_out->print("Error running process.").newLine();
					return 2;
				}
				if (ProcessManager::instance().getRunningProcess() == nullptr)
				{
					m_out->print("Null running proc.").newLine();
					return 1;
				}
				Process& proc = *ProcessManager::instance().getRunningProcess();
				while (ProcessManager::instance().tick());
				if (o_printProcInfo)
				{
					m_out->newLine().print("------------------------------------------------------------------").newLine();
					m_out->print("Total Memory used: ").print(proc.totalMemoryUsed() * 4).print(" bytes");
					m_out->print(" (").print(proc.totalMemoryUsed()).print(" Cells)").newLine();
				}
				else m_out->newLine();

				if (o_printMemory)
				{
					TextFileOutput tfo;
					OutputManager* tmp = ProcessManager::instance().getOutputHandler();
					if (pm_o_file != "")
					{
						tfo.fileName = pm_o_file;
						tfo.openOutputFile();
						ProcessManager::instance().setOutputHandler(&tfo);
					}

					if (pm_o_heap)
						ProcessManager::instance().printMemoryBlock(ProcessManager::instance().getHeapStart(), ProcessManager::instance().getHeapStart() + heap_print_size, "Heap");
					if (pm_o_stack)
						ProcessManager::instance().printMemoryBlock(ProcessManager::instance().getStackStart(), ProcessManager::instance().getStackStart() + stack_print_size, "Stack");
					if (pm_o_registers)
						ProcessManager::instance().printMemoryBlock(0, (_uint32)eRegisters::MemStart, "Registers");
					if (pm_o_process)
						ProcessManager::instance().printMemoryBlock(proc.m_startAddr, proc.m_startAddr + proc.m_codeSize + proc.m_dataSize, "Process");

					if (pm_o_file != "")
					{
						tfo.closeOutputFile();
						ProcessManager::instance().setOutputHandler(tmp);
					}
				}
			}
			return 0;
		}
	}
}