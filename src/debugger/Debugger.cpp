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
			int32 __line_length = 125, __console_height = 0;
			Utils::get_terminal_size(__line_length, __console_height);
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
				out.tc_reset();
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
					out.clear();
					printSourceCode(0, 0, 0, __line_length);
					out.tc_reset();
					out.fc_magenta();
					OmniaString __tmp = "*** Press <Enter> to go back to prompt.";
					out.print(__tmp).print(Utils::duplicateChar(' ', __line_length - __tmp.length())).newLine();
					in.read(__tmp);
					out.clear();
					continue;
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
			TMemoryList __dec_inst;
			MemAddress __off_ip = oasm_nullptr, __ip = oasm_nullptr;
			uint8 __source_line_count = 13;
			uint8 __proc_out_line_count = 10;
			bool __first_cmd_step = true;
			bool __skip_next_input_round = false;
			bool __done = false;
            while (true)
            {
				__skip_next_input_round = false;
				__ip = cpu.getLastInstructionAddr();
				__off_ip = __ip - proc.m_codeAddr;
				exec_tick = true;
				if (cmd__step)
				{
					out.clear();
					if (__first_cmd_step)
					{
						__first_cmd_step = false;
						out.fc_magenta();
						out.print("*** Running process (step-by-step)...").newLine();
						out.print("*** Press <Enter> to run the first instruction.").newLine();
						out.tc_reset();
						OmniaString __tmp = "";
						in.read(__tmp);
						__skip_next_input_round = true;
					}
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
					
					out.fc_blue().bc_brightMagenta();
					OmniaString __dec_inst_text(" Decoded instruction: ");
					out.print(__dec_inst_text);
					out.fc_brightWhite().bc_brightBlue();
					if (__dec_inst.size() > 0)
					{
						StringBuilder __sb = StringBuilder();
						for (auto& __cell : __dec_inst)
							__sb.add(Utils::intToHexStr(__cell.val())).add(", ");
						OmniaString __di = __sb.get();
						__di = __di.trim().substr(0, __di.length() - 2);
						word __space = (__line_length - __dec_inst_text.length() - __di.length()) / 2;
						out.print(Utils::duplicateChar(' ', __space + 1)).print(__di).print(Utils::duplicateChar(' ', __space)).newLine();
					}
					else
						out.print(Utils::duplicateChar(' ', __line_length - __dec_inst_text.length())).newLine();

					out.fc_blue().bc_white();
					out.print(" Script Output:").newLine();
					out.print(Utils::duplicateChar('=', __line_length)).newLine();

					out.tc_reset();
					out.fc_brightWhite().bc_brightGrey();
					if (__proc_out_buffer.size() <= __proc_out_line_count)
					{
						uint32 __i = 0;
						for (auto& line : __proc_out_buffer)
						{
							out.print("    ").print(line).print(Utils::duplicateChar(' ', __line_length - (line.length() + 4))).newLine();
							__i++;
						}
						for ( ; __i < __proc_out_line_count; __i++)
							out.print(Utils::duplicateChar(' ', __line_length)).newLine();
					}
					else
					{
						OmniaString __l = "";
						for (uint32 __i = __proc_out_line_count; __i > 0; __i--)
						{
							__l = __proc_out_buffer[__proc_out_buffer.size() - __i];
							out.print("    ").print(__l).print(Utils::duplicateChar(' ', __line_length - (__l.length() + 4))).newLine();
						}
					}
					out.fc_blue().bc_white();
					out.print(Utils::duplicateChar('=', __line_length)).newLine();
					out.tc_reset();
					if (__br_sig)
					{
						out.fc_magenta();
						out.print("*** Reached break-point: Code-Address=").print(Utils::intToHexStr(cpu.__break_point_address()));
						out.tc_reset();
						out.newLine();
						__br_sig = false;
					}
					if (proc.done())
					{
						out.fc_brightWhite().bc_green();
						out.print("*** Process terminated naturally.").newLine();
						out.tc_reset();
						out.newLine();
						__done = true;
						//break;
					}
					if (!__skip_next_input_round)
					{
						out.fc_magenta();
						out.print("*** Press <Enter> for next instructions.").newLine();
						out.fc_magenta().bc_brightYellow();
						out.print(m_prompt);
						out.tc_reset();
						in.read(m_cmd_input);
						m_cmd_input = m_cmd_input.trim().toLowerCase();
						if (m_cmd_input == "mem")
						{
							exec_tick = false;
							vm.getCPU().printMemory(out, 4, 4, 16, true);
						}
						else if (m_cmd_input == "quit" || m_cmd_input == "q")
						{
							out.fc_magenta();
							out.print("*** Exiting with force...");
							out.tc_reset();
							out.newLine();
							break;
						}
						else if (m_cmd_input == "resume" || m_cmd_input == "r")
						{
							__br_sig = false;
							exec_tick = true;
							cmd__step = false;
						}
						else if (m_cmd_input == "print-call-tree")
						{
							exec_tick = false;
							out.clear().tc_reset();
							m_sym_table.m_callTree.print(out);
							out.tc_reset();
							out.fc_magenta();
							out.print("*** Press <Enter> to return to step-by-step prompt.").newLine();
							OmniaString __tmp = "";
							in.read(__tmp);
							out.clear().tc_reset();
						}
					}
				}
				if (!exec_tick || __done) continue;
				tick_res = cpu.clock_tick();
				__br_sig = cpu.__break_point_signal();
				__dec_inst = cpu.getDecodedInstruction();
				if (__dec_inst.size() >= 3 && __dec_inst[0].val() == (word)eInstructionSet::call)
				{
					OmniaString __tmp_lbl = "";
					if (m_sym_table.isLabel(__dec_inst[2].val(), __tmp_lbl))
						m_sym_table.m_callTree.call(__tmp_lbl);
					else
						m_sym_table.m_callTree.call(Utils::intToHexStr(__dec_inst[2].val()));

				}
				else if (__dec_inst.size() >= 1 && __dec_inst[0].val() == (word)eInstructionSet::ret)
				{
					m_sym_table.m_callTree.ret();
				}
				else if (__dec_inst.size() >= 1 && __dec_inst[0].val() == (word)eInstructionSet::end)
				{
					m_sym_table.m_callTree.ret();
				}
				if (!tick_res)
				{
					out.fc_brightWhite().bc_red();
					out.print("*** Runtime error.");
					out.tc_reset();
					out.newLine();
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
					out.fc_brightWhite().bc_green();
					out.print("*** Process terminated naturally.");
					out.tc_reset();
					out.newLine();
					break;
				}
				if (__br_sig)
				{
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
			out.fc_blue().bc_white();
			out.print(" Source Code: ").newLine();
			out.print(Utils::duplicateChar('=', __line_w)).newLine();
			out.tc_reset();
			uint32 __ln = 0, __old_ln = 0;
			bool __oln_set = false;
			StringBuilder __sb;
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
					__ln++;
					__sb = StringBuilder(":");
					__sb.add(__tmp_lbl).add(":");
					__sb.add(Utils::duplicateChar(' ', __line_w - __sb.get().length()));

					out.fc_brightWhite().bc_brightGrey();
					out.print(__sb.get()).newLine();
					out.tc_reset();
				}
				if (__line_count != 0 && __ln > __old_ln + __line_count) break;
				__tmp_line = __line.second;
				if (__highlight != 0xFFFF && __line.first == __highlight)
				{
					printInstructionLine(__line, __line_w, true, false);
				}
				else if (VirtualMachine::instance().getCPU().isBreakPoint(__line.first, __line.second.tokenize(",").count()))
				{
					printInstructionLine(__line, __line_w, false, true);
				}
				else
				{
					printInstructionLine(__line, __line_w, false, false);
				}
				__ln++;
				out.tc_reset();
			}
			out.tc_reset();
			return true;
		}

		bool Debugger::printInstructionLine(std::pair<MemAddress, OmniaString> __line, word __line_w, bool __hl, bool __rhl)
		{
			word __inst_column_width = 10;
			OutputManager& out = *getOutputHandler();
			word __c_line_len = 0;
			word __tmp_space = 0;

			StringBuilder __sb("   ");
			__sb.add(Utils::intToHexStr(__line.first)).add("   ");
			__c_line_len += __sb.get().length();
			if (__hl) out.fc_red().bc_brightGreen();
			else if (__rhl) out.fc_white().bc_red();
			else out.fc_brightYellow().bc_blue();
			out.print(__sb.get());
			OmniaString::StringTokens __st = __line.second.tokenize(",", true);

			OmniaString __tmp = __st.next();
			if (__hl) out.fc_brightWhite().bc_red();
			else out.fc_brightWhite().bc_yellow();
			out.print(__tmp);
			if (__hl) out.fc_brightWhite().bc_green();
			else out.fc_brightMagenta().bc_blue();
			__tmp_space = __inst_column_width - __tmp.length();
			out.print(Utils::duplicateChar(' ', __tmp_space));
			__c_line_len += __tmp_space;
			__c_line_len += __tmp.length();
			
			if (__hl) out.fc_brightWhite().bc_green();
			else out.fc_brightMagenta().bc_blue();

			while (__st.hasNext())
			{
				__tmp = __st.next();
				if (Utils::isInt(__tmp))
					out.fc_red().bc_brightYellow();
				else if (m_sym_table.isReserve(__tmp))
					out.fc_grey().bc_white();
				else if (__hl)
					out.fc_brightWhite().bc_green();
				else
					out.fc_brightMagenta().bc_blue();
				
				out.print(__tmp);			
				if (__hl) out.fc_grey().bc_green();
				else out.fc_brightWhite().bc_blue();
				if (__st.hasNext())
				{
					out.print(",  ");
					__c_line_len += __tmp.length() + 3;
				}
				else
					__c_line_len += __tmp.length();
			}


			//__sb = StringBuilder();
			//__sb.add(__line.second);
			//__c_line_len += __sb.get().length();
			//out.print(__sb.get());
			out.print(Utils::duplicateChar(' ', __line_w - __c_line_len)).newLine();
			out.tc_reset();
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