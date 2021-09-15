#include "Debugger.hpp"
#include "Interpreter.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Assembler.hpp"
#include <unistd.h>

namespace Omnia
{
	namespace oasm	
	{
		void Debugger::tRegChangeTracker::updadte(void)
		{
			BitEditor __new_val;
			hw::REG& reg = VirtualMachine::instance().getREG();
			if (!loaded)
			{
				changeTime = 5;
				loaded = true;
				__new_val = oldValue;
			}
			else
				reg.read(this->reg, __new_val);
			if (__new_val != oldValue)
			{
				oldValue = __new_val;
				changeTime = 0;
			}
			else if (changeTime < 5) changeTime++;
		}

		Debugger::tRegChangeTracker::tRegChangeTracker(eRegisters __reg)
		{
			changeTime = 5;
			reg = __reg;
			loaded = false;
			VirtualMachine::instance().getREG().disableProtectedMode();
			VirtualMachine::instance().getREG().read(reg, oldValue);
			VirtualMachine::instance().getREG().enableProtectedMode();
		}


		void Debugger::tMemChangeTracker::updadte(void)
		{
			BitEditor __new_val;
			hw::RAM& ram = VirtualMachine::instance().getRAM();
			if (!loaded)
			{
				changeTime = 5;
				loaded = true;
				__new_val = oldValue;
			}
			else
				__new_val = ram.getAsReadOnly()[addr];
			if (__new_val != oldValue)
			{
				oldValue = __new_val;
				changeTime = 0;
			}
			else if (changeTime < 5) changeTime++;
		}

		Debugger::tMemChangeTracker::tMemChangeTracker(MemAddress __addr)
		{
			changeTime = 5;
			addr = __addr;
			loaded = false;
			VirtualMachine::instance().getRAM().disableProtectedMode();
			VirtualMachine::instance().getRAM().read(addr, oldValue);
			VirtualMachine::instance().getRAM().enableProtectedMode();
		}



		int64 Debugger::run(int argc, char** argv)
		{
			Utils::get_terminal_size(__line_length, __console_height);
			m_prompt = "(odb) > ";
			outPrompt = "( - ) #   ";
			m_as_args = nullptr;
			m_process = nullptr;
            OutputManager& out = *getOutputHandler();
            //InputManager& in = *getInputHandler();
            vm.setOutputHandler(&m_vm_buff);
			cpu.redirectErrorsTo(*this);
			gpu.redirectErrorsTo(*this);
			reg.redirectErrorsTo(*this);
			ram.redirectErrorsTo(*this);
            Interpreter::instance().setOutputHandler(&out);
            Assembler::instance().setOutputHandler(&out);
			p__input_file_path = "";
			p__input_sym_table_path = "";
			m_use_sym_table = false;
			m_allow_diff_version = false;


			m_gui_block_top = eGuiBlock::Source;
			m_gui_block_middle = eGuiBlock::Heap;
			m_gui_block_bottom = eGuiBlock::Output;
			m_gui_block_extra = eGuiBlock::Log;
			m_show_data = true;
			m_g_source_lines = 9;
			m_g_out_lines = 7;
			m_g_heap_lines = 7;
			m_g_stack_lines = 7;
			m_g_code_lines = 7;
			m_g_lib_lines = 7;
			m_g_tree_lines = 7;
			m_g_log_lines = 5;
			m_gui_error_on_block = eGuiBlockPosition::Extra;
			m_gui_data_under = eGuiBlockPosition::Top;

			out.clear();
			message(StringBuilder("oasm_dbg: version ")
			.add((long int)Omnia::eVersion::Major).add(".").add((long int)Omnia::eVersion::Minor)
			.add(".").add((long int)Omnia::eVersion::Build).get(), eMsgType::Version, true, true, true);

			if (argc > 1)
			{
				for (int i = 1; i < argc; i++)
				{
					if (OmniaString(argv[i]).trim().equals("--input-file") || OmniaString(argv[i]).trim().equals("-i"))
					{
						if (i + 1 >= argc)
						{
							message("Error: No input file specified.", eMsgType::Error, true, true, true);
							return 0xFFFF; //TODO: Add error code
						}
						i++;
						p__input_file_path = OmniaString(argv[i]);
					}
					else if (OmniaString(argv[i]).trim().equals("--debug-table") || OmniaString(argv[i]).trim().equals("-dt"))
					{
						if (i + 1 >= argc)
						{
							message("Error: No input debug table file specified.", eMsgType::Error, true, true, true);
							return 0xFFFF; //TODO: Add error code
						}
						i++;
						p__input_sym_table_path = OmniaString(argv[i]);
						m_use_sym_table = true;
					}
					else if (OmniaString(argv[i]).trim().equals("--allow-diff-ver"))
					{
						m_allow_diff_version = true;
					}
				}
			}
            if (p__input_file_path.trim() == "")
            {
                message("Error: No input file specified.", eMsgType::Error, true, true, true);
                return 0xFFFE; //TODO: Add error code
            }
			if (!m_use_sym_table || p__input_sym_table_path.trim() == "")
			{
				OmniaString __tmp_input_file_name = p__input_file_path.substr(0, p__input_file_path.lastIndexOf("."));
				__tmp_input_file_name = __tmp_input_file_name.add(".odb");
				if (std::filesystem::exists(__tmp_input_file_name.cpp()))
				{
					message(StringBuilder("Symbol table file found: ").add(__tmp_input_file_name).get(), eMsgType::Info, true, true, true);
					m_use_sym_table = true;
					p__input_sym_table_path = __tmp_input_file_name;
				}
			}
			if (m_use_sym_table && !loadSymTableFromFile(p__input_sym_table_path)) return 0xFFAC; //TODO: Error

            topLevelPrompt();


            m_process = &vm.getCurrentProcess();
			Process& proc = *m_process;

			reset();
            while (true)
            {
				if (m_force_quit) break;
				if (reg.IP() >= cpu.offsetCodeAddress(proc.m_codeSize)) __done = true;
				if (proc.done()) __done = true;
				if (__done) exec_tick = false;
				__skip_next_input_round = false;
				if (exec_tick && !m_error && !__done)
				{
					ram.disableProtectedMode(); //TODO: Find out why I get an error on <end> instruction
					tick_res = cpu.clock_tick();
					ram.enableProtectedMode();
					__ip = cpu.getLastInstructionAddr();
					__off_ip = __ip - proc.m_codeAddr;

					m_sym_table.m_callTree.tick();
					__total_tick_count++;
					__br_sig = cpu.__break_point_signal();
					if (__br_sig)
					{
						message(StringBuilder("Reached break-point: Code-Address=").add(Utils::intToHexStr(cpu.__break_point_address())).get(), eMsgType::Info, true, false, true);
						__br_sig = false;
					}
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
						__done = true;
					}
				}
				if (m_mode == eDebuggerMode::StepByStep)
				{
					stepMode(proc);
				}
				else if (m_mode == eDebuggerMode::Normal)
				{
					runProcess(proc);
				}
				if (!m_skip_next_draw)
					draw(proc);
				m_skip_next_draw = false;
				if (__done && !cmd__step)
				{
					message("Execution terminated naturally...", eMsgType::Success, true, cmd__step, true);
					break;
				}
            }
			if (m_as_args != nullptr) delete[] m_as_args;
			out.tc_reset();
			return vm.getCPU().getLastErrorCode();
		}

		void Debugger::reset(void)
		{
			m_regChangeTable.clear();
			m_memChangeTable.clear();
			for (MemAddress __addr = 0; __addr < (word)eRegisters::Count; __addr++)
				m_regChangeTable.push_back(tRegChangeTracker((eRegisters)__addr));
			for (MemAddress __addr = 0; __addr < D__MEMORY_SIZE; __addr++)
				m_memChangeTable.push_back(tMemChangeTracker(__addr));
			tick_res = false;
			exec_tick = true;
			__br_sig = false;
			proc_out.clear();
			__proc_out_buffer.clear();
			__dec_inst.clear();
			m_err_data.clear();
			__off_ip = oasm_nullptr;
			__ip = oasm_nullptr;
			__first_cmd_step = true;
			__skip_next_input_round = false;
			__done = false;
			__total_tick_count = 0;
			m_print_end_msg = true;
			if (m_as_args != nullptr) delete[] m_as_args;
			m_as_args = nullptr;
			m_error = false;
			m_cursor = 0;
			m_currentSourceLine = 0;
			m_skip_next_draw = false;
		}

		void Debugger::draw(Process& proc)
		{
			OutputManager& out = *getOutputHandler();
			out.tc_reset().clear();
			if (m_mode == eDebuggerMode::Normal)
			{
				for (auto& __log : m_msg_log)
					message(__log.second, __log.first, false, true, true);
				out.bc_brightWhite().fc_grey();
				out.print("*** Script output:").newLine();
				out.tc_reset();
				for (auto& __out : __proc_out_buffer)
				{
					out.fc_yellow();
					out.print(" + ");
					out.bc_grey().fc_brightWhite();
					out.print(__out).newLine();
				}
			}
			else if (m_mode == eDebuggerMode::StepByStep)
			{
				printFullGui(proc);
			}
			out.tc_reset();
		}

		bool Debugger::topLevelPrompt(void)
		{
			OutputManager& out = *getOutputHandler();
			InputManager& in = *getInputHandler();

			OmniaString __param3 = "--input-file ";
            __param3 = __param3.add(p__input_file_path);
            cpu.setIPC(1);
            
			ErrorCode __err_code;
			cmd__step = false;
			__add_br_on_end = false;
			while (true)
			{
				out.fc_cyan();
				out.print(m_prompt);
				out.tc_reset();
				in.read(m_cmd_input);
				m_cmd_input = m_cmd_input.trim();
				if (m_cmd_input == "run" || m_cmd_input == "r")
				{
					m_mode = eDebuggerMode::Normal;
					message("Running process...", eMsgType::Special, true, true, true);
					__err_code = Interpreter::instance().run(4, new char*[]{(char*)"dbg-vm", (char*)"--debugger-internal-call", (char*)"--input-file", (char*)p__input_file_path.c_str()});
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
					m_mode = eDebuggerMode::StepByStep;
					__err_code = Interpreter::instance().run(4, new char*[]{(char*)"dbg-vm", (char*)"--debugger-internal-call", (char*)"--input-file", (char*)p__input_file_path.c_str()});
					if (__err_code != D__NO_ERROR)
					{
						//TODO: Error
						return __err_code;
					}
					break;
				}
				else if (m_cmd_input == "quit" || m_cmd_input == "q")
				{
					message("*** Exiting...", eMsgType::Info, true, true, true);
					return 0x0000;
				}
				else if (m_cmd_input == "source")
				{
					if (!m_use_sym_table)
					{
						message("Warning: No Debug table loaded.", eMsgType::Info, true, true, true);
						continue;
					}
					if (m_sym_table.m_source.size() == 0)
					{
						message("Warning: No source code loaded.", eMsgType::Info, true, true, true);
						continue;
					}
					out.clear();
					printSourceCode(0, 0, 0);
					out.tc_reset();
					message("Press <Enter> to go back to prompt.", eMsgType::Info, false, true, true);
					OmniaString __tmp;
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
						message(StringBuilder("Break-point added at Code-Address ").add(m_cmd_input).get(), eMsgType::Info, true, true, true);
					}
					else if (m_sym_table.isLabel(m_cmd_input))
					{
						for (auto& __lbl : m_sym_table.m_labels)
						{
							if (__lbl.second == m_cmd_input)
							{
								cpu.addBreakPoint(__lbl.first);
								message(StringBuilder("Break-point added on label <").add(m_cmd_input).add("> at Code-Address ").add(__lbl.first).get(), eMsgType::Info, true, true, true);
								break;
							}
						}
					}
				}
				else if (m_cmd_input == "break-on-end" || m_cmd_input == "be")
				{
					__add_br_on_end = true;
					message(StringBuilder("Break-points will be added on all <end> instructions").add(m_cmd_input).get(), eMsgType::Info, true, true, true);
				}
				else if (m_cmd_input.startsWith("assemble ") || m_cmd_input.startsWith("as "))
				{
					OmniaString::StringTokens __st = m_cmd_input.tokenize(" ", true);
					if (m_as_args != nullptr) delete[] m_as_args;
					m_as_args = new char*[__st.count()];
					auto __vec = __st.array();
					for (uint16 i = 0; i < __vec.size(); i++)
					{
						m_as_args[i] = const_cast<char*>(__vec[i].c_str());
					}
					ErrorCode __err = Assembler::instance().run(__st.count(), m_as_args);
					if (__err != D__NO_ERROR)
						out.print("*** Exit error: ").print(Utils::intToHexStr((word)__err)).newLine();
				}
				else
				{
					message("Warning: unknown command.", eMsgType::Error, true, true, true);
					continue;
				}
			}
			return true;
		}

		void Debugger::runProcess(Process& proc)
		{
			if (__add_br_on_end)
			{
				MemAddress __end_addr = oasm_nullptr;
				for (auto& __cell : proc.m_code)
				{
					if (__cell == (word)eInstructionSet::end)
					{
						cpu.addBreakPoint(__end_addr);
						message(StringBuilder("Break-point added on <end> instruction at Code-Address ").add(Utils::intToHexStr(__end_addr)).get(), eMsgType::Info, true, true, true);
					}
					__end_addr++;
				}
				__add_br_on_end = false;
			}
			if (__first_cmd_step)
			{
				__skip_next_input_round = true;
				__first_cmd_step = false;
				return;
			}
			if (!tick_res)
			{
				cmd__step = true;
				__first_cmd_step = false;
				__skip_next_input_round = true;
				exec_tick = false;
				m_mode = eDebuggerMode::StepByStep;
				message("Runtime Error.", eMsgType::Error, true, true, true);
				message("Press <enter> to switch to Debug-view.", eMsgType::Info, false, true, true);
				OmniaString __tmp;
				getInputHandler()->read(__tmp);
				return;
			}
			m_sym_table.m_callTree.tick();
			__total_tick_count++;
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
			proc_out = m_vm_buff.flush();
			if (proc_out.size() > 0)
			{
				for (auto& line : proc_out)
				{
					line = line.replaceAll("\n", "");
					if (line.trim() != "")
					{
						__proc_out_buffer.push_back(line);
					}
				}
			}
			if (__br_sig)
			{
				cmd__step = true;
				m_mode = eDebuggerMode::StepByStep;
				exec_tick = false;
				return;
			}
			if (proc.done()) 
				__done = true;
		}
		
		void Debugger::stepMode(Process& proc)
		{
			InputManager& in = *getInputHandler();
			if (__first_cmd_step)
			{
				__first_cmd_step = false;
				message("Running process (step-by-step)...", eMsgType::Special, true, true, true);
				if (__add_br_on_end)
				{
					MemAddress __end_addr = oasm_nullptr;
					for (auto& __cell : proc.m_code)
					{
						if (__cell == (word)eInstructionSet::end)
						{
							cpu.addBreakPoint(__end_addr);
							message(StringBuilder("Break-point added on <end> instruction at Code-Address ").add(Utils::intToHexStr(__end_addr)).get(), eMsgType::Info, true, true);
						}
						__end_addr++;
					}
					__add_br_on_end = false;
				}
				message("Press <Enter> to run the first instruction.", eMsgType::Info, false, true, true);
				OmniaString __tmp = "";
				in.read(__tmp);
				__skip_next_input_round = true;
			}
			if (proc.done() || __done)
			{
				if (m_print_end_msg)
				{
					message(StringBuilder((int32)__total_tick_count).add(" total cycles executed.").get(), eMsgType::Info, true, false, true);
					message("Process terminated naturally.", eMsgType::Success, true, false, true);
					m_print_end_msg = false;
				}
				exec_tick = false;
			}
			inputPrompt();

			if (!exec_tick) return;

			proc_out = m_vm_buff.flush();
			if (proc_out.size() > 0)
			{
				for (auto& line : proc_out)
				{
					line = line.replaceAll("\n", "");
					if (line.trim() != "")
					{
						__proc_out_buffer.push_back(line);
					}
				}
			}
		}
		
		void Debugger::printFullGui(Process& proc)
		{
			OutputManager& out = *getOutputHandler();

			if (m_gui_error_on_block == eGuiBlockPosition::Top && m_error)
				printError();
			else if (m_gui_block_top == eGuiBlock::CallTree && m_use_sym_table)
				m_sym_table.m_callTree.print(out, __line_length, m_g_tree_lines);
			else if (m_gui_block_top == eGuiBlock::Code)
				printMemoryBlock(D__MEMORY_START, m_g_code_lines, "CODE MEMORY");
			else if (m_gui_block_top == eGuiBlock::Stack)
				printMemoryBlock(D__STACK_SPACE_START, m_g_stack_lines, "STACK MEMORY");
			else if (m_gui_block_top == eGuiBlock::Heap)
				printMemoryBlock(D__HEAP_SPACE_START, m_g_heap_lines, "HEAP MEMORY");
			else if (m_gui_block_top == eGuiBlock::Libraries)
				printMemoryBlock(D__MEMORY_START, m_g_lib_lines, "LIBRARIES MEMORY");
			else if (m_gui_block_top == eGuiBlock::Registers)
				printRegisters();
			else if (m_gui_block_top == eGuiBlock::Output)
				printScriptOutput(m_g_out_lines);
			else if (m_gui_block_top == eGuiBlock::Source)
				printSourceView(m_g_source_lines);
			else if (m_gui_block_top == eGuiBlock::Log)
			{
				if (!__done && !m_error)
				{
					if ((m_msg_log.size() > 0 && m_msg_log[m_msg_log.size() - 1].second != "Press <Enter> for next instruction.") || m_msg_log.size() == 0)
						message("Press <Enter> for next instruction.", eMsgType::Info, true, false, true);
				}
				printLog();
			}

			if (m_show_data && m_gui_data_under == eGuiBlockPosition::Top)
				printInfoView();

			
			if (m_gui_error_on_block == eGuiBlockPosition::Middle && m_error)
				printError();
			else if (m_gui_block_middle == eGuiBlock::CallTree && m_use_sym_table)
				m_sym_table.m_callTree.print(out, __line_length, m_g_tree_lines);
			else if (m_gui_block_middle == eGuiBlock::Code)
				printMemoryBlock(D__MEMORY_START, m_g_code_lines, "CODE MEMORY");
			else if (m_gui_block_middle == eGuiBlock::Stack)
				printMemoryBlock(D__STACK_SPACE_START, m_g_stack_lines, "STACK MEMORY");
			else if (m_gui_block_middle == eGuiBlock::Heap)
				printMemoryBlock(D__HEAP_SPACE_START, m_g_heap_lines, "HEAP MEMORY");
			else if (m_gui_block_middle == eGuiBlock::Libraries)
				printMemoryBlock(D__MEMORY_START, m_g_lib_lines, "LIBRARIES MEMORY");
			else if (m_gui_block_middle == eGuiBlock::Registers)
				printRegisters();
			else if (m_gui_block_middle == eGuiBlock::Output)
				printScriptOutput(m_g_out_lines);
			else if (m_gui_block_middle == eGuiBlock::Source)
				printSourceView(m_g_source_lines);
			else if (m_gui_block_middle == eGuiBlock::Log)
			{
				if (!__done && !m_error)
				{
					if ((m_msg_log.size() > 0 && m_msg_log[m_msg_log.size() - 1].second != "Press <Enter> for next instruction.") || m_msg_log.size() == 0)
						message("Press <Enter> for next instruction.", eMsgType::Info, true, false, true);
				}
				printLog();
			}

			if (m_show_data && m_gui_data_under == eGuiBlockPosition::Middle)
				printInfoView();
			
			if (m_gui_error_on_block == eGuiBlockPosition::Bottom && m_error)
				printError();
			else if (m_gui_block_bottom == eGuiBlock::CallTree && m_use_sym_table)
				m_sym_table.m_callTree.print(out, __line_length, m_g_tree_lines);
			else if (m_gui_block_bottom == eGuiBlock::Code)
				printMemoryBlock(D__MEMORY_START, m_g_code_lines, "CODE MEMORY");
			else if (m_gui_block_bottom == eGuiBlock::Stack)
				printMemoryBlock(D__STACK_SPACE_START, m_g_stack_lines, "STACK MEMORY");
			else if (m_gui_block_bottom == eGuiBlock::Heap)
				printMemoryBlock(D__HEAP_SPACE_START, m_g_heap_lines, "HEAP MEMORY");
			else if (m_gui_block_bottom == eGuiBlock::Libraries)
				printMemoryBlock(D__MEMORY_START, m_g_lib_lines, "LIBRARIES MEMORY");
			else if (m_gui_block_bottom == eGuiBlock::Registers)
				printRegisters();
			else if (m_gui_block_bottom == eGuiBlock::Output)
				printScriptOutput(m_g_out_lines);
			else if (m_gui_block_bottom == eGuiBlock::Source)
				printSourceView(m_g_source_lines);
			else if (m_gui_block_bottom == eGuiBlock::Log)
			{
				if (!__done && !m_error)
				{
					if ((m_msg_log.size() > 0 && m_msg_log[m_msg_log.size() - 1].second != "Press <Enter> for next instruction.") || m_msg_log.size() == 0)
						message("Press <Enter> for next instruction.", eMsgType::Info, true, false, true);
				}
				printLog();
			}

			if (m_show_data && m_gui_data_under == eGuiBlockPosition::Bottom)
				printInfoView();

			if (m_gui_error_on_block == eGuiBlockPosition::Extra && m_error)
				printError();
			else if (m_gui_block_extra == eGuiBlock::CallTree && m_use_sym_table)
				m_sym_table.m_callTree.print(out, __line_length, m_g_tree_lines);
			else if (m_gui_block_extra == eGuiBlock::Code)
				printMemoryBlock(D__MEMORY_START, m_g_code_lines, "CODE MEMORY");
			else if (m_gui_block_extra == eGuiBlock::Stack)
				printMemoryBlock(D__STACK_SPACE_START, m_g_stack_lines, "STACK MEMORY");
			else if (m_gui_block_extra == eGuiBlock::Heap)
				printMemoryBlock(D__HEAP_SPACE_START, m_g_heap_lines, "HEAP MEMORY");
			else if (m_gui_block_extra == eGuiBlock::Libraries)
				printMemoryBlock(D__MEMORY_START, m_g_lib_lines, "LIBRARIES MEMORY");
			else if (m_gui_block_extra == eGuiBlock::Registers)
				printRegisters();
			else if (m_gui_block_extra == eGuiBlock::Output)
				printScriptOutput(m_g_out_lines);
			else if (m_gui_block_extra == eGuiBlock::Source)
				printSourceView(m_g_source_lines);
			else if (m_gui_block_extra == eGuiBlock::Log)
			{
				if (!__done && !m_error)
				{
					if ((m_msg_log.size() > 0 && m_msg_log[m_msg_log.size() - 1].second != "Press <Enter> for next instruction.") || m_msg_log.size() == 0)
						message("Press <Enter> for next instruction.", eMsgType::Info, true, false, true);
				}
				printLog();
			}
			
			if (m_show_data && m_gui_data_under == eGuiBlockPosition::Extra)
				printInfoView();

			printSeparator();
		}
		
		void Debugger::printLog(void)
		{
			OutputManager& out = *getOutputHandler();
			printTitle("MESSAGE LOG", __line_length);
			out.tc_reset().bc_grey();
			uint16 __msg_len = 0;
			if (m_msg_log.size() >= m_g_log_lines)
			{
				for (uint16 i = m_msg_log.size() - m_g_log_lines; i < m_msg_log.size(); i++)
				{
					out.fc_blue().bc_white();
					out.print("│");
					__msg_len = message(m_msg_log[i].second, m_msg_log[i].first, false, true, false);
					__msg_len++;
					out.print(Utils::duplicateChar(' ', __line_length - __msg_len - 1));
					out.fc_blue().bc_white();
					out.print("│").newLine();
				}
			}
			else
			{
				for (uint16 i = 0; i < m_msg_log.size(); i++)
				{
					out.fc_blue().bc_white();
					out.print("│");
					__msg_len = message(m_msg_log[i].second, m_msg_log[i].first, false, true, false);
					__msg_len++;
					out.print(Utils::duplicateChar(' ', __line_length - __msg_len - 1));
					out.fc_blue().bc_white();
					out.print("│").newLine();
				}
				for (uint16 i = 0; i < m_g_log_lines - m_msg_log.size(); i++)
				{
					out.fc_blue().bc_white();
					out.print("│");
					out.print(Utils::duplicateChar(' ', __line_length - 2));
					out.fc_blue().bc_white();
					out.print("│").newLine();
				}
			}
			out.tc_reset();
		}

		uint16 Debugger::message(OmniaString __msg_text, eMsgType __type, bool __log, bool __force_print, bool __new_iine)
		{
			if (!cmd__step || __force_print)
			{
				getOutputHandler()->tc_reset();
				if (__type == eMsgType::Info) getOutputHandler()->fc_magenta();
				else if (__type == eMsgType::Success) getOutputHandler()->fc_green();
				else if (__type == eMsgType::Error) getOutputHandler()->fc_red();
				else if (__type == eMsgType::Special) getOutputHandler()->fc_cyan();
				else if (__type == eMsgType::Warning) getOutputHandler()->fc_yellow();
				else if (__type == eMsgType::Version) getOutputHandler()->fc_brightYellow();
				getOutputHandler()->print("*** ").print(__msg_text);
				if (__new_iine) getOutputHandler()->newLine();
				getOutputHandler()->tc_reset();
			}
			if (__log) m_msg_log.push_back({__type, __msg_text});
			return __msg_text.length() + 4;//4 for the prefix
		}

		void Debugger::inputPrompt(void)
		{
			exec_tick = true;
			OutputManager& out = *getOutputHandler();
			if (!__skip_next_input_round)
			{
				out.tc_reset();
				out.fc_magenta();
				out.fc_cyan();
				out.print(m_prompt);
				out.tc_reset();
				getInputHandler()->read(m_cmd_input);
				m_cmd_input = m_cmd_input.trim().toLowerCase();
				if (m_cmd_input == "") 
				{
					if (!__done && !m_error)
						exec_tick = true;
					return;
				}
				exec_tick = false;
				if (m_cmd_input == "proc-out" || m_cmd_input == "po")
				{
					out.clear();
					printScriptOutput(0);
					printSeparator();
					message("Press <enter> to go back to debug-view.", eMsgType::Info, false, true, true);
					OmniaString __tmp;
					getInputHandler()->read(__tmp);
					exec_tick = false;
					out.clear().tc_reset();
				}
				else if (m_cmd_input == "quit" || m_cmd_input == "q")
				{
					message("Exiting with force...", eMsgType::Info, true, true, true);
					m_force_quit = true;
				}
				else if (m_cmd_input == "resume" || m_cmd_input == "r")
				{
					__br_sig = false;
					exec_tick = true;
					cmd__step = false;
					m_mode = eDebuggerMode::Normal;
				}
				else if (m_cmd_input == "source")
				{
					exec_tick = false;
					if (!m_use_sym_table)
					{
						message("Warning: No Debug table loaded.", eMsgType::Warning, true, false, true);
						return;
					}
					if (m_sym_table.m_source.size() == 0)
					{
						message("Warning: No source code loaded.", eMsgType::Warning, true, false, true);
						return;
					}
					out.clear();
					printSourceCode(0, 0, 0);
					out.tc_reset();
					message("Press <Enter> to go back to prompt.", eMsgType::Info, false, true, true);
					OmniaString __tmp;
					getInputHandler()->read(__tmp);
					out.clear();
				}
				else if (m_cmd_input == "show-call-tree" || m_cmd_input == "sct")
				{
					exec_tick = false;
					out.clear().tc_reset();
					m_sym_table.m_callTree.print(out, __line_length);
					printSeparator();
					message("Press <Enter> to return to step-by-step prompt.", eMsgType::Info, false, true, true);
					OmniaString __tmp = "";
					getInputHandler()->read(__tmp);
					out.clear().tc_reset();
				}
				else if (m_cmd_input == "error-stack" || m_cmd_input == "err")
				{
					exec_tick = false;
					if (!m_error) return;
					out.clear().tc_reset();
					for (uint32 i = 0; i < m_err_data.size(); i++)
						printError(i);
					printSeparator();
					message("Press <Enter> to return to step-by-step prompt.", eMsgType::Info, false, true, true);
					OmniaString __tmp = "";
					getInputHandler()->read(__tmp);
					out.clear().tc_reset();
				}
				else if (m_cmd_input == "log")
				{
					out.clear();
					printTitle("MESSAGES LOG", __line_length);
					for (auto& __log : m_msg_log)
					{
						out.tc_reset().fc_brightBlue();
						out.print("  +  ");
						message(__log.second, __log.first, false, true, true);
					}
					printSeparator();
					message("Press <enter> to go back to debug-view.", eMsgType::Info, false, true, true);
					OmniaString __tmp;
					getInputHandler()->read(__tmp);
					exec_tick = false;
				}
				else if (m_cmd_input.startsWith("break-point ") || m_cmd_input.startsWith("br "))
				{
					exec_tick = false;
					m_cmd_input = m_cmd_input.substr(m_cmd_input.indexOf(" ") + 1).trim();
					if (Utils::isInt(m_cmd_input))
					{
						cpu.addBreakPoint((word)Utils::strToInt(m_cmd_input));
						message(StringBuilder("Break-point added at Code-Address ").add(m_cmd_input).get(), eMsgType::Info, true, false, true);
					}
					else if (m_sym_table.isLabel(m_cmd_input))
					{
						for (auto& __lbl : m_sym_table.m_labels)
						{
							if (__lbl.second == m_cmd_input)
							{
								cpu.addBreakPoint(__lbl.first);
								message(StringBuilder("Break-point added on label <").add(m_cmd_input).add("> at Code-Address ").add(__lbl.first).get(), eMsgType::Info, true, false, true);
								break;
							}
						}
					}
				}
				else if (m_cmd_input.startsWith("gui "))
				{
					exec_tick = false;
					OmniaString::StringTokens __st = m_cmd_input.tokenize(" ", true);
					__st.next();
					OmniaString __param;
					bool __np_block = false;
					bool __np_val = false;
					eGuiBlockPosition __pos = eGuiBlockPosition::Middle;
					eGuiBlock __block = eGuiBlock::None;
					while (__st.hasNext())
					{
						__param = __st.next();
						if (__np_val)
						{
							__np_val = false;
							if (!Utils::isInt(__param)) continue;
							switch (__block)
							{
								case eGuiBlock::CallTree:
									m_g_tree_lines = Utils::strToInt(__param);
									continue;
								case eGuiBlock::Heap:
									m_g_heap_lines = Utils::strToInt(__param);
									continue;
								case eGuiBlock::Stack:
									m_g_stack_lines = Utils::strToInt(__param);
									continue;
								case eGuiBlock::Libraries:
									m_g_lib_lines = Utils::strToInt(__param);
									continue;
								case eGuiBlock::Source:
									m_g_source_lines = Utils::strToInt(__param);
									continue;
								case eGuiBlock::Code:
									m_g_code_lines = Utils::strToInt(__param);
									continue;
								case eGuiBlock::Output:
									m_g_out_lines = Utils::strToInt(__param);
									continue;
								case eGuiBlock::Log:
									m_g_log_lines = Utils::strToInt(__param);
									continue;
								case eGuiBlock::Registers:
									message("Warning: Registers-block is fixed size, and can't be changed.", eMsgType::Warning, true, false, true);
									continue;
								default: continue;
							};
							continue;
						}
						if (__np_block)
						{
							if (__param == "heap") __block = eGuiBlock::Heap;
							else if (__param == "stack") __block = eGuiBlock::Stack;
							else if (__param == "registers") __block = eGuiBlock::Registers;
							else if (__param == "output") __block = eGuiBlock::Output;
							else if (__param == "source") __block = eGuiBlock::Source;
							else if (__param == "call-tree") __block = eGuiBlock::CallTree;
							else if (__param == "libraries") __block = eGuiBlock::Libraries;
							else if (__param == "code") __block = eGuiBlock::Code;
							else if (__param == "log") __block = eGuiBlock::Log;
							else if (__param == "none") __block = eGuiBlock::None;
							else
							{
								message("Error: Invalid gui-block name.", eMsgType::Error, true, false, true);
								break;
							}
							
							if (__pos == eGuiBlockPosition::Top)
							{
								m_gui_block_top = __block;
								message(StringBuilder("Top panel set to <").add(__param).add(">.").get(), eMsgType::Success, true, false, true);
							}
							else if (__pos == eGuiBlockPosition::Middle)
							{
								m_gui_block_middle = __block;
								message(StringBuilder("Middle panel set to <").add(__param).add(">.").get(), eMsgType::Success, true, false, true);
							}
							else if (__pos == eGuiBlockPosition::Bottom)
							{
								m_gui_block_bottom = __block;
								message(StringBuilder("Bottom panel set to <").add(__param).add(">.").get(), eMsgType::Success, true, false, true);
							}
							else if (__pos == eGuiBlockPosition::Extra)
							{
								m_gui_block_extra = __block;
								message(StringBuilder("Extra panel set to <").add(__param).add(">.").get(), eMsgType::Success, true, false, true);
							}
							__np_val = true;
							__np_block = false;
							continue;
						}

						if (__param.equals("--top") || __param.equals("-t"))
						{
							__np_block = true;
							__pos = eGuiBlockPosition::Top;
						}
						else if (__param.equals("--middle") || __param.equals("-m"))
						{
							__np_block = true;
							__pos = eGuiBlockPosition::Middle;
						}
						else if (__param.equals("--bottom") || __param.equals("-b"))
						{
							__np_block = true;
							__pos = eGuiBlockPosition::Bottom;
						}
						else if (__param.equals("--extra") || __param.equals("-e"))
						{
							__np_block = true;
							__pos = eGuiBlockPosition::Extra;
						}
						else if (__param.equals("--show-data") || __param.equals("-sd"))
						{
							if (!__st.hasNext())
							{
								message("Error: missing parameter in --show-data option (either true or false).", eMsgType::Error, true, false, true);
								break;
							}
							__param = __st.next();
							if (__param == "true" || __param == "1")
							{
								m_show_data = true;
								message("Extra information is now visible.", eMsgType::Success, true, false, true);
								continue;
							}
							else if (__param == "false" || __param == "0")
							{
								m_show_data = false;
								message("Extra information is now hidden.", eMsgType::Success, true, false, true);
								continue;
							}
							else
							{
								message("Error: Invalid value for --show-data option.", eMsgType::Error, true, false, true);
								break;
							}
						}
						else if (__param.equals("--show-errors-on"))
						{
							if (!__st.hasNext())
							{
								message("Error: missing parameter in --show-errors-on option (top, middle or bottom).", eMsgType::Error, true, false, true);
								break;
							}
							__param = __st.next();
							if (__param == "top")
							{
								m_gui_error_on_block = eGuiBlockPosition::Top;
								message("Errors will be show on the top panel.", eMsgType::Success, true, false, true);
								continue;
							}
							else if (__param == "middle")
							{
								m_gui_error_on_block = eGuiBlockPosition::Middle;
								message("Errors will be show on the middle panel.", eMsgType::Success, true, false, true);
								continue;
							}
							else if (__param == "bottom")
							{
								m_gui_error_on_block = eGuiBlockPosition::Bottom;
								message("Errors will be show on the bottom panel.", eMsgType::Success, true, false, true);
								continue;
							}
							else if (__param == "extra")
							{
								m_gui_error_on_block = eGuiBlockPosition::Extra;
								message("Errors will be show on the extra panel.", eMsgType::Success, true, false, true);
								continue;
							}
							else
							{
								message("Error: Invalid value for --show-errors-on option.", eMsgType::Error, true, false, true);
								break;
							}
						}
						else if (__param.equals("--show-data-under"))
						{
							if (!__st.hasNext())
							{
								message("Error: missing parameter in --show-data-under option (top, middle or bottom).", eMsgType::Error, true, false, true);
								break;
							}
							__param = __st.next();
							if (__param == "top")
							{
								m_gui_data_under = eGuiBlockPosition::Top;
								message("Extra information will be shown under the top panel.", eMsgType::Success, true, false, true);
								continue;
							}
							else if (__param == "middle")
							{
								m_gui_data_under = eGuiBlockPosition::Middle;
								message("Extra information will be shown under the middle panel.", eMsgType::Success, true, false, true);
								continue;
							}
							else if (__param == "bottom")
							{
								m_gui_data_under = eGuiBlockPosition::Bottom;
								message("Extra information will be shown under the bottom panel.", eMsgType::Success, true, false, true);
								continue;
							}
							else if (__param == "extra")
							{
								m_gui_data_under = eGuiBlockPosition::Extra;
								message("Extra information will be shown under the extra panel.", eMsgType::Success, true, false, true);
								continue;
							}
							else
							{
								message("Error: Invalid value for --show-data-under option.", eMsgType::Error, true, false, true);
								break;
							}
						}
					}
				}
				else if (m_cmd_input == "stop")
				{
					out.clear().tc_reset();
					reset();
					message("Stopped process.", eMsgType::Special, true, true, true);
					topLevelPrompt();
					out.clear().tc_reset();
					exec_tick = false;
					m_skip_next_draw = true;
				}
			}
		}

		void Debugger::printInfoView(void)
		{
			OutputManager& out = *getOutputHandler();
			out.fc_blue().bc_white();
			out.print("│");
			out.fc_blue().bc_magenta();
			OmniaString __dec_inst_text(" Decoded instruction: ");
			out.print(__dec_inst_text);
			out.fc_brightYellow().bc_blue();
			m_cursor = (word)__dec_inst_text.length() + 1;
			if (__dec_inst.size() > 0)
			{
				StringBuilder __sb = StringBuilder();
				for (auto& __cell : __dec_inst)
					__sb.add(Utils::intToHexStr(__cell.val())).add(", ");
				OmniaString __di = __sb.get();
				__di = __di.trim().substr(0, __di.length() - 2);
				word __space = ((__line_length / 2) - __dec_inst_text.length() - __di.length());
				out.print(" ").print(__di).print(Utils::duplicateChar(' ',  __space));
				m_cursor += __di.length() + 1 + __space;
			}
			else
				out.print(Utils::duplicateChar(' ', __line_length - __dec_inst_text.length())).newLine();

			out.fc_blue().bc_magenta();
			__dec_inst_text = " Current sub-routine: ";
			m_cursor += __dec_inst_text.length();
			out.print(__dec_inst_text);
			out.fc_brightYellow().bc_blue();
			
			if (m_sym_table.m_callTree.getCurrentCall(__dec_inst_text))
			{
				out.print(" ").print(__dec_inst_text);
				m_cursor += __dec_inst_text.length() + 1;
				out.print(Utils::duplicateChar(' ',  __line_length - m_cursor - 1));
			}
			else
			{
				out.print(Utils::duplicateChar(' ',  __line_length - m_cursor - 1));
			}
			out.fc_blue().bc_white();
			out.print("│").newLine();
			m_cursor = 0;
			out.tc_reset();
		}

		void Debugger::printSourceView(uint8 __lines)
		{
			if (m_use_sym_table)
			{
				if (m_sym_table.m_source.size() > 0 && __ip != oasm_nullptr)
				{
					m_currentSourceLine = findCurrentLine(__off_ip);
					if (m_currentSourceLine <= (uint32)((__lines / 2) + 1))
						printSourceCode(0, __lines, __off_ip);
					//	else if (m_currentSourceLine >= (uint32)(m_sym_table.m_source.size() - ( (__lines / 2) + 1)))
					//	printSourceCode(m_sym_table.m_source.size() - __lines, __lines, __off_ip);
					else
						printSourceCode(m_currentSourceLine -  ((__lines / 2) + 1), __lines, __off_ip);
				}
			}
			else
				printSeparator();
		}

		void Debugger::fillRestOfLine(char __c, int8 __neg_offset, bool __new_line)
		{
			if (__neg_offset > 0) __neg_offset = 0;
			getOutputHandler()->print(Utils::duplicateChar(__c, (__line_length - m_cursor) + __neg_offset));
			if (__new_line) getOutputHandler()->newLine();
			m_cursor = 0;
		}

		void Debugger::pushError(ErrorCode __err_code, OutputManager& out, OmniaString __extra_text, MemAddress __addr, word __op_code)
		{
			tErrorData __error_data;
			__error_data.code = __err_code;
			__error_data.extraText = __extra_text;
			__error_data.address = __addr;
			__error_data.instruction = __op_code;
			
			m_err_data.push_back(__error_data);
			m_error = true;
		}

		void Debugger::printError(uint32 __err_index)
		{
			if (!m_error || m_err_data.size() == 0 || __err_index >= m_err_data.size()) return;
			OutputManager& out = *getOutputHandler();
			printTitle("ERROR", __line_length);
			out.bc_grey().fc_brightRed();
			OmniaString __err_text;
			StringBuilder __sb;
			m_cursor = 0;
			tErrorData __err = m_err_data[__err_index];
			__err_text = ErrorReciever::__error_map[__err.code];
			out.fc_blue().bc_white();
			out.print("│");
			out.bc_grey().fc_brightRed();
			out.print("Error ").print(Utils::intToHexStr(__err.code));
			__sb.add("|Error ").add(Utils::intToHexStr(__err.code));
			m_cursor += __sb.get().length();
			__sb = StringBuilder();
			fillRestOfLine(' ', -1, false);
			out.fc_blue().bc_white();
			out.print("│");
			out.newLine();
			out.print("│");
			out.bc_grey().fc_brightRed();
			out.print(__err_text);
			m_cursor += __err_text.length() + 1;
			fillRestOfLine(' ', -1, false);
			out.fc_blue().bc_white();
			out.print("│");
			out.newLine();
			out.print("│");
			out.bc_grey().fc_brightRed();
			m_cursor++;
			fillRestOfLine(' ', -1, false);
			out.fc_blue().bc_white();
			out.print("│");
			out.newLine().tc_reset();
			if (__err.extraText.trim() != "")
			{
				out.fc_blue().bc_white();
				out.print("│");
				out.bc_grey().fc_brightRed();
				out.print("Extra info: ").print(__err.extraText);
				__sb.add("|Extra Info: ").add(__err.extraText);
				m_cursor += __sb.get().length();
				__sb = StringBuilder();
				fillRestOfLine(' ', -1, false);
				out.fc_blue().bc_white();
				out.print("│");
				out.newLine().tc_reset();
			}
			if (__err.address != oasm_nullptr)
			{
				out.fc_blue().bc_white();
				out.print("│");
				out.bc_grey().fc_brightRed();
				out.print("Address: ").print(Utils::intToHexStr(__err.address));
				__sb.add("|Address: ").add(Utils::intToHexStr(__err.address));
				if (__err.address < D__HEAP_SPACE_START)
				{
					out.print(" (-CODE-)");
					__sb.add(" (-CODE-)");
				}
				else if (__err.address < D__STACK_SPACE_START)
				{
					out.print(" (-HEAP-)");
					__sb.add(" (-HEAP-)");
				}
				else if (__err.address < D__LIB_SPACE_START)
				{
					out.print(" (-STACK-)");
					__sb.add(" (-STACK-)");
				}
				else if (__err.address < D__MEMORY_SIZE)
				{
					out.print(" (-LIBRARY-)");
					__sb.add(" (-LIBRARY-)");
				}
				m_cursor += __sb.get().length();
				__sb = StringBuilder();
				fillRestOfLine(' ', -1, false);
				out.fc_blue().bc_white();
				out.print("│");
				out.newLine().tc_reset();
			}
			if (__err.instruction != (word)eInstructionSet::no_op)
			{
				out.fc_blue().bc_white();
				out.print("│");
				out.bc_grey().fc_brightRed();
				out.print("Instruction: ").print(Utils::intToHexStr(__err.instruction)).print(" (-").print(Utils::mapInstruction((eInstructionSet)__err.instruction)).print("-)");
				__sb.add("|Instruction: ").add(Utils::intToHexStr(__err.instruction)).add(" (-").add(Utils::mapInstruction((eInstructionSet)__err.instruction)).add("-)");
				m_cursor += __sb.get().length();
				__sb = StringBuilder();
				fillRestOfLine(' ', -1, false);
				out.fc_blue().bc_white();
				out.print("│");
				out.newLine().tc_reset();
			}
		}

		void Debugger::printScriptOutput(uint16 __line_count)
		{
			if (__line_count == 0) __line_count = __proc_out_buffer.size();
			OutputManager& out = *getOutputHandler();
			printTitle("SCRIPT OUTPUT", __line_length);
			out.tc_reset();
			out.fc_brightWhite().bc_grey();
			if (__proc_out_buffer.size() <= __line_count)
			{
				uint32 __i = 0;
				for (auto& line : __proc_out_buffer)
				{
					out.fc_blue().bc_white();
					out.print("│");
					out.fc_brightWhite().bc_grey();
					out.print("    ").print(line).print(Utils::duplicateChar(' ', __line_length - (line.length() + 6)));
					out.fc_blue().bc_white();
					out.print("|").newLine();
					__i++;
				}
				for ( ; __i < __line_count; __i++)
				{
					out.fc_blue().bc_white();
					out.print("│");
					out.fc_brightWhite().bc_grey();
					out.print(Utils::duplicateChar(' ', __line_length - 2));
					out.fc_blue().bc_white();
					out.print("│");
					out.newLine();
				}
			}
			else
			{
				OmniaString __l = "";
				for (uint32 __i = __line_count; __i > 0; __i--)
				{
					__l = __proc_out_buffer[__proc_out_buffer.size() - __i];
					out.fc_blue().bc_white();
					out.print("│");
					out.fc_brightWhite().bc_grey();
					out.print("    ").print(__l).print(Utils::duplicateChar(' ', __line_length - (__l.length() + 6)));
					out.fc_blue().bc_white();
					out.print("|").newLine();
				}
			}
		}

		void Debugger::printMemoryBlock(MemAddress start, uint8 rows, OmniaString title)
		{
			OutputManager& out = *getOutputHandler();
			hw::RAM& ram = VirtualMachine::instance().getRAM();
			ram.disableProtectedMode();
			printTitle(title, __line_length);
			uint8 __sq = 10;
			word __c = 0;
			uint8 __cur_curs = 0;
			word __rll = 0;
			OmniaString __addr_txt;
			BitEditor __r_val = 0;
			out.fc_blue().bc_white();
			out.print("│");
			out.print("     ");
			__cur_curs += 6;
			uint8 __cells_per_row = 12;
			MemAddress size = rows * __cells_per_row; //TODO: check if out of bounds

			for (MemAddress __addr = start; __addr < start + size; __addr++)
			{
				m_memChangeTable[__addr].updadte();
				setColorFromTimeGradient(out, m_memChangeTable[__addr].changeTime);

				__addr_txt = Utils::intToHexStr(__addr, false);
				__r_val = ram.getAsReadOnly()[__addr];

				__rll = __addr_txt.length();
				out.fc_grey();
				out.print(__addr_txt).print(Utils::duplicateChar(' ', ZERO(__sq - __rll - 4 - 1 - 2)));
				out.fc_white();
				out.print("[");
				setColorFromTimeGradient(out, m_memChangeTable[__addr].changeTime);
				out.print(Utils::intToHexStr(__r_val.val(), false));
				out.fc_white();
				out.print("]");
				__cur_curs += __sq;

				out.tc_reset();
				if (++__c % __cells_per_row == 0)
				{
					__c = 0;
					out.tc_reset();
					out.fc_blue().bc_white();
					out.print(Utils::duplicateChar(' ', __line_length - __cur_curs - 1));
					out.print("│");
					__cur_curs = 0;
					out.newLine();
					if (__addr < (start + size) - 1)
					{
						out.fc_blue().bc_white();
						out.print("│");
						out.print("     ");
						__cur_curs += 6;
					}
				}
			}
			out.tc_reset();
			ram.enableProtectedMode();
		}

		void Debugger::printRegisters(void)
		{
			OutputManager& out = *getOutputHandler();
			VirtualMachine::instance().getREG().disableProtectedMode();
			MemAddress start = 0;
			uint8 rows = 4;
			OmniaString title = "REGISTERS";
			printTitle(title, __line_length);
			uint8 __sq = 10;
			word __c = 0;
			uint8 __cur_curs = 0;
			word __rll = 0;
			OmniaString __reg;
			BitEditor __r_val = 0;
			out.fc_blue().bc_white();
			out.print("│");
			out.print("     ");
			__cur_curs += 6;
			uint8 __cells_per_row = 12;
			MemAddress size = rows * __cells_per_row; //TODO: check if out of bounds
			for (MemAddress __addr = start; __addr < start + size; __addr++)
			{
				m_regChangeTable[__addr].updadte();
				setColorFromTimeGradient(out, m_regChangeTable[__addr].changeTime);

				__reg = Utils::mapRegister(__addr);
				__r_val = VirtualMachine::instance().getREG().getAsReadOnly()[__addr];

				__rll = __reg.length();
				out.fc_grey();
				out.print(__reg).print(Utils::duplicateChar(' ', __sq - __rll - 4 - 1));
				setColorFromTimeGradient(out, m_regChangeTable[__addr].changeTime);
				out.print(Utils::intToHexStr(__r_val.val(), false)).print(" ");
				__cur_curs += __sq;

				out.tc_reset();

				if (++__c % __cells_per_row == 0)
				{
					__c = 0;
					out.tc_reset();
					out.fc_blue().bc_white();
					out.print(Utils::duplicateChar(' ', __line_length - __cur_curs - 1));
					out.print("│");
					__cur_curs = 0;
					out.newLine();
					if (__addr < (word)eRegisters::R31)
					{
						out.fc_blue().bc_white();
						out.print("│");
						out.print("     ");
						__cur_curs += 6;
					}
				}
			}
			out.tc_reset();
			VirtualMachine::instance().getREG().enableProtectedMode();
		}
		
		void Debugger::setColorFromTimeGradient(OutputManager& out, uint8 __tc)
		{
			if (__tc == 0) out.bc_red().fc_brightWhite();
			else if (__tc == 1) out.bc_brightRed().fc_brightWhite();
			else if (__tc == 2) out.bc_yellow().fc_brightWhite();
			else if (__tc == 3) out.bc_brightYellow().fc_grey();
			else if (__tc == 4) out.bc_blue().fc_brightWhite();
			else if (__tc >= 5) out.bc_brightBlue().fc_brightWhite();
		}

		void Debugger::printTitle(OmniaString __title, word __line_length)
		{
			__title = StringBuilder("[ ").add(__title).add(" ]").get();
			word __l = (__line_length / 2) - (__title.length() / 2) - 1;
			getOutputHandler()->fc_blue().bc_white();
			getOutputHandler()->print("│");
			getOutputHandler()->print(Utils::duplicateChar('=', __l)).print(__title).print(Utils::duplicateChar('=', __l));
			getOutputHandler()->print("│");
			getOutputHandler()->newLine();
		}

		uint32 Debugger::findCurrentLine(MemAddress __off_ip)
		{
			if (!exec_tick) return m_currentSourceLine;
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

		bool Debugger::printSourceCode(uint32 __start, uint32 __line_count, MemAddress __highlight)
		{
			OutputManager& out = *getOutputHandler();
			OmniaString __tmp_lbl = "", __tmp_line = "";
			printTitle("SOURCE CODE", __line_length);
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
					out.fc_blue().bc_white();
					out.print("│");
					__sb = StringBuilder(":");
					__sb.add(__tmp_lbl).add(":");
					__sb.add(Utils::duplicateChar(' ', __line_length - __sb.get().length() - 2));

					out.fc_brightWhite().bc_brightGrey();
					out.print(__sb.get());
					out.fc_blue().bc_white();
					out.print("│").newLine();
					out.tc_reset();
				}
				if (__line_count != 0 && __ln > __old_ln + __line_count) break;
				__tmp_line = __line.second;
				if (__highlight != 0xFFFF && __line.first == __highlight)
				{
					printInstructionLine(__line, true, false);
				}
				else if (VirtualMachine::instance().getCPU().isBreakPoint(__line.first, __line.second.tokenize(",").count()))
				{
					printInstructionLine(__line, false, true);
				}
				else
				{
					printInstructionLine(__line, false, false);
				}
				__ln++;
				out.tc_reset();
			}
			out.tc_reset();
			return true;
		}

		bool Debugger::printInstructionLine(std::pair<MemAddress, OmniaString> __line, bool __hl, bool __rhl)
		{
			word __inst_column_width = 10;
			OutputManager& out = *getOutputHandler();
			word __c_line_len = 0;
			word __tmp_space = 0;

			out.fc_blue().bc_white();
			out.print("│");
			StringBuilder __sb("   ");
			__sb.add(Utils::intToHexStr(__line.first)).add("   ");
			__c_line_len += __sb.get().length();
			if (__hl) out.fc_red().bc_brightGreen();
			else if (__rhl) out.fc_white().bc_red();
			else out.fc_white().bc_grey();
			out.print(__sb.get());
			OmniaString::StringTokens __st = __line.second.tokenize(",", true);

			OmniaString __tmp = __st.next();
			if (__hl) out.fc_yellow().bc_green();
			else out.fc_yellow().bc_grey();
			out.print(__tmp);
			if (__hl) out.fc_brightWhite().bc_green();
			else out.fc_brightMagenta().bc_grey();
			__tmp_space = __inst_column_width - __tmp.length();
			out.print(Utils::duplicateChar(' ', __tmp_space));
			__c_line_len += __tmp_space;
			__c_line_len += __tmp.length();
			
			if (__hl) out.fc_brightWhite().bc_green();
			else out.fc_brightMagenta().bc_grey();

			while (__st.hasNext())
			{
				__tmp = __st.next();
				if (Utils::isInt(__tmp))
					out.fc_blue().bc_grey();
				else if (m_sym_table.isLabel(__tmp))
					out.fc_brightGreen().bc_grey();
				else if (m_sym_table.isReserve(__tmp))
					out.fc_grey().bc_white();
				else if (__hl)
					out.fc_brightWhite().bc_green();
				else
					out.fc_brightRed().bc_grey();
				
				out.print(__tmp);			
				if (__hl) out.fc_grey().bc_green();
				else out.fc_brightWhite().bc_grey();
				if (__st.hasNext())
				{
					out.print(",  ");
					__c_line_len += __tmp.length() + 3;
				}
				else
					__c_line_len += __tmp.length();
			}

			out.print(Utils::duplicateChar(' ', __line_length - __c_line_len - 2));
			out.fc_blue().bc_white();
			out.print("│").newLine();
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
			if (!lines[0].startsWith("#version(") || !lines[0].endsWith(")"))
				message("Warning: no version information found inside debug file.", eMsgType::Warning, true, true, true);
			else
			{
				OmniaString __ver_str = lines[0].substr(9, lines[0].length() - 1).trim();
				OmniaString __cur_ver = Utils::getVerionString();
				if (__ver_str != __cur_ver)
				{
					if (m_allow_diff_version)
						message(StringBuilder("Warning: script version is different from the running version: (running=")
											.add(__cur_ver).add(", script=").add(__ver_str).add(")").get(), eMsgType::Warning, true, true, true);
					else
					{
						message(StringBuilder("Error: script version is different from the running version: (running=")
											.add(__cur_ver).add(", script=").add(__ver_str).add(")").get(), eMsgType::Error, true, true, true);
						message("Note: run the debugger with the --allow-diff-ver option if you wish to force the script to run anyways.", eMsgType::Special, true, true, true);
						return false;
					}
				}
			}
			for (auto& __line : lines)
			{
				__line = __line.trim();
				if (__line.startsWith("#")) continue;
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
			message(StringBuilder("Debug table loaded successfully: ").add((word)__sym_count).add(" Symbols found.").get(), eMsgType::Info, true, true, true);
			if (m_sym_table.m_source.size() > 0)
				message(StringBuilder("Source loaded: ").add((word)__src_lines).add(" lines.").get(), eMsgType::Info, true, true, true);
			return true;
		}

		void Debugger::printSeparator(unsigned char c)
		{
			OutputManager& out = *getOutputHandler();
			out.fc_blue().bc_white();
			out.print(Utils::duplicateChar(c, __line_length)).newLine();
			out.tc_reset();
		}
	}
}