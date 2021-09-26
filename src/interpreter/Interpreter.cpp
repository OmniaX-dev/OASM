#include "Interpreter.hpp"
#include <iostream>
#include "Assembler.hpp"

namespace Omnia
{
	namespace oasm
	{
		//-----------------------------------------------CMD HANDLERS-----------------------------------------------
		bool EC_PrintChar_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			iomgr.getOutputHandler()->print(OmniaString() + (char)param.val());
			return true;
		}
		bool EC_PrintString_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			OmniaString __str = "";
			if (!VirtualMachine::instance().getRAM().readStringFromStream(param.val(), __str))
				return false;
			iomgr.getOutputHandler()->print(__str);
			return true;
		}
		bool EC_PrintNewLine_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			iomgr.getOutputHandler()->newLine();
			return true;
		}
		bool EC_PrintInt_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			iomgr.getOutputHandler()->print(param.val());
			return true;
		}
		bool EC_Sleep_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			BitEditor __unit;
			if (!VirtualMachine::instance().getREG().read(eRegisters::R31, __unit)) return false;
			eTimeUnits __u = (eTimeUnits)__unit.val();
			Utils::sleep(param.val(), __u);
			return true;
		}
		bool EC_GetRunningTime_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			uint64 __rt = Utils::getRunningTime_ms();
			outData = (word)__rt;
			return true;
		}
		bool EC_TimeDiff_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			if (param == 1)
			{
				m_time = Utils::getRunningTime_ms();
			}
			else
			{
				int64 __t = ((signed)m_time - Utils::getRunningTime_ms());
				if (__t < 0)
				outData = (word)__t;
				m_time = Utils::getRunningTime_ms();
			}
			return true;
		}
		bool EC_RefreshScreen_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			if (VirtualMachine::instance().getCPU().getVideoMode() != eVideoModes::AsciiGrid) return true;
			//iomgr.getOutputHandler()->clear();
			VirtualMachine::instance().getGPU().clearScreenBuffer();
			Utils::hideCursor();
			Utils::moveConsoleCursor(0, 0);
			return true;
		}
		bool EC_Draw_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			if (VirtualMachine::instance().getCPU().getVideoMode() != eVideoModes::AsciiGrid) return true;
			VirtualMachine::instance().getGPU().clock_tick();
			return true;
		}
		bool EC_SetVideoMode_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			VirtualMachine::instance().getCPU().setVideoMode((eVideoModes)param.val());
			return true;
		}
		bool EC_PlotChar_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			if (VirtualMachine::instance().getCPU().getVideoMode() != eVideoModes::AsciiGrid) return true;
			BitEditor __x, __y, __c, __bg_c, __fg_c;
			hw::CPU& cpu = VirtualMachine::instance().getCPU();
			hw::GPU& gpu = VirtualMachine::instance().getGPU();
			if (!VirtualMachine::instance().getRAM().read(cpu.offsetHeapAddress(param.val()), __x)) return false;
			if (!VirtualMachine::instance().getRAM().read(cpu.offsetHeapAddress(param.val() + 1), __y)) return false;
			if (!VirtualMachine::instance().getRAM().read(cpu.offsetHeapAddress(param.val() + 2), __c)) return false;
			if (!VirtualMachine::instance().getRAM().read(cpu.offsetHeapAddress(param.val() + 3), __bg_c)) return false;
			if (!VirtualMachine::instance().getRAM().read(cpu.offsetHeapAddress(param.val() + 4), __fg_c)) return false;
			if (__x >= gpu.getScreenW() || __y >= gpu.getScreenH()) return true;
			gpu.plotChar(__x.val(), __y.val(), (char)__c.val(), (eOasmColors)__bg_c.val(), (eOasmColors)__fg_c.val());
			return true;
		}
		bool EC_GetScreenW_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			outData = VirtualMachine::instance().getGPU().getScreenW();
			return true;
		}
		bool EC_GetScreenH_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			outData = VirtualMachine::instance().getGPU().getScreenH();
			return true;
		}
		bool EC_Random_cmd::handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			hw::CPU& cpu = VirtualMachine::instance().getCPU();
			BitEditor __min, __max;
			if (!VirtualMachine::instance().getREG().read(eRegisters::R31, __max)) return false;
			if (!VirtualMachine::instance().getREG().read(eRegisters::R30, __min)) return false;
			word __rnd = (word)RANDOM(__min.val(), __max.val());
			if (!VirtualMachine::instance().getRAM().write(cpu.offsetHeapAddress(param.val()), __rnd)) return false;
			return true;
		}
		//----------------------------------------------------------------------------------------------------------




		int64 Interpreter::run(int argc, char **argv)
		{
			OutputManager &out = *getOutputHandler();

			p__step_exec = false;
			p__print_memory = false;
			p__assemble = false;
			p__debugger_call = false;
			p__input_file_path = "";
			proc = Process();
			if (argc > 1)
			{
				for (int i = 1; i < argc; i++)
				{
					if (OmniaString(argv[i]).trim().equals("--step-execution", true) || OmniaString(argv[i]).trim().equals("-S"))
						p__step_exec = true;
					else if (OmniaString(argv[i]).trim().equals("--print-memory", true))
						p__print_memory = true;
					else if (OmniaString(argv[i]).trim().equals("--debugger-internal-call", true))
						p__debugger_call = true;
					else if (OmniaString(argv[i]).trim().equals("--input-file") || OmniaString(argv[i]).trim().equals("-i"))
					{
						if (i + 1 >= argc)
						{
							out.print("Error: No input file specified.").newLine();
							return 0xFFFF;  //TODO: Add error code
						}
						i++;
						p__input_file_path = OmniaString(argv[i]);
					}
					else if (OmniaString(argv[i]).trim().equals("--assemble") || OmniaString(argv[i]).trim().equals("-a"))
					{
						p__assemble = true;
					}
				}
			}

			ECM::instance().addHandler((word)eComCodes::PrintCharToConsole, __ec_printChar_cmd);
			ECM::instance().addHandler((word)eComCodes::PrintStringToConsole, __ec_prinString_cmd);
			ECM::instance().addHandler((word)eComCodes::PrintNewLineToConsole, __ec_prinSNewLine_cmd);
			ECM::instance().addHandler((word)eComCodes::PrintIntToConsole, __ec_printInt_cmd);
			ECM::instance().addHandler((word)eComCodes::Sleep, __ec_sleep_cmd);
			ECM::instance().addHandler((word)eComCodes::GetRunningTime, __ec_getRunningTime_cmd);
			ECM::instance().addHandler((word)eComCodes::TimeDiff, __ec_timeDiff_cmd);
			ECM::instance().addHandler((word)eComCodes::RefreshScreen, __ec_refreshScreen_cmd);
			ECM::instance().addHandler((word)eComCodes::SetVideoMode, __ec_setVideoMode_cmd);
			ECM::instance().addHandler((word)eComCodes::PlotChar, __ec_plotChar_cmd);
			ECM::instance().addHandler((word)eComCodes::GetScreenW, __ec_getScreenW_cmd);
			ECM::instance().addHandler((word)eComCodes::GetScreenH, __ec_getScreenH_cmd);
			ECM::instance().addHandler((word)eComCodes::Draw, __ec_draw_cmd);
			ECM::instance().addHandler((word)eComCodes::Random, __ec_random_cmd);

			Flags::set(FLG__PRINT_ERROR_ON_PUSH);

			VirtualMachine &vm = VirtualMachine::instance();
			hw::RAM &ram = vm.getRAM();
			vm.getCPU().setStepExecution(p__step_exec);

			proc.setID(2522); //TODO: change

			if (p__assemble && p__input_file_path.trim() != "")
				proc.m_code = Assembler::instance().assemble(p__input_file_path);
			else if (!p__assemble && p__input_file_path.trim() != "")
			{
				if (!loadFromFile(p__input_file_path, proc.m_code))
					return 0xADAD; //TODO: Error
			}
			else
			{
				return 0xEBDB; //TODO: Error
			}
			proc.validate();
			vm.setCurrentProc(proc);
			MemAddress __tmp_code_addr;
			if (!ram.request(proc.m_code.size() + 6, __tmp_code_addr, eMemCellType::Normal, proc)) //TODO: +6 adds offset because of a weird bug
			{
				//TODO: Error
				return 0xAFFA;
			}
			ram.disableProtectedMode();
			proc.m_codeAddr = __tmp_code_addr; //TODO: This override needs to be implemented directly into RAM::request function
			for (MemAddress i = proc.m_codeAddr; i < proc.m_codeAddr + proc.m_code.size(); i++)
			{
				//ram.set(proc, (MemAddress)i, eMemCellType::Normal, eMemState::Allocated, eMemCellFlag::NoFlag);
				ram.write(i, proc.m_code[i - proc.m_codeAddr]);
			}
			ram.enableProtectedMode();
			vm.getREG().disableProtectedMode();
			vm.getREG().write(eRegisters::IP, proc.m_codeAddr);
			vm.getREG().enableProtectedMode();
			if (p__debugger_call) return 0x0000;
			while (vm.getCPU().clock_tick() && !proc.done()) ;
			ErrorCode __err = vm.getCPU().getLastErrorCode();

			if (p__print_memory && __err == D__NO_ERROR)
				vm.getCPU().printMemory(out, 4, 4, 16, false);

			if (vm.getCPU().getVideoMode() == eVideoModes::AsciiGrid)
			{
				out.clear().tc_reset();
				Utils::hideCursor(false);
			}

			return __err;
		}

		bool Interpreter::loadFromFile(OmniaString __oex_file, TMemoryList& outProgram)
        {
            std::ifstream rf(__oex_file.cpp(), std::ios::out | std::ios::binary);
            if(!rf) return false; //TODO: Error
            word cell = 0;
            while(rf.read((char*)&cell, sizeof(cell)))
                outProgram.push_back(BitEditor(cell));
            if (outProgram.size() == 0) return false; //TODO: Error
            return true;
        }




		namespace hw
		{
			CPU &CPU::create(uint8 ipc)
			{
				m_ipc = ipc;

				m_raw_1 = (word)eInstructionSet::no_op;
				m_raw_2 = (word)eAddressingModes::Invalid;
				m_raw_3 = (word)eFlags::no_flag;
				m_raw_4 = oasm_nullptr;
				m_raw_5 = oasm_nullptr;
				m_raw_6 = oasm_nullptr;

				m_dec_opCode = eInstructionSet::no_op;
				m_dec_addrMode = eAddressingModes::Invalid;
				m_dec_flags = eFlags::no_flag;
				m_dec_op1 = BitEditor(oasm_nullptr);
				m_dec_op2 = BitEditor(oasm_nullptr);
				m_dec_extra = BitEditor(0x0010);

				m_err_1 = D__NO_ERROR;
				m_err_2 = D__NO_ERROR;
				m_err_3 = D__NO_ERROR;
				m_err_4 = D__NO_ERROR;
				m_err_5 = D__NO_ERROR;
				m_err_6 = D__NO_ERROR;

				m_old_pc_val = oasm_nullptr;
				m_inst_mode = D__NORMAL_INST_MODE;
				m_single_op_inst = false;
				m_process_end = false;
				m_inst_size = 0;
				m_m_param = (word)(eAddressingModes::b_2ByteMode_mask);
				m_op1_addr_mode = eSingleAddrModes::_2B;
				m_op2_addr_mode = eSingleAddrModes::_2B;
				m_const_op1 = false;
				m_pop_r_flg = false;
				m_offset = 0;
				m_video_mode = eVideoModes::Console;

				m_step_execution = false;
				m_current_ipc = 0;
				m_cmd_command = "";
				m_variable_inst_size = false;
				m_heap_reserve_count = 0;
				m_next_single_heap = 0;
				m_break_points.clear();
				m_break_point_signal = false;
				m_break_point_addr = oasm_nullptr;
				m_decoded_inst.clear();

				return *this;
			}

			bool CPU::fetch(RAM &_ram, REG &_reg)
			{
				if (VirtualMachine::instance().getCurrentProcess().done()) return true;
				m_decoded_inst.clear();
				m_old_pc_val = _reg.IP().val();
				BitEditor __tmp;
				_ram.read(_reg.rw_IP()++.val(), __tmp);
				m_err_1 = _ram.getLastErrorCode();
				if (m_err_1 != D__NO_ERROR)
				{
					pushError(m_err_1);
					return false;
				}
				Flags::unset(FLG__PRINT_ERROR_ON_PUSH);
				m_raw_1 = __tmp.val();
				_ram.read(_reg.rw_IP()++.val(), __tmp);
				m_err_2 = _ram.getLastErrorCode();
				m_raw_2 = __tmp.val();
				_ram.read(_reg.rw_IP()++.val(), __tmp);
				m_err_3 = _ram.getLastErrorCode();
				m_raw_3 = __tmp.val();
				_ram.read(_reg.rw_IP()++.val(), __tmp);
				m_err_4 = _ram.getLastErrorCode();
				m_raw_4 = __tmp.val();
				_ram.read(_reg.rw_IP()++.val(), __tmp);
				m_err_5 = _ram.getLastErrorCode();
				m_raw_5 = __tmp.val();
				_ram.read(_reg.rw_IP()++.val(), __tmp);
				m_err_6 = _ram.getLastErrorCode();
				m_raw_6 = __tmp.val();
				Flags::set(FLG__PRINT_ERROR_ON_PUSH);
				return true;
			}

			bool CPU::decode(RAM &_ram, REG &_reg)
			{
				if (VirtualMachine::instance().getCurrentProcess().done()) return true;
				m_single_op_inst = false;
				m_pop_r_flg = false;
				m_inst_mode = D__NORMAL_INST_MODE;
				m_dec_opCode = (eInstructionSet)m_raw_1;
				m_inst_size = 0;
				m_m_param = (word)eAddressingModes::b_2ByteMode_mask;
				m_op1_addr_mode = eSingleAddrModes::_2B;
				m_op2_addr_mode = eSingleAddrModes::_2B;
				m_const_op1 = false;
				m_dec_op1 = 0;
				m_dec_op2 = 0;
				m_variable_inst_size = false;
				if (m_dec_opCode == eInstructionSet::no_op)
				{
					m_inst_size = 1;
					m_decoded_inst.push_back((word)m_dec_opCode);
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::req)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					m_dec_flags = (eFlags)m_raw_2;
					m_dec_op1 = m_raw_3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_flags);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::end)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = m_raw_3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::mem || m_dec_opCode == eInstructionSet::mem_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::mem_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_dec_opCode = eInstructionSet::mem;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::add || m_dec_opCode == eInstructionSet::add_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::add_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_dec_opCode = eInstructionSet::add;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::sub || m_dec_opCode == eInstructionSet::sub_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::sub_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_dec_opCode = eInstructionSet::sub;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::mul || m_dec_opCode == eInstructionSet::mul_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::mul_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_dec_opCode = eInstructionSet::mul;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::div || m_dec_opCode == eInstructionSet::div_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::div_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_dec_opCode = eInstructionSet::div;
					m_inst_size = 4;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::cmp || m_dec_opCode == eInstructionSet::cmp_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::cmp_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_dec_opCode = eInstructionSet::cmp;
					m_inst_size = 4;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::inc || m_dec_opCode == eInstructionSet::inc_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::inc_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_opCode = eInstructionSet::inc;
					m_inst_size = 3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::dec || m_dec_opCode == eInstructionSet::dec_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::dec_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_opCode = eInstructionSet::dec;
					m_inst_size = 3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::push || m_dec_opCode == eInstructionSet::push_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::push_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_opCode = eInstructionSet::push;
					m_inst_size = 3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::pop || m_dec_opCode == eInstructionSet::pop_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::pop_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_opCode = eInstructionSet::pop;
					m_inst_size = 3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::pop_r || m_dec_opCode == eInstructionSet::pop_r_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::pop_r_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_opCode = eInstructionSet::pop;
					m_pop_r_flg = true;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::flg || m_dec_opCode == eInstructionSet::flg_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::flg_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_opCode = eInstructionSet::flg;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::_and || m_dec_opCode == eInstructionSet::and_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::and_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_dec_opCode = eInstructionSet::_and;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::_or || m_dec_opCode == eInstructionSet::or_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::or_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_dec_opCode = eInstructionSet::_or;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::_not || m_dec_opCode == eInstructionSet::not_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::not_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_opCode = eInstructionSet::_not;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::bit)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::mask)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::jmp)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_flags = (eFlags)m_raw_3;
					m_dec_op1 = (word)m_raw_4;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back((word)m_dec_flags);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::call)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					m_variable_inst_size = true;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::ret || m_dec_opCode == eInstructionSet::ret_m)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_dec_opCode == eInstructionSet::ret_m)
						m_inst_mode = D__MASKED_INST_MODE;
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_opCode = eInstructionSet::ret;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::cmd)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::lda_str)
				{
					m_inst_size = 1;
					m_variable_inst_size = true;
					m_decoded_inst.push_back((word)m_dec_opCode);
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::alloc)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::free)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::realloc)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_dec_op2 = (word)m_raw_4;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 4;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::reserve)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::free_s)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_op1 = (word)m_raw_3;
					m_inst_size = 3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back(m_dec_op1);
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::str_cpy)
				{ 
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}	
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					m_dec_op1 = (word)m_raw_2;
					m_dec_op2 = (word)m_raw_3;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 3;
					return true;
				}
				else if (m_dec_opCode == eInstructionSet::add_str)
				{
					if (m_err_2 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_2);
						return false;
					}
					if (m_err_3 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_3);
						return false;
					}
					if (m_err_4 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_4);
						return false;
					}
					if (m_err_5 != D__NO_ERROR)
					{
						pushError(D__CPU_ERR__INST_ERR_STEP_5);
						return false;
					}
					m_dec_addrMode = (eAddressingModes)m_raw_2;
					decode_addr_mode();
					m_dec_flags = (eFlags)m_raw_3;
					m_dec_op1 = (word)m_raw_4;
					m_dec_op2 = (word)m_raw_5;
					m_decoded_inst.push_back((word)m_dec_opCode);
					m_decoded_inst.push_back((word)m_dec_addrMode);
					m_decoded_inst.push_back((word)m_dec_flags);
					m_decoded_inst.push_back(m_dec_op1);
					m_decoded_inst.push_back(m_dec_op2);
					m_inst_size = 5;
					m_variable_inst_size = true;
					return true;
				}
				

				pushError(D__CPU_ERR__UNKNOWN_INSTRUCTION);
				return false;
			}

			bool CPU::execute(RAM &_ram, REG &_reg)
			{
				if (VirtualMachine::instance().getCurrentProcess().done()) return true;
				if (m_dec_opCode == eInstructionSet::no_op)
				{
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::req)
				{
					MemAddress __tmp_addr = oasm_nullptr;
					if (((word)m_dec_flags & (word)eFlags::req_all) == (word)eFlags::req_all) 
					{
						if (!_ram.request(m_dec_op1.val(), __tmp_addr, eMemCellType::Heap, VirtualMachine::instance().getCurrentProcess()))
						{
							pushError(D__CPU_ERR__HEAP_ALLOC_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						if (!_ram.request(m_dec_op1.val(), __tmp_addr, eMemCellType::Stack, VirtualMachine::instance().getCurrentProcess()))
						{
							pushError(D__CPU_ERR__STACK_ALLOC_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						_reg.rw_SP() = VirtualMachine::instance().getCurrentProcess().m_stackAddr + 1;
					}
					else if (((word)m_dec_flags & (word)eFlags::req_heap) == (word)eFlags::req_heap)
					{
						if (!_ram.request(m_dec_op1.val(), __tmp_addr, eMemCellType::Heap, VirtualMachine::instance().getCurrentProcess()))
						{
							pushError(D__CPU_ERR__HEAP_ALLOC_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else if (((word)m_dec_flags & (word)eFlags::req_stack) == (word)eFlags::req_stack)
					{
						if (!_ram.request(m_dec_op1.val(), __tmp_addr, eMemCellType::Stack, VirtualMachine::instance().getCurrentProcess()))
						{
							pushError(D__CPU_ERR__STACK_ALLOC_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						_reg.rw_SP() = VirtualMachine::instance().getCurrentProcess().m_stackAddr + 1;
					}
					else
					{
						pushError(D__CPU_ERR__UNKNOWN_FLAG);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::end)
				{
					BitEditor outData;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData, outData, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (__reg_op1)
					{
						if (!_reg.__read(outData.val(), outData))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else if (!m_const_op1)
					{
						if (!_ram.read(outData.val(), outData))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					_reg.rw_ES() = outData;
					VirtualMachine::instance().getCurrentProcess().stop();
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::mem)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false)
					if (__reg_op1)
					{
						if (!_reg.__write_m(outData1.val(), map_op2_to_m_param(outData2), (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.write_m(outData1.val(), map_op2_to_m_param(outData2), (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::add)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false)
					if (__reg_op1)
					{
						BitEditor __op1_val;
						if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val += map_op2_to_m_param(outData2);
						if (!_reg.__write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						BitEditor __op1_val;
						if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val += map_op2_to_m_param(outData2);
						if (!_ram.write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::sub)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false)
					if (__reg_op1)
					{
						BitEditor __op1_val;
						if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val -= map_op2_to_m_param(outData2);
						if (!_reg.__write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						BitEditor __op1_val;
						if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val -= map_op2_to_m_param(outData2);
						if (!_ram.write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::mul)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false) if (__reg_op1)
					{
						BitEditor __op1_val;
						if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val *= map_op2_to_m_param(outData2);
						if (!_reg.__write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						BitEditor __op1_val;
						if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val *= map_op2_to_m_param(outData2);
						if (!_ram.write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::div)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false) if (__reg_op1)
					{
						BitEditor __op1_val;
						if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						BitEditor __op2_val = map_op2_to_m_param(outData2);
						if (__op2_val == 0x0000)
						{
							pushError(D__CPU_ERR__DIVISION_BY_ZERO);
							__return_and_set_ip(false, m_old_pc_val)
						}
						word __reminder = (__op1_val % __op2_val).val();
						__op1_val /= __op2_val;
						_reg.rw_DR() = __reminder;
						if (!_reg.__write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						BitEditor __op1_val;
						if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						BitEditor __op2_val = map_op2_to_m_param(outData2);
						if (__op2_val == 0x0000)
						{
							pushError(D__CPU_ERR__DIVISION_BY_ZERO);
							__return_and_set_ip(false, m_old_pc_val)
						}
						word __reminder = (__op1_val % __op2_val).val();
						__op1_val /= __op2_val;
						_reg.rw_DR() = __reminder;
						if (!_ram.write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::cmp)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val, __op2_val;
					if (m_const_op1)
					{
						__op1_val = map_op1_to_m_param(outData1);
						__op2_val = map_op2_to_m_param(outData2);
					}
					else if (__reg_op1)
					{
						;
						if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op2_val = map_op2_to_m_param(outData2);
					}
					else
					{
						if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op2_val = map_op2_to_m_param(outData2);
					}
					if (__op1_val > __op2_val)
						_reg.rw_CF() = (word)eCompareState::Greater;
					else if (__op1_val < __op2_val)
						_reg.rw_CF() = (word)eCompareState::Less;
					else
						_reg.rw_CF() = (word)eCompareState::Equals;
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::inc)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false) if (__reg_op1)
					{
						BitEditor __op1_val;
						if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val++;
						if (!_reg.__write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						BitEditor __op1_val;
						if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val++;
						if (!_ram.write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::dec)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false) if (__reg_op1)
					{
						BitEditor __op1_val;
						if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val--;
						if (!_reg.__write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						BitEditor __op1_val;
						if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val--;
						if (!_ram.write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::push)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val;
					if (__reg_op1)
					{
						if (m_const_op1)
						{
							__op1_val = outData1;
						}
						else if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (m_const_op1)
						{
							__op1_val = outData1;
						}
						else if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					if (!pushToStack(__op1_val))
					{
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::pop)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false) bool __pop_flush = outData1 == oasm_nullptr;
					BitEditor __data;
					if (!__pop_flush)
					{
						if (!_ram.read(_reg.SP().val() - 1, __data))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						if (__reg_op1)
						{
							if (!_reg.__write_m(outData1.val(), __data, (word)m_op1_addr_mode))
							{
								pushError(D__CPU_ERR__WRITE_REG_FAILED);
								__return_and_set_ip(false, m_old_pc_val)
							}
						}
						else
						{
							if (!_ram.write_m(outData1.val(), __data, (word)m_op1_addr_mode))
							{
								pushError(D__CPU_ERR__READ_FAILED);
								__return_and_set_ip(false, m_old_pc_val)
							}
						}
					}
					else if (m_pop_r_flg)
					{
						pushError(D__CPU_ERR__POP_FLUSH_AND_READ);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (!m_pop_r_flg && _reg.SP() > VirtualMachine::instance().getCurrentProcess().m_stackAddr + 1)
						_reg.rw_SP()--;
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::flg)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val;
					if (m_const_op1)
					{
						__op1_val = outData1;
					}
					else if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else if (!_ram.read(outData1.val(), __op1_val))
					{
						pushError(D__CPU_ERR__READ_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					_reg.disableProtectedMode();
					if (!_reg.__write_m((word)eRegisters::FL, __op1_val, (word)m_op1_addr_mode))
					{
						pushError(D__CPU_ERR__WRITE_REG_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					_reg.enableProtectedMode();
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::_or ||
						 m_dec_opCode == eInstructionSet::_not ||
						 m_dec_opCode == eInstructionSet::_and)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false)
						BitEditor __op1_val;
					if (__reg_op1)
					{
						if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val = (__logic_op(__op1_val, map_op2_to_m_param(outData2), m_dec_opCode) > 0 ? 0x0001 : 0x0000);
						if (!_reg.__write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val = (__logic_op(__op1_val, map_op2_to_m_param(outData2), m_dec_opCode) > 0 ? 0x0001 : 0x0000);
						if (!_ram.write_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::bit)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false)
						BitEditor __op1_val;
					if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						eBit __bit = BitData::bitLst[outData2.getMSB()];
						bool __val = (outData2.getLSB() > 0);
						__op1_val.setBit(__bit, __val);
						if (!_reg.__write(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						eBit __bit = BitData::bitLst[outData2.getMSB()];
						bool __val = (outData2.getLSB() > 0);
						__op1_val.setBit(__bit, __val);
						if (!_ram.write(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::mask)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false)
						BitEditor __op1_val;
					if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val.mask(outData2.val());
						if (!_reg.__write(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op1_val.mask(outData2.val());
						if (!_ram.write(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::jmp)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val;
					if (m_const_op1)
					{
						__op1_val = outData1;
					}
					else if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					BitEditor __flags(m_raw_3);
					__op1_val = offsetCodeAddress(__op1_val.val());
					if (__flags.getMasked((word)eFlags::jmp_unconditional) == (word)eFlags::jmp_unconditional)
					{
						__return_and_set_ip(true, __op1_val)
					}
					else if (__flags.bit((eBit)eFlags::jmp_equals))
					{
						eCompareState __ecs = (eCompareState)_reg.CF().val();
						if (__ecs == eCompareState::Equals)
						{
							__return_and_set_ip(true, __op1_val)
						}
					}
					else if (__flags.bit((eBit)eFlags::jmp_greater))
					{
						eCompareState __ecs = (eCompareState)_reg.CF().val();
						if (__ecs == eCompareState::Greater)
						{
							__return_and_set_ip(true, __op1_val)
						}
					}
					else if (__flags.bit((eBit)eFlags::jmp_greater_eq))
					{
						eCompareState __ecs = (eCompareState)_reg.CF().val();
						if (__ecs == eCompareState::Equals || __ecs == eCompareState::Greater)
						{
							__return_and_set_ip(true, __op1_val)
						}
					}
					else if (__flags.bit((eBit)eFlags::jmp_less))
					{
						eCompareState __ecs = (eCompareState)_reg.CF().val();
						if (__ecs == eCompareState::Less)
						{
							__return_and_set_ip(true, __op1_val)
						}
					}
					else if (__flags.bit((eBit)eFlags::jmp_less_eq))
					{
						eCompareState __ecs = (eCompareState)_reg.CF().val();
						if (__ecs == eCompareState::Equals || __ecs == eCompareState::Less)
						{
							__return_and_set_ip(true, __op1_val)
						}
					}
					else if (__flags.bit((eBit)eFlags::jmp_not_eq))
					{
						eCompareState __ecs = (eCompareState)_reg.CF().val();
						if (__ecs != eCompareState::Equals)
						{
							__return_and_set_ip(true, __op1_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::call)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val, __op2_val;
					if (m_const_op1)
					{
						__op1_val = outData1;
						__op2_val = outData2;
					}
					else if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op2_val = outData2;
					}
					else
					{
						if (!_ram.read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op2_val = outData2;
					}
					MemAddress __new_ip = m_old_pc_val + m_inst_size;
					BitEditor __param_addr_mode, __param_data, __out_param_data, __param_data_val;
					TMemoryList __params;

					for (word __param_count = 0; __param_count < __op2_val.val(); __param_count++)
					{
						if (!_ram.read(__new_ip++, __param_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						if (!_ram.read(__new_ip++, __param_data))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						m_const_op1 = false;
						__reg_op1 = false;
						if (!map_addr_mode(_ram, _reg, (eAddressingModes)__param_addr_mode.val(), __param_data, __param_data, __out_param_data, __out_param_data, __reg_op1))
						{
							pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						if (m_const_op1)
						{
							__param_data_val = __out_param_data;
						}
						else if (__reg_op1)
						{
							if (!_reg.__read(__out_param_data.val(), __param_data_val))
							{
								pushError(D__CPU_ERR__READ_REG_FAILED);
								__return_and_set_ip(false, m_old_pc_val)
							}
						}
						else
						{
							if (!_ram.read(__out_param_data.val(), __param_data_val))
							{
								pushError(D__CPU_ERR__READ_FAILED);
								__return_and_set_ip(false, m_old_pc_val)
							}
						}
						__params.push_back(__param_data_val);
						m_inst_size += 2;
					}
					if (!pushToStack(__new_ip))
					{
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (!pushToStack(__op2_val))
					{
						__return_and_set_ip(false, m_old_pc_val)
					}
					for (auto& __p : __params)
					{
						if (!pushToStack(__p))
						{
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, offsetCodeAddress(__op1_val.val()))
				}
				else if (m_dec_opCode == eInstructionSet::ret)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val;
					if (m_const_op1)
					{
						__op1_val = map_op1_to_m_param(outData1);
					}
					else if (__reg_op1)
					{
						if (!_reg.__read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.read_m(outData1.val(), __op1_val, (word)m_op1_addr_mode))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					if (_reg.SP() > VirtualMachine::instance().getCurrentProcess().m_stackAddr + 2)
						_reg.rw_SP() -= 2;
					BitEditor __data;
					if (!_ram.read(_reg.SP().val(), __data))
					{
						pushError(D__CPU_ERR__READ_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					MemAddress __new_pc_val = __data.val();
					_reg.rw_RV() = __op1_val;
					__return_and_set_ip(true, __new_pc_val)
				}
				else if (m_dec_opCode == eInstructionSet::cmd)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val, __op2_val;
					if (m_const_op1)
					{
						__op1_val = outData1;
						__op2_val = outData2;
					}
					else if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op2_val = outData2;
					}
					else
					{
						if (!_ram.read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__op2_val = outData2;
					}
					BitEditor __tmp_result;
					if (!ECM::instance().execHandler(__op1_val.val(), __op2_val, VirtualMachine::instance(), __tmp_result))
					{//TODO: Diversify his error
						pushError(D__CPU_ERR__UNKNOWN_CMD_CODE);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (!_reg.write(eRegisters::R31, __tmp_result))
					{
						pushError(D__CPU_ERR__WRITE_REG_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::lda_str)
				{
					MemAddress __new_ip = m_old_pc_val + m_inst_size;
					BitEditor __tmp_stream_cell;
					TMemoryList __str_stream;
					while (true)
					{
						//TODO: Add Reserved MemCell for string length
						if (!_ram.read(__new_ip++, __tmp_stream_cell))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						__str_stream.push_back(__tmp_stream_cell);
						m_inst_size++;
						if (__tmp_stream_cell == 0 || __tmp_stream_cell.getLSB() == 0)
							break;
					}
					MemAddress __tmp_heap_addr;
					if (!_ram.requestPtr(__str_stream.size(), __tmp_heap_addr))
					{
						pushError(D__CPU_ERR__REQUEST_PTR_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (!_ram.writeToPtr(__tmp_heap_addr, __str_stream, true))
					{
						pushError(D__CPU_ERR__WRITE_PTR_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					_reg.write(eRegisters::R31, (word)__tmp_heap_addr);
					__return_and_set_ip(true, __new_ip)
				}
				else if (m_dec_opCode == eInstructionSet::alloc)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val;
					if (m_const_op1)
					{
						__op1_val = outData1;
					}
					else if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					MemAddress __ptr_addr;
					if (!_ram.requestPtr(__op1_val.val(), __ptr_addr))
					{
						pushError(D__CPU_ERR__REQUEST_PTR_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					_reg.write(eRegisters::R31, (word)__ptr_addr);
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::free)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val;
					if (m_const_op1)
					{
						__op1_val = outData1;
					}
					else if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					if (!_ram.freePtr(__op1_val.val()))
					{
						pushError(D__CPU_ERR__FREE_PTR_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::realloc)
				{
					BitEditor outData1, outData2;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op2, outData1, outData2, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_if_const_op1(false)
					BitEditor __op1_val = outData1;
					if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					//TODO: More efficient realloc instruction
					__op1_val = __localize_real_addr(__op1_val.val());
					TMemoryList __stream;
					if (!_ram.readFromPtr(__op1_val.val(), __stream, true))
					{
						pushError(D__CPU_ERR__READ_PTR_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}

					if (outData2 <= _ram.m_memory[__op1_val.val() - 1] - __op1_val)
					{
						pushError(D__CPU_ERR__REALLOC_SIZE_TOO_SMALL);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (!_ram.freePtr(__op1_val.val()))
					{
						pushError(D__CPU_ERR__FREE_PTR_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					MemAddress __ptr_addr;
					if (!_ram.requestPtr(outData2.val(), __ptr_addr))
					{
						pushError(D__CPU_ERR__REQUEST_PTR_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (!_ram.writeToPtr(__ptr_addr, __stream, true))
					{
						pushError(D__CPU_ERR__WRITE_PTR_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					_reg.write(eRegisters::R31, (word)__ptr_addr);
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::reserve)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					//__return_if_const_op1(false);
					MemAddress __addr;
					if (m_const_op1)
					{
						for (word __i = 0; __i < outData1.val(); __i++)
						{
							if (!__next_single_heap_cell(__addr))
							{
								pushError(D__CPU_ERR__RESERVE_SINGLE_FAILED);
								__return_and_set_ip(false, m_old_pc_val)
							}
						}
						__return_and_set_ip(true, m_old_pc_val + m_inst_size)
					}
					if (!__next_single_heap_cell(__addr))
					{
						pushError(D__CPU_ERR__RESERVE_SINGLE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (__reg_op1)
					{
						if (!_reg.__write(outData1.val(), __addr))
						{
							pushError(D__CPU_ERR__WRITE_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.write(outData1.val(), __addr))
						{
							pushError(D__CPU_ERR__WRITE_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::free_s)
				{
					BitEditor outData1;
					bool __reg_op1;
					if (!map_addr_mode(_ram, _reg, m_dec_addrMode, m_dec_op1, m_dec_op1, outData1, outData1, __reg_op1))
					{
						pushError(D__CPU_ERR__MAP_ADDR_MODE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					BitEditor __op1_val;
					if (m_const_op1)
					{
						__op1_val = outData1;
					}
					else if (__reg_op1)
					{
						if (!_reg.__read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_REG_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else
					{
						if (!_ram.read(outData1.val(), __op1_val))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					if (!__free_single_heap_cell(__op1_val.val()))
					{
						pushError(D__CPU_ERR__FREE_SINGLE_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::str_cpy)
				{
					BitEditor __op1_val, __op2_val;
					if (!_ram.read(offsetHeapAddress(m_dec_op1.val()), __op1_val))
					{
						pushError(D__CPU_ERR__READ_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (!_ram.read(offsetHeapAddress(m_dec_op2.val()), __op2_val))
					{
						pushError(D__CPU_ERR__READ_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					OmniaString __tmp_str;
					if (!_ram.readStringFromStream(__op2_val.val(), __tmp_str))
					{
						pushError(D__CPU_ERR__READ_STR_STREAM_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (!_ram.writeStringToStream(__op1_val.val(), __tmp_str))
					{
						pushError(D__CPU_ERR__WRITE_STR_STREAM_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				else if (m_dec_opCode == eInstructionSet::add_str)
				{
					BitEditor __op1_val, __op2_val;
					if (!_ram.read(offsetHeapAddress(m_dec_op1.val()), __op1_val))
					{
						pushError(D__CPU_ERR__READ_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					OmniaString __tmp_str_1, __tmp_str_2;
					if (!_ram.readStringFromStream(__op1_val.val(), __tmp_str_1))
					{
						pushError(D__CPU_ERR__READ_STR_STREAM_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					if (((word)m_dec_flags & (word)eFlags::add_str_str_ptr) == (word)eFlags::add_str_str_ptr)
					{
						if (!_ram.read(offsetHeapAddress(m_dec_op2.val()), __op2_val))
						{
							pushError(D__CPU_ERR__READ_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
						if (!_ram.readStringFromStream(__op2_val.val(), __tmp_str_2))
						{
							pushError(D__CPU_ERR__READ_STR_STREAM_FAILED);
							__return_and_set_ip(false, m_old_pc_val)
						}
					}
					else if (((word)m_dec_flags & (word)eFlags::add_str_const_stream) == (word)eFlags::add_str_const_stream)
					{
						m_inst_size--;
						MemAddress __new_ip = m_old_pc_val + m_inst_size;
						BitEditor __tmp_stream_cell;
						TMemoryList __str_stream;
						while (true)
						{
							//TODO: Add Reserved MemCell for string length
							if (!_ram.read(__new_ip++, __tmp_stream_cell))
							{
								pushError(D__CPU_ERR__READ_FAILED);
								__return_and_set_ip(false, m_old_pc_val)
							}
							__str_stream.push_back(__tmp_stream_cell);
							m_inst_size++;
							if (__tmp_stream_cell == 0 || __tmp_stream_cell.getLSB() == 0)
								break;
						}
						__tmp_str_2 = BitEditor::constStreamToString(__str_stream);
					}
					else if (((word)m_dec_flags & (word)eFlags::add_str_char) == (word)eFlags::add_str_char)
					{
					}
					else if (((word)m_dec_flags & (word)eFlags::add_str_int) == (word)eFlags::add_str_int)
					{
					}
					__tmp_str_1 = __tmp_str_1.add(__tmp_str_2);
					if (!_ram.writeStringToStream(__op1_val.val(), __tmp_str_1))
					{
						pushError(D__CPU_ERR__WRITE_STR_STREAM_FAILED);
						__return_and_set_ip(false, m_old_pc_val)
					}
					__return_and_set_ip(true, m_old_pc_val + m_inst_size)
				}
				
				__return_and_set_ip(false, m_old_pc_val)
			}

			bool CPU::map_addr_mode(RAM &_ram, REG &_reg, eAddressingModes mode, BitEditor op1, BitEditor op2, BitEditor &outOp1, BitEditor &outOp2, bool &__reg_op1)
			{
				__reg_op1 = false;
				/* --------------------------------------------- */
				if (mode == eAddressingModes::ConstToAddr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					outOp2 = op2;
					return true;
				}
				else if (mode == eAddressingModes::ConstToPtr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					BitEditor __tmp;
					if (!_ram.read(op1.val(), __tmp)) return false;
					outOp1 = offsetHeapAddress(__tmp.val());
					outOp2 = op2;
					return outOp1 != oasm_nullptr;
				}
				else if (mode == eAddressingModes::ConstToReg)
				{
					__reg_op1 = true;
					outOp2 = op2;
					outOp1 = op1;
					return true;
				}
				else if (mode == eAddressingModes::ConstToRegPtr)
				{
					outOp2 = op2;
					if (!_reg.__read(op1.val(), outOp1)) return false;
					outOp1 = offsetHeapAddress(outOp1.val());
					return outOp1 != oasm_nullptr;
				}
				/* --------------------------------------------- */
				else if (mode == eAddressingModes::AddrToAddr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::AddrToPtr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					if (!_ram.read(op2.val(), outOp2))
						return false;
					if (!_ram.read(op1.val(), op1)) return false;
					outOp1 = offsetHeapAddress(op1.val());
					return outOp1 != oasm_nullptr;
				}
				else if (mode == eAddressingModes::AddrToReg)
				{
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					__reg_op1 = true;
					outOp1 = op1;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::AddrToRegPtr)
				{
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					if (!_ram.read(op2.val(), outOp2))
						return false;
					BitEditor __tmp;
					if (!_reg.__read(op1.val(), __tmp)) return false;
					outOp1 = offsetHeapAddress(__tmp.val());
					return outOp1 != oasm_nullptr;
				}
				/* --------------------------------------------- */
				else if (mode == eAddressingModes::PtrToAddr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					BitEditor __tmp;
					if (!_ram.read(op2.val(), __tmp)) return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::PtrToPtr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					BitEditor __tmp;
					if (!_ram.read(op1.val(), __tmp)) return false;
					op1 = offsetHeapAddress(__tmp.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					if (!_ram.read(op2.val(), __tmp)) return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::PtrToReg)
				{
					__reg_op1 = true;
					outOp1 = op1;
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					BitEditor __tmp;
					if (!_ram.read(op2.val(), __tmp)) return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::PtrToRegPtr)
				{
					if (!_reg.__read(op1.val(), outOp1))
						return false;

					BitEditor __tmp;
					if (!_reg.__read(op1.val(), __tmp)) return false;
					op1 = offsetHeapAddress(__tmp.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					if (!_ram.read(op2.val(), __tmp)) return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				/* --------------------------------------------- */
				else if (mode == eAddressingModes::RefToAddr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					outOp2 = op2;
					return true;
				}
				else if (mode == eAddressingModes::RefToPtr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					BitEditor __tmp;
					if (!_ram.read(op1.val(), __tmp)) return false;
					outOp1 = offsetHeapAddress(__tmp.val());
					outOp2 = op2;
					return outOp1 != oasm_nullptr;
				}
				else if (mode == eAddressingModes::RefToReg)
				{
					__reg_op1 = true;
					outOp2 = op2;
					outOp1 = op1;
					return true;
				}
				else if (mode == eAddressingModes::RefToRegPtr)
				{
					outOp2 = op2;
					if (!_reg.__read(op1.val(), outOp1)) return false;
					outOp1 = offsetHeapAddress(outOp1.val());
					return outOp1 != oasm_nullptr;
				}
				/* --------------------------------------------- */
				else if (mode == eAddressingModes::RegPtrToAddr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					BitEditor __tmp;
					if (!_reg.__read(op2.val(), __tmp))
						return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::RegPtrToPtr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					BitEditor __tmp;
					if (!_ram.read(op1.val(), __tmp)) return false;
					op1 = offsetHeapAddress(__tmp.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					if (!_reg.__read(op2.val(), __tmp)) return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::RegPtrToReg)
				{
					outOp1 = op1;
					__reg_op1 = true;
					BitEditor __tmp;
					if (!_reg.__read(op2.val(), __tmp)) return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::RegPtrToRegPtr)
				{
					BitEditor __tmp;
					if (!_reg.__read(op1.val(), __tmp)) return false;
					op1 = offsetHeapAddress(__tmp.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					if (!_reg.__read(op2.val(), __tmp)) return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				/* --------------------------------------------- */
				else if (mode == eAddressingModes::RegToAddr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					return _reg.__read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::RegToPtr)
				{
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					BitEditor __tmp;
					if (!_ram.read(op1.val(), __tmp)) return false;
					op1 = offsetHeapAddress(__tmp.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					return _reg.__read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::RegToReg)
				{
					__reg_op1 = true;
					outOp1 = op1;
					return _reg.__read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::RegToRegPtr)
				{
					BitEditor __tmp;
					if (!_reg.__read(op1.val(), __tmp))
						return false;
					op1 = offsetHeapAddress(__tmp.val());
					if (op1 == oasm_nullptr) return false;
					outOp1 = op1;
					return _reg.__read(op2.val(), outOp2);
				}
				/* --------------------------------------------- */
				else if (mode == eAddressingModes::ConstConst)
				{
					m_const_op1 = true;
					outOp1 = op1;
					outOp2 = op2;
					return true;
				}
				else if (mode == eAddressingModes::ConstAddr)
				{
					m_const_op1 = true;
					outOp1 = op1;
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::ConstPtr)
				{
					m_const_op1 = true;
					outOp1 = op1;
					op2 = offsetHeapAddress(op2.val());
					if (op2 == oasm_nullptr) return false;
					BitEditor __tmp;
					if (!_ram.read(op2.val(), __tmp)) return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::ConstRef)
				{
					m_const_op1 = true;
					outOp1 = op1;
					outOp2 = op2;
					return true;
				}
				else if (mode == eAddressingModes::ConstRegPtr)
				{
					m_const_op1 = true;
					outOp1 = op1;
					BitEditor __tmp;
					if (!_reg.__read(op2.val(), __tmp)) return false;
					op2 = offsetHeapAddress(__tmp.val());
					if (op2 == oasm_nullptr) return false;
					return _ram.read(op2.val(), outOp2);
				}
				else if (mode == eAddressingModes::ConstReg)
				{
					m_const_op1 = true;
					outOp1 = op1;
					return _reg.__read(op2.val(), outOp2);
				}
				/* --------------------------------------------- */
				else if (mode == eAddressingModes::SingleOp_const)
				{
					m_const_op1 = true;
					m_single_op_inst = true;
					outOp1 = op1;
					outOp2 = outOp1;
					return true;
				}
				else if (mode == eAddressingModes::SingleOp_addr)
				{
					m_single_op_inst = true;
					outOp1 = offsetHeapAddress(op1.val());
					outOp2 = outOp1;
					return outOp1 != oasm_nullptr;
				}
				else if (mode == eAddressingModes::SingleOp_reg)
				{
					m_single_op_inst = true;
					__reg_op1 = true;
					outOp1 = op1;
					outOp2 = outOp1;
					return true;
				}
				else if (mode == eAddressingModes::SingleOp_ptr)
				{
					m_single_op_inst = true;
					op1 = offsetHeapAddress(op1.val());
					if (op1 == oasm_nullptr) return false;
					if (!_ram.read(op1.val(), outOp1)) return false;
					outOp1 = offsetHeapAddress(outOp1.val());
					outOp2 = outOp1;
					return outOp1 != oasm_nullptr;
				}
				else if (mode == eAddressingModes::SingleOp_regPtr)
				{
					if (!_reg.__read(op1.val(), outOp1))
						return false;
					outOp1 = offsetHeapAddress(outOp1.val());
					m_single_op_inst = true;
					outOp2 = outOp1;
					return outOp1 != oasm_nullptr;
				}
				else if (mode == eAddressingModes::SingleOp_ref)
				{
					m_single_op_inst = true;
					outOp1 = op1;
					outOp2 = outOp1;
					return true;
				}
				return false;
			}

			BitEditor CPU::map_op2_to_m_param(BitEditor op2)
			{
				if (m_op2_addr_mode == eSingleAddrModes::_2B)
					return op2;
				else if (m_op2_addr_mode == eSingleAddrModes::LSB)
					return (word)op2.getLSB();
				else if (m_op2_addr_mode == eSingleAddrModes::MSB)
					return (word)op2.getMSB();
				pushError(D__CPU_ERR__MAP_OP2_ADDR_MODE_FAILED);
				return 0;
			}

			BitEditor CPU::map_op1_to_m_param(BitEditor op1)
			{
				if (m_op1_addr_mode == eSingleAddrModes::_2B)
					return op1;
				else if (m_op1_addr_mode == eSingleAddrModes::LSB)
					return (word)op1.getLSB();
				else if (m_op1_addr_mode == eSingleAddrModes::MSB)
					return (word)op1.getMSB();
				pushError(D__CPU_ERR__MAP_OP1_ADDR_MODE_FAILED);
				return 0;
			}

			void CPU::decode_addr_mode(void)
			{
				if (m_inst_mode == D__MASKED_INST_MODE)
				{
					BitEditor __addr_mode = (word)m_dec_addrMode;
					m_m_param = __addr_mode.getSliced(eBit::Thirteen, 4);
					m_dec_addrMode = (eAddressingModes)__addr_mode.getSliced(eBit::One, 12);
					if (m_m_param == (word)eAddressingModes::b_2ByteMode_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::_2B;
						m_op2_addr_mode = eSingleAddrModes::_2B;
					}
					else if (m_m_param == (word)eAddressingModes::b_LSB_LSB_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::LSB;
						m_op2_addr_mode = eSingleAddrModes::LSB;
					}
					else if (m_m_param == (word)eAddressingModes::b_LSB_MSB_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::LSB;
						m_op2_addr_mode = eSingleAddrModes::MSB;
					}
					else if (m_m_param == (word)eAddressingModes::b_MSB_LSB_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::MSB;
						m_op2_addr_mode = eSingleAddrModes::LSB;
					}
					else if (m_m_param == (word)eAddressingModes::b_MSB_MSB_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::MSB;
						m_op2_addr_mode = eSingleAddrModes::MSB;
					}
					else if (m_m_param == (word)eAddressingModes::b_2B_LSB_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::_2B;
						m_op2_addr_mode = eSingleAddrModes::LSB;
					}
					else if (m_m_param == (word)eAddressingModes::b_2B_MSB_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::_2B;
						m_op2_addr_mode = eSingleAddrModes::MSB;
					}
					else if (m_m_param == (word)eAddressingModes::b_LSB_2B_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::LSB;
						m_op2_addr_mode = eSingleAddrModes::_2B;
					}
					else if (m_m_param == (word)eAddressingModes::b_MSB_2B_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::MSB;
						m_op2_addr_mode = eSingleAddrModes::_2B;
					}
					else if (m_m_param == (word)eAddressingModes::b_SingleOp_LSB_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::LSB;
						m_op2_addr_mode = eSingleAddrModes::LSB;
					}
					else if (m_m_param == (word)eAddressingModes::b_SingleOp_MSB_mask)
					{
						m_op1_addr_mode = eSingleAddrModes::MSB;
						m_op2_addr_mode = eSingleAddrModes::MSB;
					}
				}
			}

			BitEditor CPU::__logic_op(BitEditor op1, BitEditor op2, eInstructionSet inst)
			{
				bool __op1 = (op1 > 0), __op2 = (op2 > 0);
				switch (inst)
				{
				case eInstructionSet::_and:
					return __op1 && __op2;
				case eInstructionSet::_or:
					return __op1 || __op2;
				case eInstructionSet::_not:
					return !__op1;
				default:
					return false;
				}
				return false;
			}

			bool CPU::__next_single_heap_cell(MemAddress& outAddr)
			{
				Process& __proc = VirtualMachine::instance().getCurrentProcess();
				RAM& _ram = VirtualMachine::instance().getRAM();

				MemAddress __addr = __proc.m_heapAddr + 1;
				bool found = false;
				for ( ; __addr < __proc.m_heapAddr + __proc.m_heapSize + 1; __addr++)
				{
					if (_ram.m_memCells[__addr].proc->getID() != __proc.getID()) return false;;
					if (_ram.m_memCells[__addr].flag != eMemCellFlag::NoFlag) continue;
					if (_ram.m_memCells[__addr].state != eMemState::Allocated) continue;
					found = true;
					break;
				}
				if (!found) return false;

				_ram.m_memCells[__addr].flag = eMemCellFlag::UsedSingleHeapCell;
				_ram.m_memory[__addr] = 0x0000;
				__addr -= (__proc.m_heapAddr + 1);
				outAddr = __addr;
				return true;
			}

			bool CPU::__free_single_heap_cell(MemAddress __addr)
			{
				//if (m_heap_reserve_count == 0) return false;
				//Process& __proc = VirtualMachine::instance().getCurrentProcess();
				RAM& _ram = VirtualMachine::instance().getRAM();
				MemAddress __tmp_offset = offsetHeapAddress(__addr);
				if (__tmp_offset == oasm_nullptr) return false;
				if (_ram.m_memCells[__tmp_offset].flag != eMemCellFlag::UsedSingleHeapCell) return false;
				if (_ram.m_memCells[__tmp_offset].state != eMemState::Allocated) return false;
				m_next_single_heap = __addr;
				//m_heap_reserve_count--;
				_ram.m_memory[__tmp_offset] = 0xFFFF;
				_ram.m_memCells[__tmp_offset].flag = eMemCellFlag::NoFlag;
				return true;
			}

			MemAddress CPU::offsetHeapAddress(MemAddress __local_addr)
			{
				Process& __proc = VirtualMachine::instance().getCurrentProcess();
				if (__proc.isInvalidProc())
				{
					pushError(D__CPU_ERR__OFFSET_HEAP_INVALID_PROC);
					return oasm_nullptr;
				}
				if (!__proc.m_heapAllocated)
				{
					pushError(D__CPU_ERR__OFFSET_HEAP_NOT_REQUESTED);
					return oasm_nullptr;
				}
				return __local_addr + __proc.m_heapAddr + 1;
			}

			MemAddress CPU::offsetCodeAddress(MemAddress __local_addr)
			{
				Process& __proc = VirtualMachine::instance().getCurrentProcess();
				if (__proc.isInvalidProc())
				{
					pushError(D__CPU_ERR__OFFSET_CODE_INVALID_PROC);
					return oasm_nullptr;
				}
				if (!__proc.m_codeAllocated)
				{
					pushError(D__CPU_ERR__OFFSET_CODE_NOT_REQUESTED);
					return oasm_nullptr;
				}
				return __local_addr + __proc.m_codeAddr;
			}

			MemAddress CPU::__localize_real_addr(MemAddress __real_addr)
			{
				Process& __proc = VirtualMachine::instance().getCurrentProcess();
				if (__proc.isInvalidProc())
				{
					pushError(D__CPU_ERR__OFFSET_HEAP_INVALID_PROC);
					return oasm_nullptr;
				}
				if (!__proc.m_heapAllocated)
				{
					pushError(D__CPU_ERR__OFFSET_HEAP_NOT_REQUESTED);
					return oasm_nullptr;
				}
				return __real_addr - (__proc.m_heapAddr + 1);
			}

			bool CPU::clock_tick(void)
			{
				if (VirtualMachine::instance().getCurrentProcess().done()) return true;
				VirtualMachine &_vm = VirtualMachine::instance();
				OutputManager& out = *VirtualMachine::instance().getOutputHandler();
				Process& proc = _vm.getCurrentProcess();
				RAM &ram = _vm.getRAM();
				REG &reg = _vm.getREG();
				for (uint8 i = 0; i < m_ipc; i++)
				{
					reg.enableProtectedMode();
					m_break_point_signal = false;
					m_break_point_addr = oasm_nullptr;
					m_current_ipc = i;
					if (proc.done())
						return true;
					if (!fetch(ram, reg))
					{
						pushError(D__CPU_ERR__FETCH_STEP_FAILED);
						if (Interpreter::instance().__is_dbg_call()) return false;
						OmniaString __tmp = "";
						out.newLine().print("   Press <Enter> to show additional info on the last instruction.");
						out.newLine().newLine();
						_vm.getInputHandler()->read(__tmp);
						printMemory(out, 4, 4, 16, true);
						return false;
					}
					if (!decode(ram, reg))
					{
						pushError(D__CPU_ERR__DECODE_STEP_FAILED);
						if (Interpreter::instance().__is_dbg_call()) return false;
						OmniaString __tmp = "";
						out.newLine().print("   Press <Enter> to show additional info on the last instruction.");
						out.newLine().newLine();
						_vm.getInputHandler()->read(__tmp);
						printMemory(out, 4, 4, 16, true);
						return false;
					}
					if (!execute(ram, reg))
					{
						pushError(D__CPU_ERR__EXECUTE_STEP_FAILED);
						if (Interpreter::instance().__is_dbg_call()) return false;
						OmniaString __tmp = "";
						out.newLine().print("   Press <Enter> to show additional info on the last instruction.");
						out.newLine().newLine();
						_vm.getInputHandler()->read(__tmp);
						printMemory(out, 4, 4, 16, true);
						return false;
					}
					
					if (!Interpreter::instance().__is_dbg_call() && m_step_execution)
					{
						printMemory(out, 4, 4, 16, true);
						out.newLine().print("  Press <Enter> for next cycle.").newLine();
						out.newLine().print("  #[-cmd-]/> ");
						_vm.getInputHandler()->read(m_cmd_command);
					}
					else if (!m_break_points.empty())
					{
						for (MemAddress __br = m_old_pc_val; __br < m_old_pc_val + m_inst_size; __br++)
						{
							if (STDVEC_CONTAINS(m_break_points, __br - proc.m_codeAddr))
							{
								if (Interpreter::instance().__is_dbg_call())
								{
									m_break_point_signal = true;
									m_break_point_addr = __br - proc.m_codeAddr;
									break;
								}
								printMemory(out, 4, 4, 16, true);
								out.newLine().print("  Press <Enter> to continue execution.").newLine();
								out.newLine().print("  #[-cmd-]/> ");
								_vm.getInputHandler()->read(m_cmd_command);
								break;
							}
						}
					}
				}
				return true;
			}

			bool CPU::isBreakPoint(MemAddress __addr, uint8 __inst_size)
			{
				for (MemAddress __br = __addr; __br < __addr + __inst_size; __br++)
				{
					if (STDVEC_CONTAINS(m_break_points, __br - VirtualMachine::instance().getCurrentProcess().m_codeAddr))
						return true;
				}
				return false;
			}

			void CPU::printMemory(OutputManager& out, word __stack_rows, word __heap_rows, word __code_rows, bool __print_instruction_data)
			{
				char __sep_c = '~';
				uint16 w = 107;
				StringBuilder tmp;
				out.newLine();
				tmp.add(Utils::duplicateChar(' ', 35)).add(Utils::duplicateChar(__sep_c, w - 70)).add(Utils::duplicateChar(' ', 35));
				Utils::printMemoryBlock(D__STACK_SPACE_START, D__STACK_SPACE_START + (__stack_rows * 16), "Stack", out, VirtualMachine::instance().getREG().SP().val());
				out.newLine().print(tmp.get());
				Utils::printRegisters(out);
				out.newLine().print(tmp.get());
				Utils::printMemoryBlock(D__HEAP_SPACE_START, D__HEAP_SPACE_START + (__heap_rows * 16), "Heap", out);
				out.newLine().print(tmp.get());
				Utils::printMemoryBlock(D__MEMORY_START, D__MEMORY_START + (__code_rows * 16), "Code", out, (__print_instruction_data ? m_old_pc_val : oasm_nullptr), m_inst_size - 1);

				if (!__print_instruction_data) return;

				out.newLine().print(tmp.get());
				
				tmp = StringBuilder("|Last Instruction - INFO");
				out.newLine().print(Utils::duplicateChar('=', w)).newLine();
				out.print(tmp.get()).print(Utils::duplicateChar(' ', w - tmp.get().length() - 1)).print("|");
				out.newLine().print(Utils::duplicateChar('=', w)).newLine();
				tmp = StringBuilder("<<    ");
				if (m_inst_size >= 1)
					tmp.add(Utils::mapInstruction((eInstructionSet)m_raw_1));
				if (m_inst_size >= 2)
					tmp.add(",   ").add(Utils::intToHexStr(m_raw_2));
				if (m_inst_size >= 3)
					tmp.add(",   ").add(Utils::intToHexStr(m_raw_3));
				if (m_inst_size >= 4)
					tmp.add(",   ").add(Utils::intToHexStr(m_raw_4));
				if (m_inst_size >= 5)
					tmp.add(",   ").add(Utils::intToHexStr(m_raw_5));
				if (m_inst_size >= 6)
					tmp.add(",   ").add(Utils::intToHexStr(m_raw_6));
				if (m_variable_inst_size)
					tmp.add(",   (***)");
				tmp.add("    >>");
				out.print("|").print(Utils::duplicateChar(' ', (w - tmp.get().length() - 2) / 2)).print(tmp.get());
				out.print(Utils::duplicateChar(' ', (w - tmp.get().length() - 2) / 2)).print("|").newLine();
				out.print("|").print(Utils::duplicateChar('-', w - 2)).print("|").newLine();
				
				tmp = StringBuilder("| Address:                 ").add(Utils::intToHexStr(m_old_pc_val));
				out.print(tmp.get()).print(Utils::duplicateChar(' ', w - tmp.get().length() - 1)).print("|").newLine();
				
				tmp = StringBuilder("| Curent IPC:              ");
				tmp.add(m_current_ipc);
				tmp.add(Utils::duplicateChar(' ', w - tmp.get().length() - 1)).add("|");
				out.print(tmp.get()).newLine();

				tmp = StringBuilder("| Addressing Mode:         ");
				tmp.add(Utils::mapAddressingMode(m_dec_addrMode));
				tmp.add(Utils::duplicateChar(' ', w - tmp.get().length() - 1)).add("|");
				out.print(tmp.get()).newLine();

				tmp = StringBuilder("| Mask Parameter:          ");
				tmp.add(Utils::mapMaskParam(m_m_param));
				tmp.add(Utils::duplicateChar(' ', w - tmp.get().length() - 1)).add("|");
				out.print(tmp.get()).newLine();

				bool __sep = false;
				tmp = StringBuilder("| VirtualCPU Flags:        ");
				if (m_inst_mode == D__MASKED_INST_MODE)
				{
					if (__sep) tmp.add(", ");
					tmp.add("masked");
					__sep = true;
				}
				if (m_single_op_inst)
				{
					if (__sep) tmp.add(", ");
					tmp.add("single_op");
					__sep = true;
				}
				if (m_const_op1)
				{
					if (__sep) tmp.add(", ");
					tmp.add("const_op1");
					__sep = true;
				}
				if (m_variable_inst_size)
				{
					if (__sep) tmp.add(", ");
					tmp.add("variable_inst_size");
					__sep = true;
				}
				tmp.add(Utils::duplicateChar(' ', w - tmp.get().length() - 1)).add("|");
				out.print(tmp.get()).newLine();

				out.print(Utils::duplicateChar('=', w)).newLine().newLine();
			}

			bool CPU::pushToStack(BitEditor __data)
			{
				REG& _reg = VirtualMachine::instance().getREG();
				if (!VirtualMachine::instance().getRAM().write(_reg.SP().val(), __data))
				{
					pushError(D__CPU_ERR__WRITE_FAILED);
					return false;
				}
				_reg.rw_SP()++;
				if (_reg.SP() >= D__LIB_SPACE_START)
				{
					pushError(D__CPU_ERR__STACK_OVERFLOW);
					return false;
				}
				return true;
			}

			void CPU::pushError(ErrorCode __err_code)
			{
				ErrorReciever::pushError(__err_code, *VirtualMachine::instance().getOutputHandler(), "VirtualCPU Error", m_old_pc_val, m_raw_1);
			}




			GPU::GPU(void)
			{
				m_vram.resize(D__VIDEO_MEMORY_SIZE);
				Utils::get_terminal_size(m_screen_x, m_screen_y);
				m_screen_buffer_addr = 0x00FF;
				clearScreenBuffer();
			}

			void GPU::mapBgColor(eOasmColors __bg)
			{
				switch (__bg)
				{
					case eOasmColors::Grey:
						VirtualMachine::instance().getOutputHandler()->bc_grey();
					break;
					case eOasmColors::Red:
						VirtualMachine::instance().getOutputHandler()->bc_red();
					break;
					case eOasmColors::Green:
						VirtualMachine::instance().getOutputHandler()->bc_green();
					break;
					case eOasmColors::Yellow:
						VirtualMachine::instance().getOutputHandler()->bc_yellow();
					break;
					case eOasmColors::Blue:
						VirtualMachine::instance().getOutputHandler()->bc_blue();
					break;
					case eOasmColors::Magenta:
						VirtualMachine::instance().getOutputHandler()->bc_magenta();
					break;
					case eOasmColors::Cyan:
						VirtualMachine::instance().getOutputHandler()->bc_cyan();
					break;
					case eOasmColors::White:
						VirtualMachine::instance().getOutputHandler()->bc_white();
					break;

					case eOasmColors::BrightGrey:
						VirtualMachine::instance().getOutputHandler()->bc_brightGrey();
					break;
					case eOasmColors::BrightRed:
						VirtualMachine::instance().getOutputHandler()->bc_brightRed();
					break;
					case eOasmColors::BrightGreen:
						VirtualMachine::instance().getOutputHandler()->bc_brightGreen();
					break;
					case eOasmColors::BrightYellow:
						VirtualMachine::instance().getOutputHandler()->bc_brightYellow();
					break;
					case eOasmColors::BrightBlue:
						VirtualMachine::instance().getOutputHandler()->bc_brightBlue();
					break;
					case eOasmColors::BrightMagenta:
						VirtualMachine::instance().getOutputHandler()->bc_brightMagenta();
					break;
					case eOasmColors::BrightCyan:
						VirtualMachine::instance().getOutputHandler()->bc_brightCyan();
					break;
					case eOasmColors::BrightWhite:
						VirtualMachine::instance().getOutputHandler()->bc_brightWhite();
					break;
					default: break;
				}
			}

			void GPU::mapFgColor(eOasmColors __fg)
			{
				switch (__fg)
				{
					case eOasmColors::Grey:
						VirtualMachine::instance().getOutputHandler()->fc_grey();
					break;
					case eOasmColors::Red:
						VirtualMachine::instance().getOutputHandler()->fc_red();
					break;
					case eOasmColors::Green:
						VirtualMachine::instance().getOutputHandler()->fc_green();
					break;
					case eOasmColors::Yellow:
						VirtualMachine::instance().getOutputHandler()->fc_yellow();
					break;
					case eOasmColors::Blue:
						VirtualMachine::instance().getOutputHandler()->fc_blue();
					break;
					case eOasmColors::Magenta:
						VirtualMachine::instance().getOutputHandler()->fc_magenta();
					break;
					case eOasmColors::Cyan:
						VirtualMachine::instance().getOutputHandler()->fc_cyan();
					break;
					case eOasmColors::White:
						VirtualMachine::instance().getOutputHandler()->fc_white();
					break;

					case eOasmColors::BrightGrey:
						VirtualMachine::instance().getOutputHandler()->fc_brightGrey();
					break;
					case eOasmColors::BrightRed:
						VirtualMachine::instance().getOutputHandler()->fc_brightRed();
					break;
					case eOasmColors::BrightGreen:
						VirtualMachine::instance().getOutputHandler()->fc_brightGreen();
					break;
					case eOasmColors::BrightYellow:
						VirtualMachine::instance().getOutputHandler()->fc_brightYellow();
					break;
					case eOasmColors::BrightBlue:
						VirtualMachine::instance().getOutputHandler()->fc_brightBlue();
					break;
					case eOasmColors::BrightMagenta:
						VirtualMachine::instance().getOutputHandler()->fc_brightMagenta();
					break;
					case eOasmColors::BrightCyan:
						VirtualMachine::instance().getOutputHandler()->fc_brightCyan();
					break;
					case eOasmColors::BrightWhite:
						VirtualMachine::instance().getOutputHandler()->fc_brightWhite();
					break;
					default: break;
				}
			}

			bool GPU::clock_tick(void)
			{
				BitEditor __char_data;
				eOasmColors __bg, __fg;
				for (MemAddress __addr = m_screen_buffer_addr; __addr < m_screen_buffer_addr + (m_screen_x * m_screen_y); __addr++)
				{
					__char_data = m_vram[__addr];
					__fg = (eOasmColors)((ubyte)((__char_data.getMSB() & 0x0F)));
					__bg = (eOasmColors)((ubyte)((__char_data.getMSB() & 0xF0) >> 4));
					mapFgColor(__fg);
					mapBgColor(__bg);

					VirtualMachine::instance().getOutputHandler()->print(OmniaString("").add((char)__char_data.getLSB()));
				}
				return true;
			}

			void GPU::plotChar(word __x, word __y, char __c, eOasmColors __bg_col, eOasmColors __fg_col)
			{
				word __index = (m_screen_x * __y) + __x;
				__index += m_screen_buffer_addr;
				BitEditor __char_data;
				ubyte __bg = (ubyte)__bg_col;
				ubyte __fg = (ubyte)__fg_col;
				__char_data.setLSB((ubyte)__c);
				__char_data.setMSB((ubyte)((__bg << 4) | __fg));
				m_vram[__index] = __char_data;
			}

			void GPU::clearScreenBuffer(void)
			{
				for (MemAddress __addr = m_screen_buffer_addr; __addr < m_screen_buffer_addr + (m_screen_x * m_screen_y); __addr++)
					m_vram[__addr] = (word)(' ');
			}




			RAM &RAM::create(uint16 size)
			{
				m_memory.resize(size, BitEditor(0));
				m_memCells.resize(size, MemCell());

				m_memCells[oasm_nullptr].state = eMemState::Reserved;
				m_memCells[oasm_nullptr].type = eMemCellType::Normal;
				m_memCells[oasm_nullptr].flag = eMemCellFlag::NullPointer;

				MemAddress i = oasm_nullptr;
				for (i = 1; i < D__HEAP_SPACE_START; i++)
				{
					m_memCells[i].state = eMemState::Free;
					m_memCells[i].type = eMemCellType::Normal;
					m_memCells[i].proc = &Process::invalidProc();
					m_memory[i] = 0xEEEE;
				}
				for (; i < D__STACK_SPACE_START; i++)
				{
					m_memCells[i].state = eMemState::Free;
					m_memCells[i].type = eMemCellType::Heap;
					m_memCells[i].proc = &Process::invalidProc();
					m_memory[i] = 0xEEEE;
				}
				for (; i < D__LIB_SPACE_START; i++)
				{
					m_memCells[i].state = eMemState::Free;
					m_memCells[i].type = eMemCellType::Stack;
					m_memCells[i].proc = &Process::invalidProc();
					m_memory[i] = 0xEEEE;
				}
				for (; i < D__MEMORY_SIZE; i++)
				{
					m_memCells[i].state = eMemState::Free;
					m_memCells[i].type = eMemCellType::Library;
					m_memCells[i].proc = &Process::invalidProc();
					m_memory[i] = 0xEEEE;
				}

				return *this;
			}

			MemAddress RAM::requestFrom(MemAddress start, word size, MemAddress max, bool __alloc)
			{
				size += 1;
				for (MemAddress i = start; i < max; i++)
				{
					if (m_memCells[i].state == (__alloc ? eMemState::Free : eMemState::Allocated))
					{
						if (!__alloc && m_memCells[i].flag == eMemCellFlag::UsedSingleHeapCell) continue;
						bool found = true;
						for (MemAddress j = 1; j < size; j++)
						{
							if (i + j >= max)
								return oasm_nullptr;
							if (m_memCells[i + j].state != (__alloc ? eMemState::Free : eMemState::Allocated))
							{
								i = i + j + 1;
								found = false;
								break;
							}
						}
						if (!found)
							continue;
						return i;
					}
				}
				return oasm_nullptr;
			}

			bool RAM::allocate(Process &proc, MemAddress start, word size, word __start_value, eMemCellFlag __ptr_flag)
			{
				size += 1;
				if (start + size >= D__MEMORY_SIZE)
					return false;
				for (MemAddress i = start; i < start + size; i++)
				{
					m_memCells[i].proc = &proc;
					m_memCells[i].state = eMemState::Allocated;
					m_memory[i] = __start_value;
				}
				m_memory[start] = start + size - 1;
				m_memCells[start].state = eMemState::Reserved;
				m_memCells[start].flag = __ptr_flag;
				return true;
			}

			bool RAM::request(word size, MemAddress &outAddr, eMemCellType type, Process &proc)
			{
				outAddr = oasm_nullptr;
				MemAddress _start = oasm_nullptr;
				MemAddress _tmp = oasm_nullptr;
				if (type == eMemCellType::Heap)
				{
					if (proc.m_heapAllocated)
						return false;
					_start = D__HEAP_SPACE_START;
					_tmp = requestFrom(_start, size, D__STACK_SPACE_START - 1);
					if (_tmp == oasm_nullptr)
						return false;
					if (!allocate(proc, _tmp, size))
						return false;
					proc.m_heapAddr = _tmp;
					proc.m_heapAllocated = true;
					proc.m_heapSize = size;
					outAddr = _tmp + 1;
					return true;
				}
				else if (type == eMemCellType::Stack)
				{
					if (proc.m_stackAllocated)
						return false;
					_start = D__STACK_SPACE_START;
					_tmp = requestFrom(_start, size, D__LIB_SPACE_START - 1);
					if (_tmp == oasm_nullptr)
						return false;
					if (!allocate(proc, _tmp, size))
						return false;
					proc.m_stackAddr = _tmp;
					proc.m_stackAllocated = true;
					proc.m_stackSize = size;
					outAddr = _tmp + 1;
					return true;
				}
				else if (type == eMemCellType::Normal)
				{
					if (proc.m_codeAllocated)
						return false;
					_start = D__MEMORY_START;
					_tmp = requestFrom(_start, size, D__HEAP_SPACE_START - 1, true);
					if (_tmp == oasm_nullptr)
						return false;
					if (!allocate(proc, _tmp, size))
						return false;
					proc.m_codeAddr = _tmp;
					proc.m_codeAllocated = true;
					proc.m_codeSize = size;
					outAddr = _tmp + 1;
					return true;
				}
				else
					return false;
			}

			bool RAM::requestPtr(word size, MemAddress &outAddr)
			{
				__return_if_current_proc_invalid(false)
					size += 1;
				outAddr = oasm_nullptr;
				MemAddress addr = requestFrom(__proc.m_heapAddr, size, m_memory[__proc.m_heapAddr].val(), false);
				if (addr == oasm_nullptr)
					return false;
				for (MemAddress i = addr; i < addr + size; i++)
				{
					m_memCells[i].state = eMemState::Used;
					m_memory[i] = 0x0000;
				}
				m_memory[addr] = addr + size - 1;
				m_memCells[addr].state = eMemState::Reserved;
				m_memCells[addr].flag = eMemCellFlag::HeapPtr;
				outAddr = addr + 1;
				return true;
			}

			bool RAM::freePtr(MemAddress addr)
			{
				__return_if_current_proc_invalid(false)
				if (addr == oasm_nullptr)
				{
					pushError(D__RAM_ERR__FREE_NULLPTR, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				if (m_memCells[addr - 1].state != eMemState::Reserved || m_memCells[addr - 1].flag != eMemCellFlag::HeapPtr)
				{
					pushError(D__RAM_ERR__FREE_INVALID_PTR, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				Process *proc = m_memCells[addr - 1].proc;
				if (__proc.getID() != proc->getID() || proc->isInvalidProc())
				{
					pushError(D__RAM_ERR__FREE_DISCREPANT_IDS, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				for (MemAddress i = addr; i <= m_memory[addr - 1].val(); i++)
				{
					m_memCells[i].state = eMemState::Allocated;
					m_memory[i] = 0xFFFF;
				}
				m_memCells[addr - 1].state = eMemState::Allocated;
				m_memCells[addr - 1].flag = eMemCellFlag::NoFlag;
				m_memory[addr - 1] = 0xFFFF;
				return true;
			}

			bool RAM::read(MemAddress addr, BitEditor &outData)
			{
				if (!protectedMode())
				{
					outData = m_memory[addr];
					return true;
				}
				__return_if_current_proc_invalid(false)
				if (m_memCells[addr].state == eMemState::Reserved)
				{
					pushError(D__RAM_ERR__READ_FAILED_RESERVED_CELL, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				Process &cellProc = *m_memCells[addr].proc;
				if (__proc.getID() != cellProc.getID() || cellProc.isInvalidProc())
				{
					pushError(D__RAM_ERR__READ_FAILED_DISCREPANT_IDS, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				outData = m_memory[addr];
				return true;
			}

			bool RAM::read_m(MemAddress addr, BitEditor &outData, word __m_param)
			{
				if (protectedMode())
				{
					__return_if_current_proc_invalid(false)
					if (m_memCells[addr].state == eMemState::Reserved)
					{
						pushError(D__RAM_ERR__READ_FAILED_RESERVED_CELL, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
						return false;
					}
					Process &cellProc = *m_memCells[addr].proc;
					if (__proc.getID() != cellProc.getID() || cellProc.isInvalidProc())
					{
						pushError(D__RAM_ERR__READ_FAILED_DISCREPANT_IDS, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
						return false;
					}
				}
				if (__m_param == (word)eSingleAddrModes::_2B)
					outData = m_memory[addr];
				else if (__m_param == (word)eSingleAddrModes::LSB)
					outData = (word)m_memory[addr].getLSB();
				else if (__m_param == (word)eSingleAddrModes::MSB)
					outData = (word)m_memory[addr].getMSB();
				else
				{
					pushError(D__RAM_ERR__READ_M_UNKNOWN_PARAM, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				return true;
			}

			bool RAM::write(MemAddress addr, BitEditor data)
			{
				if (!protectedMode())
				{
					m_memory[addr] = data;
					return true;
				}
				__return_if_current_proc_invalid(false)
				if (m_memCells[addr].state == eMemState::Reserved)
				{
					pushError(D__RAM_ERR__WRITE_FAILED_RESERVED_CELL, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				if (m_memCells[addr].flag == eMemCellFlag::ConstLocalSpace)
				{
					pushError(D__RAM_ERR__WRITE_FAILED_CONST, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				if (m_memCells[addr].state == eMemState::Used)
				{
					pushError(D__RAM_ERR__WRITE_USED, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				if (m_memCells[addr].type == eMemCellType::Normal)
				{
					pushError(D__RAM_ERR__WRITE_CODE, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				Process &cellProc = *m_memCells[addr].proc;
				if (__proc.getID() != cellProc.getID() || cellProc.isInvalidProc())
				{
					pushError(D__RAM_ERR__WRITE_FAILED_DISCREPANT_IDS, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				if (addr >= D__HEAP_SPACE_START && addr < D__STACK_SPACE_START)
				{
					if (m_memCells[addr].flag != eMemCellFlag::UsedSingleHeapCell)
					{
						pushError(D__RAM_ERR__WRITE_FAILED_UNALLOCATED, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
						return false;
					}
					m_memCells[addr].flag = eMemCellFlag::UsedSingleHeapCell;
				}
				m_memory[addr] = data;
				return true;
			}

			bool RAM::write_m(MemAddress addr, BitEditor data, word __m_param)
			{
				if (protectedMode())
				{
					__return_if_current_proc_invalid(false)
					if (m_memCells[addr].state == eMemState::Reserved)
					{
						pushError(D__RAM_ERR__WRITE_FAILED_RESERVED_CELL, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
						return false;
					}
					if (m_memCells[addr].flag == eMemCellFlag::ConstLocalSpace)
					{
						pushError(D__RAM_ERR__WRITE_FAILED_CONST, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
						return false;
					}
					if (m_memCells[addr].state == eMemState::Used)
					{
						pushError(D__RAM_ERR__WRITE_USED, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
						return false;
					}
					if (m_memCells[addr].flag != eMemCellFlag::UsedSingleHeapCell) //TODO: this will fail when instructions try to access stack via this method
					{
						pushError(D__RAM_ERR__WRITE_FAILED_UNALLOCATED, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
						return false;
					}
					Process &cellProc = *m_memCells[addr].proc;
					if (__proc.getID() != cellProc.getID() || cellProc.isInvalidProc())
					{
						pushError(D__RAM_ERR__WRITE_FAILED_DISCREPANT_IDS, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
						return false;
					}
				}
				if (__m_param == (word)eSingleAddrModes::_2B)
					m_memory[addr] = data;
				else if (__m_param == (word)eSingleAddrModes::LSB)
					m_memory[addr].setLSB((ubyte)data.val());
				else if (__m_param == (word)eSingleAddrModes::MSB)
					m_memory[addr].setMSB((ubyte)data.val());
				else
				{
					pushError(D__RAM_ERR__WRITE_M_UNKNOWN_PARAM, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error", addr);
					return false;
				}
				return true;
			}

			bool RAM::writeToPtr(MemAddress ptr, std::vector<BitEditor> data, bool __safe)
			{//TODO: Add protected mode check
				__return_if_current_proc_invalid(false)
				if (m_memCells[ptr - 1].state != eMemState::Reserved) return false;
				if (m_memCells[ptr - 1].flag != eMemCellFlag::HeapPtr)
					return false;
				if (ptr + (word)data.size() - 1 > m_memory[ptr - 1].val())
					return false;
				Process *proc = m_memCells[ptr - 1].proc;
				if (__proc.getID() != proc->getID() || proc->isInvalidProc())
					return false;
				if (__safe)
				{
					for (word i = ptr; i < ptr + data.size(); i++)
					{
						proc = m_memCells[i].proc;
						if (m_memCells[i].state == eMemState::Reserved ||
							m_memCells[i].flag == eMemCellFlag::HeapPtr ||
							m_memCells[i].flag == eMemCellFlag::HeapBlockStart ||
							m_memCells[i].flag == eMemCellFlag::ConstCell ||
							__proc.getID() != proc->getID() ||
							proc->isInvalidProc())
							return false;
					}
				}
				for (word i = 0; i < (word)data.size(); i++)
					m_memory[ptr + i] = data[i];
				return true;
			}

			bool RAM::readFromPtr(MemAddress ptr, std::vector<BitEditor>& outData, bool __safe)
			{//TODO: Add protected mode check
				__return_if_current_proc_invalid(false)
				if (m_memCells[ptr - 1].state != eMemState::Reserved) return false;
				if (m_memCells[ptr - 1].flag != eMemCellFlag::HeapPtr)
					return false;
				MemAddress __end_addr = m_memory[ptr - 1].val();
				//word __size = __end_addr - ptr;
				Process *proc = m_memCells[ptr - 1].proc;
				if (__proc.getID() != proc->getID() || proc->isInvalidProc())
					return false;
				if (__safe)
				{
					for (word i = ptr; i < __end_addr; i++)
					{
						proc = m_memCells[i].proc;
						if (m_memCells[i].state == eMemState::Reserved ||
							m_memCells[i].flag == eMemCellFlag::HeapPtr ||
							m_memCells[i].flag == eMemCellFlag::HeapBlockStart ||
							m_memCells[i].flag == eMemCellFlag::ConstCell ||
							__proc.getID() != proc->getID() ||
							proc->isInvalidProc())
							return false;
					}
				}
				for (word i = ptr; i <= __end_addr; i++)
					outData.push_back(m_memory[i]);
				return true;
			}

			bool RAM::readStringFromStream(MemAddress __stream_addr, OmniaString& out)
			{
				TMemoryList __stream;
				if (!readFromPtr(__stream_addr, __stream, true)) return false;
				out = BitEditor::constStreamToString(__stream);
				return true;
			}

			bool RAM::writeStringToStream(MemAddress __stream_addr, OmniaString str)
			{
				TMemoryList __str_stream = BitEditor::stringToConstSream(str);
				return writeToPtr(__stream_addr, __str_stream, true);
			}




			REG &REG::create(void)
			{
				m_regs.resize((word)eRegisters::Count, BitEditor(0x0000));
				rw_IP() = D__MEMORY_START;
				return *this;
			}

			bool REG::__read(word addr, BitEditor &outData)
			{
				if (!m_safe_mode || (addr >= (word)eRegisters::ES && addr <= (word)eRegisters::FL) ||
					 (addr >= (word)eRegisters::R0 && addr <= (word)eRegisters::R31))
				{
					outData = m_regs[addr];
					return true;
				}
				outData = BitEditor(oasm_nullptr);
				return false;
			}

			bool REG::__read_m(word addr, BitEditor &outData, word __m_param)
			{
				if (!m_safe_mode || (addr >= (word)eRegisters::ES && addr <= (word)eRegisters::FL) ||
					 (addr >= (word)eRegisters::R0 && addr <= (word)eRegisters::R31))
				{
					if (__m_param == (word)eSingleAddrModes::_2B)
						outData = m_regs[addr];
					else if (__m_param == (word)eSingleAddrModes::LSB)
						outData = (word)m_regs[addr].getLSB();
					else if (__m_param == (word)eSingleAddrModes::MSB)
						outData = (word)m_regs[addr].getMSB();
					else
						return false;
					return true;
				}
				outData = BitEditor(oasm_nullptr);
				return false;
			}

			bool REG::__write(word addr, BitEditor data)
			{
				if (!m_safe_mode || (addr >= (word)eRegisters::R0 && addr <= (word)eRegisters::R31))
				{
					m_regs[addr] = data;
					return true;
				}
				return false;
			}

			bool REG::__write_m(word addr, BitEditor data, word __m_param)
			{
				if (!m_safe_mode || (addr >= (word)eRegisters::R0 && addr <= (word)eRegisters::R31))
				{
					if (__m_param == (word)eSingleAddrModes::_2B)
						m_regs[addr] = data;
					else if (__m_param == (word)eSingleAddrModes::LSB)
						m_regs[addr].setLSB((ubyte)data.val());
					else if (__m_param == (word)eSingleAddrModes::MSB)
						m_regs[addr].setMSB((ubyte)data.val());
					else
						return false;
					return true;
				}
				return false;
			}
		}




		VirtualMachine::VirtualMachine(void)
		{
		}

		Process &VirtualMachine::getCurrentProcess(void)
		{
			if (m_currentProc == nullptr)
				return Process::invalidProc();
			return *m_currentProc;
		}
	}
}