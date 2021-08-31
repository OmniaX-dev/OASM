#include "Debugger.hpp"
#include "Interpreter.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace Omnia
{
	namespace oasm
	{
		int64 Debugger::run(int argc, char** argv)
		{
			m_prompt = "(odb) > ";
			m_currentSourceLine = 0;
			m_currentSourceLine--;
			OmniaString outPrompt = "( - ) #   ";
            OutputManager& out = *getOutputHandler();
            InputManager& in = *getInputHandler();
			VirtualMachine &vm = VirtualMachine::instance();
			//hw::RAM &ram = vm.getRAM();
			//hw::REG &reg = vm.getREG();
			hw::CPU &cpu = vm.getCPU();
			//hw::GPU &gpu = vm.getGPU();
            vm.setOutputHandler(&m_vm_buff);
            Interpreter::instance().setOutputHandler(&out);
			OmniaString p__input_file_path = "";
			OmniaString p__input_sym_table_path = "";
			m_use_sym_table = false;
			if (argc > 1)
			{
				for (int i = 1; i < argc; i++)
				{
					if (OmniaString(argv[i]).trim().equals("--input-file") || OmniaString(argv[i]).trim().equals("-i"))
					{
						if (i + 1 >= argc)
						{
							out.print("*** Error: No input file specified.").newLine();
							return 0xFFFF; //TODO: Add error code
						}
						i++;
						p__input_file_path = OmniaString(argv[i]);
					}
					else if (OmniaString(argv[i]).trim().equals("--debug-table") || OmniaString(argv[i]).trim().equals("-dt"))
					{
						if (i + 1 >= argc)
						{
							out.print("*** Error: No input debug table file specified.").newLine();
							return 0xFFFF; //TODO: Add error code
						}
						i++;
						p__input_sym_table_path = OmniaString(argv[i]);
						m_use_sym_table = true;
					}

				}
			}
            if (p__input_file_path.trim() == "")
            {
                out.print("*** Error: No input file specified.").newLine();
                return 0xFFFE; //TODO: Add error code
            }
			if (!m_use_sym_table || p__input_sym_table_path.trim() == "")
			{
				OmniaString __tmp_input_file_name = p__input_file_path.substr(0, p__input_file_path.lastIndexOf("."));
				__tmp_input_file_name = __tmp_input_file_name.add(".odb");
				if (std::filesystem::exists(__tmp_input_file_name.cpp()))
				{
					out.print("*** Symbol table file found: ").print(__tmp_input_file_name).newLine();
					m_use_sym_table = true;
					p__input_sym_table_path = __tmp_input_file_name;
				}
			}
			if (m_use_sym_table && !loadSymTableFromFile(p__input_sym_table_path)) return 0xFFAC; //TODO: Error
            OmniaString __param3 = "--input-file ";
            __param3 = __param3.add(p__input_file_path);
            cpu.setIPC(1);
            
			ErrorCode __err_code;
			bool cmd__step = false;
			while (true)
			{
				out.print(m_prompt);
				in.read(m_cmd_input);
				m_cmd_input = m_cmd_input.trim().toLowerCase();
				if (m_cmd_input == "run" || m_cmd_input == "r")
				{
					out.print("*** Running process...").newLine();
					__err_code = Interpreter::instance().run(4, new char*[]{argv[0], (char*)"--debugger-internal-call", (char*)"--input-file", (char*)p__input_file_path.c_str()});
					if (__err_code != D__NO_ERROR)
					{
						//TODO: Error
						return __err_code;
					}
					break;
				}
				else if (m_cmd_input == "step" || m_cmd_input == "s")
				{
					cmd__step = true;
					out.print("*** Running process (step-by-step)...").newLine();
					out.print("*** Press <Enter> to step through instructions.").newLine();
					__err_code = Interpreter::instance().run(4, new char*[]{argv[0], (char*)"--debugger-internal-call", (char*)"--input-file", (char*)p__input_file_path.c_str()});
					if (__err_code != D__NO_ERROR)
					{
						//TODO: Error
						return __err_code;
					}
					break;
				}
				else if (m_cmd_input == "quit" || m_cmd_input == "q")
				{
					out.print("*** Exiting...").newLine();
					return 0x0000;
				}
				else if (m_cmd_input == "source")
				{
					if (!m_use_sym_table)
					{
						out.print("*** Warning: No Debug table loaded.").newLine();
						continue;
					}
					if (m_sym_table.m_source.size() == 0)
					{
						out.print("*** Warning: No source code loaded.").newLine();
						continue;
					}
					printSourceCode(0, oasm_nullptr, 0x0015);
				}
				else if (m_cmd_input.startsWith("break-point ") || m_cmd_input.startsWith("br "))
				{
					m_cmd_input = m_cmd_input.substr(m_cmd_input.indexOf(" ") + 1).trim();
					if (Utils::isInt(m_cmd_input))
					{
						cpu.addBreakPoint((word)Utils::strToInt(m_cmd_input));
						out.print("*** Break-point added at Code-Address ").print(m_cmd_input).newLine();
					}
				}
				else
				{
					out.print("*** Warning: unknown command.").newLine();
					continue;
				}
			}
            Process& proc = vm.getCurrentProcess();
			bool tick_res = false, exec_tick = true, __br_sig = false;
			std::vector<OmniaString> proc_out, __proc_out_buffer;
			MemAddress __off_ip = oasm_nullptr, __ip = oasm_nullptr;
			uint8 __source_line_count = 15;
			uint8 __proc_out_line_count = 10;
			word __line_length = 120;
            while (true)
            {
				__ip = cpu.getLastInstructionAddr();
				__off_ip = __ip - proc.m_codeAddr;
				exec_tick = true;
				if (cmd__step)
				{
					out.clear();
					if (m_use_sym_table)
					{
						if (m_sym_table.m_source.size() > 0 && __ip != oasm_nullptr)
						{
							m_currentSourceLine = findCurrentLine(__off_ip);
							if (m_currentSourceLine <= (uint32)((__source_line_count / 2) + 1))
								printSourceCode(0, __source_line_count, __off_ip, __line_length);
							//else if (m_currentSourceLine >= (uint32)(m_sym_table.m_source.size() - ( (__source_line_count / 2) + 1)))
							//	printSourceCode(m_sym_table.m_source.size() - __source_line_count, __source_line_count, __off_ip);
							else
								printSourceCode(m_currentSourceLine -  ((__source_line_count / 2) + 1), __source_line_count, __off_ip, __line_length);
						}
					}
					else
						out.print(Utils::duplicateChar('=', __line_length)).newLine();
					out.print(" Script Output:").newLine();
					out.print(Utils::duplicateChar('=', __line_length)).newLine();
					if (__proc_out_buffer.size() <= __proc_out_line_count)
					{
						uint32 __i = 0;
						for (auto& line : __proc_out_buffer)
						{
							out.tab().print(line).newLine();
							__i++;
						}
						for ( ; __i < __proc_out_line_count; __i++)
							out.newLine();
					}
					else
					{
						for (uint32 __i = __proc_out_line_count; __i > 0; __i--)
							out.tab().print(__proc_out_buffer[__proc_out_buffer.size() - __i]).newLine();
					}
					out.print(Utils::duplicateChar('=', __line_length)).newLine();
					if (proc.done())
					{
						out.print("*** Process terminated naturally.").newLine();
						break;
					}
                	out.print(m_prompt);
            		in.read(m_cmd_input);
                	m_cmd_input = m_cmd_input.trim().toLowerCase();
					if (m_cmd_input == "mem")
					{
						exec_tick = false;
						vm.getCPU().printMemory(out, 4, 4, 16, true);
					}
					else if (m_cmd_input == "quit" || m_cmd_input == "q")
					{
						out.print("*** Exiting with force...");
						break;
					}
					else if (m_cmd_input == "resume" || m_cmd_input == "r")
					{
						__br_sig = false;
						exec_tick = true;
						cmd__step = false;
					}
				}
				if (!exec_tick) continue;
				tick_res = cpu.clock_tick();
				__br_sig = cpu.__break_point_signal();
				if (!tick_res)
				{
					out.print("*** Runtime error.").newLine();
					cmd__step = true;
				}
				proc_out = m_vm_buff.flush();
				if (proc_out.size() > 0)
				{
					for (auto& line : proc_out)
					{
						line = line.replaceAll("\n", "");
						if (line.trim() != "")
						{
							//if (cmd__step) out.print(Utils::duplicateChar('-', 100)).newLine();
							if (!cmd__step) out.print(outPrompt).print(line).newLine();
							//if (cmd__step) out.print(Utils::duplicateChar('-', 100)).newLine();
							__proc_out_buffer.push_back(line);
						}
					}
				}
				if (!cmd__step && proc.done())
				{
					out.print("*** Process terminated naturally.").newLine();
					break;
				}
				if (__br_sig)
				{
					out.print("*** Reached break-point: Code-Address=").print(Utils::intToHexStr(cpu.__break_point_address())).newLine();
					cmd__step = true;
				}
            }
			return vm.getCPU().getLastErrorCode();
		}

		uint32 Debugger::findCurrentLine(MemAddress __off_ip)
		{
			if (m_use_sym_table)
			{
				if (m_sym_table.m_source.size() > 0 && __off_ip != oasm_nullptr)
				{
					uint32 __ln = 0;
					for (auto& __line : m_sym_table.m_source)
					{
						if (__line.first == __off_ip) return __ln;
						__ln++;
					}
				}
			}
			return 0;
		}

		bool Debugger::printSourceCode(uint32 __start, uint32 __line_count, MemAddress __highlight, word __line_w)
		{
			OutputManager& out = *getOutputHandler();
			OmniaString __tmp_lbl = "", __tmp_line = "";
			out.print(Utils::duplicateChar('=', __line_w)).newLine();
			out.print(" Source Code: ").newLine();
			out.print(Utils::duplicateChar('=', __line_w)).newLine();
			uint32 __ln = 0, __old_ln = 0;
			bool __oln_set = false;
			for (auto& __line : m_sym_table.m_source)
			{
				if (__ln < __start)
				{
					__ln++;
					continue;
				}
				if (!__oln_set)
				{
					__old_ln = __ln;
					__oln_set = true;
				}
				if (__line_count != 0 && __ln > __old_ln + __line_count) break;
				if (m_sym_table.isLabel(__line.first, __tmp_lbl))
				{
					//if (__line_count != 0) __line_count--;
					//if (__line_count != 0 && __ln > __old_ln + __line_count) break;
					__ln++;
					out.print(":").print(__tmp_lbl).print(":").newLine();
				}
				if (__line_count != 0 && __ln > __old_ln + __line_count) break;
				__tmp_line = __line.second;
				if (__highlight != 0xFFFF && __line.first == __highlight)
				{
					StringBuilder __sb;
					__sb.add(">>>>----").add(Utils::intToHexStr(__line.first)).add("--------");
					__tmp_line = __tmp_line.replaceAll(" ", "-");
					__sb.add(__tmp_line);
					out.print(__sb.get()).print(Utils::duplicateChar('-', __line_w - __sb.get().length() - 4)).print("<<<<").newLine();
				}
				else
				{
					out.tab().tab().print(Utils::intToHexStr(__line.first)).tab().tab().print(__tmp_line).newLine();
				}
				__ln++;
			}
			out.print(Utils::duplicateChar('=', __line_w)).newLine();
			return true;
		}

		bool Debugger::loadSymTableFromFile(OmniaString __sym_table_file_path)
		{
			std::vector<OmniaString> lines;
            if (!Utils::readFile(__sym_table_file_path, lines))
            {
				//TODO: Error
                return false;
            }
            if (lines.size() == 0)
            {
				//TODO: Error
                return false;
            }

			OmniaString __symbol = "";
			MemAddress __addr = 0;
			uint32 __sym_count = 0, __src_lines = 0;
			bool __in_source = false;
			for (auto& __line : lines)
			{
				__line = __line.trim();
				if (!__in_source && !__line.contains("=")) continue;
				if (__line.toLowerCase().startsWith(".label"))
				{
					__line = __line.substr(6).trim();
					__symbol = __line.substr(0, __line.indexOf("=")).trim();
					__addr = (MemAddress)Utils::strToInt(__line.substr(__line.indexOf("=") + 1).trim());
					m_sym_table.m_labels[__addr] = __symbol;
					__sym_count++;
				}
				else if (!__in_source && __line.toLowerCase().startsWith(".data"))
				{
					__line = __line.substr(5).trim();
					__symbol = __line.substr(0, __line.indexOf("=")).trim();
					__addr = (MemAddress)Utils::strToInt(__line.substr(__line.indexOf("=") + 1).trim());
					m_sym_table.m_reserves[__addr] = __symbol;
					__sym_count++;
				}
				else if (!__in_source && __line.toLowerCase().startsWith(".source = {"))
				{
					__in_source = true;
				}
				else if (__in_source && __line.toLowerCase().startsWith("}"))
				{
					__in_source = false;
				}
				else if (__in_source)
				{
					__src_lines++;
					if (!__line.contains(":")) continue;
					OmniaString __tmp_addr = __line.substr(0, __line.indexOf(":"));
					if (!Utils::isInt(__tmp_addr)) continue;
					__line = __line.substr(__line.indexOf(":") + 1);
					m_sym_table.m_source[(MemAddress)(Utils::strToInt(__tmp_addr))] = __line;
				}
			}
			getOutputHandler()->print("*** Debug table loaded successfully: ").print((word)__sym_count).print(" Symbols found.").newLine();
			if (m_sym_table.m_source.size() > 0)
				getOutputHandler()->print("*** Source loaded: ").print((word)__src_lines).print(" lines.").newLine();
			return true;
		}
	}
}