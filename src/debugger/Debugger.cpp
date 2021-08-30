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
			m_prompt = "!$:> ";
            OutputManager& out = *getOutputHandler();
            InputManager& in = *getInputHandler();
			VirtualMachine &vm = VirtualMachine::instance();
			hw::RAM &ram = vm.getRAM();
			hw::REG &reg = vm.getREG();
			hw::CPU &cpu = vm.getCPU();
			hw::GPU &gpu = vm.getGPU();
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
							out.print("Error: No input file specified.").newLine();
							return 0xFFFF; //TODO: Add error code
						}
						i++;
						p__input_file_path = OmniaString(argv[i]);
					}
					else if (OmniaString(argv[i]).trim().equals("--debug-table") || OmniaString(argv[i]).trim().equals("-dt"))
					{
						if (i + 1 >= argc)
						{
							out.print("Error: No input debug table file specified.").newLine();
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
                out.print("Error: No input file specified.").newLine();
                return 0xFFFE; //TODO: Add error code
            }
			if (!m_use_sym_table || p__input_sym_table_path.trim() == "")
			{
				OmniaString __tmp_input_file_name = p__input_file_path.substr(0, p__input_file_path.lastIndexOf("."));
				__tmp_input_file_name = __tmp_input_file_name.add(".odb");
				if (std::filesystem::exists(__tmp_input_file_name.cpp()))
				{
					out.tab().print("Symbol table file found: ").print(__tmp_input_file_name).newLine();
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
				out.newLine().tab().print(m_prompt);
				in.read(m_cmd_input);
				m_cmd_input = m_cmd_input.trim().toLowerCase();
				if (m_cmd_input == "run")
				{
					out.tab().tab().print("Running process...").newLine();
					__err_code = Interpreter::instance().run(4, new char*[]{argv[0], (char*)"--debugger-internal-call", (char*)"--input-file", (char*)p__input_file_path.c_str()});
					if (__err_code != D__NO_ERROR)
					{
						//TODO: Error
						return __err_code;
					}
					break;
				}
				else if (m_cmd_input == "step")
				{
					cmd__step = true;
					out.tab().tab().print("Running process (step-by-step)...").newLine();
					__err_code = Interpreter::instance().run(4, new char*[]{argv[0], (char*)"--debugger-internal-call", (char*)"--input-file", (char*)p__input_file_path.c_str()});
					if (__err_code != D__NO_ERROR)
					{
						//TODO: Error
						return __err_code;
					}
					break;
				}
				else
				{
					out.tab().tab().print("Error: unknown command.").newLine();
				}
			}
            Process& proc = vm.getCurrentProcess();
			bool tick_res = false, exec_tick = true;
			std::vector<OmniaString> proc_out;
			MemAddress __off_ip = oasm_nullptr;
            while (true)
            {
				exec_tick = true;
				if (cmd__step)
				{
					if (m_use_sym_table)
					{
						__off_ip = cpu.getLastInstructionAddr();
						if (m_sym_table.m_source.size() > 0 && __off_ip != oasm_nullptr)
						{
							out.newLine().tab().tab().print(m_sym_table.m_source[__off_ip - proc.m_codeAddr]).newLine();
						}
					}
					out.newLine().tab().tab().print("Press <Enter> for next instruction.");
                	out.newLine().tab().tab().print(m_prompt);
            		in.read(m_cmd_input);
                	m_cmd_input = m_cmd_input.trim().toLowerCase();
					if (m_cmd_input == "print-mem")
					{
						exec_tick = false;
						vm.getCPU().printMemory(out, 4, 4, 16, true);
					}
					else if (m_cmd_input == "quit")
					{
						out.tab().tab().tab().print("Exiting with force...");
						break;
					}
					else if (m_cmd_input == "resume")
					{
						exec_tick = true;
						cmd__step = false;
					}
				}
				if (!exec_tick) continue;
				tick_res = cpu.clock_tick();
				proc_out = m_vm_buff.flush();
				if (proc_out.size() > 0)
				{
					for (auto& line : proc_out)
					{
						line = line.replaceAll("\n", "");
						if (line.trim() != "")
							out.newLine().tab().tab().tab().print(m_prompt).print(line);
					}
				}
				if (proc.done()) break;
            }
			ErrorCode __err = vm.getCPU().getLastErrorCode();
			return __err_code;
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
				__line = __line.trim().toLowerCase();
				if (!__in_source && !__line.contains("=")) continue;
				if (__line.startsWith(".label"))
				{
					__line = __line.substr(6).trim();
					__symbol = __line.substr(0, __line.indexOf("=")).trim();
					__addr = (MemAddress)Utils::strToInt(__line.substr(__line.indexOf("=") + 1).trim());
					m_sym_table.m_labels[__addr] = __symbol;
					__sym_count++;
				}
				else if (!__in_source && __line.startsWith(".data"))
				{
					__line = __line.substr(5).trim();
					__symbol = __line.substr(0, __line.indexOf("=")).trim();
					__addr = (MemAddress)Utils::strToInt(__line.substr(__line.indexOf("=") + 1).trim());
					m_sym_table.m_reserves[__addr] = __symbol;
					__sym_count++;
				}
				else if (!__in_source && __line.startsWith(".source = {"))
				{
					__in_source = true;
				}
				else if (__in_source && __line.startsWith("}"))
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
			getOutputHandler()->tab().print("Debug table loaded successfully: ").print((word)__sym_count).print(" Symbols found.").newLine();
			if (m_sym_table.m_source.size() > 0)
				getOutputHandler()->tab().tab().print("*Source loaded: ").print((word)__src_lines).print(" lines.").newLine();
			return true;
		}
	}
}