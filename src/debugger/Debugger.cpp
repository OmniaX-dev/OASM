#include "Debugger.hpp"
#include "Interpreter.hpp"
#include <iostream>
#include <fstream>

namespace Omnia
{
	namespace oasm
	{
		int64 Debugger::run(int argc, char** argv)
		{
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
				}
			}
            if (p__input_file_path.trim() == "")
            {
                out.print("Error: No input file specified.").newLine();
                return 0xFFFE; //TODO: Add error code
            }
            OmniaString __param3 = "--input-file ";
            __param3 = __param3.add(p__input_file_path);
            cpu.setIPC(1);
            
			ErrorCode __err_code;
			out.newLine().tab().print("#[-cmd-]/> ");
			in.read(m_cmd_input);
            m_cmd_input = m_cmd_input.trim().toLowerCase();
			bool cmd__step = false;
			if (m_cmd_input == "run")
			{
				out.tab().print("Running process...").newLine();
				__err_code = Interpreter::instance().run(4, new char*[]{argv[0], (char*)"--debugger-internal-call", (char*)"--input-file", (char*)p__input_file_path.c_str()});
				if (__err_code != D__NO_ERROR)
				{
					//TODO: Error
					return __err_code;
				}
			}
			else if (m_cmd_input == "step")
			{
				cmd__step = true;
				out.tab().print("Running process (step-by-step)...").newLine();
				__err_code = Interpreter::instance().run(4, new char*[]{argv[0], (char*)"--debugger-internal-call", (char*)"--input-file", (char*)p__input_file_path.c_str()});
				if (__err_code != D__NO_ERROR)
				{
					//TODO: Error
					return __err_code;
				}
			}
			else
			{
                out.print("Error: unknown command.").newLine();
                return 0xABBC; //TODO: Add error code
			}
            Process& proc = vm.getCurrentProcess();
			bool tick_res = false, exec_tick = true;
			std::vector<OmniaString> proc_out;
            while (true)
            {
				exec_tick = true;
				if (cmd__step)
				{
                	out.newLine().tab().print("#[-cmd-]/> ");
            		in.read(m_cmd_input);
                	m_cmd_input = m_cmd_input.trim().toLowerCase();
					if (m_cmd_input == "print-mem")
					{
						exec_tick = false;
						vm.getCPU().printMemory(out, 4, 4, 16, true);
					}
					else if (m_cmd_input == "quit")
					{
						out.tab().print("Exiting with force...");
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
							out.newLine().tab().print("[-*-]/> ").print(line);
					}
				}
				if (proc.done()) break;
            }
			ErrorCode __err = vm.getCPU().getLastErrorCode();
			//if (__err == D__NO_ERROR)
				//vm.getCPU().printMemory(out, 4, 4, 16, false);
			return __err_code;
		}
	}
}