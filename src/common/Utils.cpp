#include "Utils.hpp"
#include "Interpreter.hpp"
#include "IOManager.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#endif // Windows/Linux

namespace Omnia
{
	namespace common
	{
		bool Utils::isHex(OmniaString hex)
		{
			_string s = hex.toLowerCase().cpp();
			return s.compare(0, 2, "0x") == 0 &&
					s.size() > 2 &&
					s.find_first_not_of("0123456789abcdef", 2) == std::string::npos;
		}
		bool Utils::isBin(OmniaString bin)
		{
			_string s = bin.toLowerCase().cpp();
			return s.compare(0, 2, "0b") == 0 &&
					s.size() > 2 &&
					s.find_first_not_of("01", 2) == std::string::npos;
		}
		bool Utils::isInt(OmniaString str)
		{
			return Utils::isHex(str) || Utils::isBin(str) || str.isNumeric();
		}
		int32 Utils::strToInt(OmniaString str)
		{
			if (!Utils::isInt(str)) return 0;
			return (int32)strtol(str.trim().toLowerCase().c_str(), NULL, 0);
		}
		bool Utils::readFile(OmniaString fileName, std::vector<OmniaString>& lines)
		{
			_string line;
			std::ifstream file(fileName.cpp());
			if (file.fail()) return false;
			lines.clear();
			while (std::getline(file, line))
				lines.push_back(OmniaString(line));
			return true;
		}
		OmniaString Utils::replaceAllVarName(OmniaString str, OmniaString search, OmniaString replace)
		{
			_string s = str.trim().cpp();
			for (size_t pos = 0; ; pos += replace.length())
			{
				pos = s.find(search.cpp(), pos);
				if (pos == std::string::npos) break;
				if (pos + search.length() >= s.length() && pos == 0)
				{
					s.erase(pos, search.length());
					s.insert(pos, replace.cpp());
					continue;
				}
				else if (pos + search.length() >= s.length())
				{
					if (s[pos - 1] == '_' || 
						(s[pos - 1] >= '0' && s[pos - 1] <= '9') ||
						(s[pos - 1] >= 'A' && s[pos - 1] <= 'Z') ||
						(s[pos - 1] >= 'a' && s[pos - 1] <= 'z'))
						continue;
				}
				else
				{
					if (s[pos + search.length()] == '_' || 
						(s[pos + search.length()] >= '0' && s[pos + search.length()] <= '9') ||
						(s[pos + search.length()] >= 'A' && s[pos + search.length()] <= 'Z') ||
						(s[pos + search.length()] >= 'a' && s[pos + search.length()] <= 'z'))
						continue;
					if (s[pos - 1] == '_' || 
						(s[pos - 1] >= '0' && s[pos - 1] <= '9') ||
						(s[pos - 1] >= 'A' && s[pos - 1] <= 'Z') ||
						(s[pos - 1] >= 'a' && s[pos - 1] <= 'z'))
						continue;
				}
				s.erase(pos, search.length());
				s.insert(pos, replace.cpp());
			}
			return OmniaString(s);
		}
		OmniaString Utils::intToHexStr(word i, bool prefix)
		{
			char buff[5];
			sprintf(buff, "%04X", i);
			OmniaString m = "";
			for (uint16 j = 0; j < 4; j++)
				m = m.add(buff[j]);
			if (prefix) return OmniaString("0x").add(m);
			return m;
		}
		OmniaString Utils::intToBinStr(word i, bool prefix)
		{
			OmniaString m(std::bitset<sizeof(i) * 8>(i).to_string());
			if (prefix) return OmniaString("0b").add(m);
			return m;
		}
		OmniaString Utils::duplicateChar(unsigned char c, uint16 count)
		{
			OmniaString str = "";
			for (uint16 i = 0; i < count; i++)
				str = str.add(c);
			return str;
		}
		void Utils::printMemoryBlock(MemAddress start, MemAddress end, OmniaString text, OutputManager& out, MemAddress highlight, uint8 inst_len)
		{
			if (end == 0) return;
			uint16 w = 107;
			uint8 hcount = 16;
			StringBuilder tmp("|");
			out.newLine().print(Utils::duplicateChar('=', w)).newLine();
			if (text.trim() != "") tmp.add(text).add("  -  ");
			tmp.add("startAddr=").add(Utils::intToHexStr(start)).add("  endAddr=").add(Utils::intToHexStr(end - 1));
			out.print(tmp.get()).print(Utils::duplicateChar(' ', w - tmp.get().length() - 1)).print("|");
			out.newLine().print(Utils::duplicateChar('=', w)).newLine();
			out.print("|").print(Utils::intToHexStr(start)).print("| |").print((highlight != oasm_nullptr && start == highlight ? "<" : " "));
			OmniaString __lc = " ", __rc = " ";
			for (MemAddress i = start; i < end; i++)
			{
				__lc = (highlight != oasm_nullptr && highlight == i - 1 ? "<" : " ");
				__rc = (highlight != oasm_nullptr && highlight == i + 1 ? ">" : " ");
				out.print(Utils::intToHexStr(oasm::VirtualMachine::instance().getRAM().getAsReadOnly()[i].val(), false));
				if (((i - start) + 1) % hcount == 0 && i + 1 < end)
				{
					if (highlight != oasm_nullptr && highlight == i)
						out.print(">|").newLine().print("|").print(Utils::intToHexStr(i + 1)).print("| | ");
					else if (highlight != oasm_nullptr && inst_len != 0 && highlight + inst_len == i)
						out.print("*|").newLine().print("|").print(Utils::intToHexStr(i + 1)).print("| | ");
					else if (highlight != oasm_nullptr && highlight == i + 1)
						out.print(" |").newLine().print("|").print(Utils::intToHexStr(i + 1)).print("| |<");
					else
						out.print(" |").newLine().print("|").print(Utils::intToHexStr(i + 1)).print("| | ");
				}
				else if (((i - start) + 1) % hcount != 0)
				{
					if (highlight != oasm_nullptr && highlight == i) out.print("> ");
					else if (highlight != oasm_nullptr && inst_len != 0 && highlight + inst_len == i) out.print("* ");
					else if (highlight != oasm_nullptr && highlight == i + 1) out.print(" <");
					else out.print("  ");
				}
			}
			out.print(" |").newLine().print(Utils::duplicateChar('=', w));
		}

		void Utils::printRegisters(OutputManager& out)
		{
			OmniaString text = "Registers";
			uint8 hcount = 16;
			MemAddress start = 0x0000;
			MemAddress end = (MemAddress)eRegisters::Count;
			if (end == 0) return;
			uint16 w = 107;
			StringBuilder tmp("|");
			out.newLine().print(Utils::duplicateChar('=', w)).newLine();
			if (text.trim() != "") tmp.add(text).add("  -  ");
			tmp.add("startAddr=").add(Utils::intToHexStr(start)).add("  endAddr=").add(Utils::intToHexStr(end - 1));
			out.print(tmp.get()).print(Utils::duplicateChar(' ', w - tmp.get().length() - 1)).print("|");
			out.newLine().print(Utils::duplicateChar('=', w)).newLine();
			out.print("|").print(Utils::duplicateChar(' ', 10));
			out.print(" NP    CF    IP    ES    SP    RV    DR    FL    S1    S2    S3    S4    S5    S6    S7    S8  |").newLine();
			out.print("|").print(Utils::intToHexStr(start)).print("| | ");
			for (MemAddress i = start; i < 16; i++)
			{
				out.print(Utils::intToHexStr(oasm::VirtualMachine::instance().getREG().getAsReadOnly()[i].val(), false));
				if (((i - start) + 1) % hcount == 0 && i + 1 < end)
					out.print(" |");
				else if (((i - start) + 1) % hcount != 0)
					out.print("  ");
			}
			out.newLine().print("|").print(Utils::duplicateChar('-', w - 2)).print("|").newLine();
			out.print("|").print(Utils::intToHexStr(16)).print("| | ");
			for (MemAddress i = 16; i < end; i++)
			{
				out.print(Utils::intToHexStr(oasm::VirtualMachine::instance().getREG().getAsReadOnly()[i].val(), false));
				if (((i - start) + 1) % hcount == 0 && i + 1 < end)
					out.print(" |").newLine().print("|").print(Utils::intToHexStr(i + 1)).print("| | ");
				else if (((i - start) + 1) % hcount != 0)
					out.print("  ");
			}
			out.print(" |").newLine().print(Utils::duplicateChar('=', w));
		}

		OmniaString Utils::mapInstruction(eInstructionSet __inst)
		{
			switch (__inst)
			{
				case eInstructionSet::no_op: 	return "no_op";
				case eInstructionSet::req: 		return "req";
				case eInstructionSet::end: 		return "end";
				case eInstructionSet::reserve: 	return "reserve";
				case eInstructionSet::free_s: 	return "free_s";

				case eInstructionSet::mem: 		return "mem";
				case eInstructionSet::mem_m: 	return "mem_m";
				case eInstructionSet::push: 	return "push";
				case eInstructionSet::push_m: 	return "push_m";
				case eInstructionSet::pop: 		return "pop";
				case eInstructionSet::pop_m: 	return "pop_m";
				case eInstructionSet::pop_r: 	return "pop_r";
				case eInstructionSet::pop_r_m: 	return "pop_r_m";

				case eInstructionSet::lda_str: 	return "lda_str";
				case eInstructionSet::str_cpy: 	return "str_cpy";
				case eInstructionSet::add_str: 	return "add_str";

				case eInstructionSet::inc: 		return "inc";
				case eInstructionSet::inc_m: 	return "inc_m";
				case eInstructionSet::dec: 		return "dec";
				case eInstructionSet::dec_m: 	return "dec_m";
				case eInstructionSet::add: 		return "add";
				case eInstructionSet::add_m: 	return "add_m";
				case eInstructionSet::sub: 		return "sub";
				case eInstructionSet::sub_m: 	return "sub_m";
				case eInstructionSet::mul: 		return "mul";
				case eInstructionSet::mul_m: 	return "mul_m";
				case eInstructionSet::div: 		return "div";
				case eInstructionSet::div_m: 	return "div_m";
				case eInstructionSet::cmp: 		return "cmp";
				case eInstructionSet::cmp_m: 	return "cmp_m";

				case eInstructionSet::jmp: 		return "jmp";
				case eInstructionSet::call: 	return "call";
				case eInstructionSet::ret: 		return "ret";
				case eInstructionSet::ret_m: 	return "ret_m";

				case eInstructionSet::cmd: 		return "cmd";
				case eInstructionSet::flg: 		return "flg";
				case eInstructionSet::flg_m: 	return "flg_m";

				case eInstructionSet::alloc: 	return "alloc";
				case eInstructionSet::free: 	return "free";
				case eInstructionSet::realloc: 	return "realloc";

				case eInstructionSet::_and: 	return "and";
				case eInstructionSet::and_m: 	return "and_m";
				case eInstructionSet::_or: 		return "or";
				case eInstructionSet::or_m: 	return "or_m";
				case eInstructionSet::_not: 	return "not";
				case eInstructionSet::not_m: 	return "not_m";
				case eInstructionSet::bit: 		return "bit";
				case eInstructionSet::mask: 	return "mask";

				default: 						return "INVALID_OP_CODE";
			}
			return "INVALID_OP_CODE";
		}

		OmniaString Utils::mapAddressingMode(eAddressingModes __mode)
		{
			switch (__mode)
			{
				case eAddressingModes::ConstToAddr: 		return "Addr-Const";
				case eAddressingModes::ConstToPtr: 			return "Ptr-Const";
				case eAddressingModes::ConstToReg: 			return "Reg-Const";
				case eAddressingModes::ConstToRegPtr: 		return "RegPtr-Const";

				case eAddressingModes::AddrToAddr: 			return "Addr-Addr";
				case eAddressingModes::AddrToPtr: 			return "Ptr-Addr";
				case eAddressingModes::AddrToReg: 			return "Reg-Addr";
				case eAddressingModes::AddrToRegPtr: 		return "RegPtr-Addr";

				case eAddressingModes::PtrToAddr: 			return "Addr-Ptr";
				case eAddressingModes::PtrToPtr: 			return "Ptr-Ptr";
				case eAddressingModes::PtrToReg: 			return "Reg-Ptr";
				case eAddressingModes::PtrToRegPtr: 		return "RegPtr-Ptr";

				case eAddressingModes::RefToAddr: 			return "Addr-Ref";
				case eAddressingModes::RefToPtr: 			return "Ptr-Ref";
				case eAddressingModes::RefToReg: 			return "Reg-Ref";
				case eAddressingModes::RefToRegPtr: 		return "RegPtr-Ref";

				case eAddressingModes::RegPtrToAddr: 		return "Addr-RegPtr";
				case eAddressingModes::RegPtrToPtr: 		return "Ptr-RegPtr";
				case eAddressingModes::RegPtrToReg: 		return "Reg-RegPtr";
				case eAddressingModes::RegPtrToRegPtr: 		return "RegPtr-RegPtr";

				case eAddressingModes::RegToAddr: 			return "Addr-Reg";
				case eAddressingModes::RegToPtr: 			return "Ptr-Reg";
				case eAddressingModes::RegToReg: 			return "Reg-Reg";
				case eAddressingModes::RegToRegPtr: 		return "RegPtr-Reg";

				case eAddressingModes::SingleOp_const: 		return "Const (SingleOP)";
				case eAddressingModes::SingleOp_addr: 		return "Addr (SingleOP)";
				case eAddressingModes::SingleOp_reg: 		return "Reg (SingleOP)";
				case eAddressingModes::SingleOp_ptr: 		return "Ptr (SingleOP)";
				case eAddressingModes::SingleOp_regPtr: 	return "RegPtr (SingleOP)";
				case eAddressingModes::SingleOp_ref: 		return "Ref (SingleOP)";

				case eAddressingModes::ConstConst: 			return "Const-Const";
				case eAddressingModes::ConstAddr: 			return "Const-Addr";
				case eAddressingModes::ConstPtr: 			return "Const-Ptr";
				case eAddressingModes::ConstRef: 			return "Const-Ref";
				case eAddressingModes::ConstRegPtr:			return "Const-RegPtr";
				case eAddressingModes::ConstReg:			return "Const-Reg";

				default: 									return "NO_ADDR_MODE";
			}
			return "NO_ADDR_MODE";
		}

        OmniaString Utils::mapMaskParam(word __m_param)
		{
			switch (__m_param)
			{
				case (word)eAddressingModes::b_2ByteMode_mask: 		return "Normal-mode";
				case (word)eAddressingModes::b_LSB_LSB_mask: 		return "LSB, LSB";
				case (word)eAddressingModes::b_LSB_MSB_mask: 		return "LSB, MSB";
				case (word)eAddressingModes::b_MSB_LSB_mask: 		return "MSB, LSB";
				case (word)eAddressingModes::b_MSB_MSB_mask: 		return "MSB, MSB";
				case (word)eAddressingModes::b_SingleOp_LSB_mask: 	return "LSB (SingleOP)";
				case (word)eAddressingModes::b_SingleOp_MSB_mask: 	return "MSB (SingleOP)";
				case (word)eAddressingModes::b_2B_LSB_mask: 		return "2B, LSB";
				case (word)eAddressingModes::b_2B_MSB_mask: 		return "2B, MSB";
				case (word)eAddressingModes::b_LSB_2B_mask: 		return "LSB, 2B";
				case (word)eAddressingModes::b_MSB_2B_mask: 		return "MSB, 2B";
				default: 											return "INVALID_MASK_PARAM";
			}
			return "INVALID_MASK_PARAM";
		}

		OmniaString Utils::mapRegister(word __reg)
		{
			switch (__reg)
			{
				case (word)eRegisters::CF: return "CF";
				case (word)eRegisters::DR: return "DR";
				case (word)eRegisters::ES: return "ES";
				case (word)eRegisters::FL: return "FL";
				case (word)eRegisters::IP: return "IP";
				case (word)eRegisters::NP: return "NP";
				case (word)eRegisters::RV: return "RV";
				case (word)eRegisters::SP: return "SP";
				
				case (word)eRegisters::S1: return "S1";
				case (word)eRegisters::S2: return "S2";
				case (word)eRegisters::S3: return "S3";
				case (word)eRegisters::S4: return "S4";
				case (word)eRegisters::S5: return "S5";
				case (word)eRegisters::S6: return "S6";
				case (word)eRegisters::S7: return "S7";
				case (word)eRegisters::S8: return "S8";
				
				case (word)eRegisters::R0: return "R0";
				case (word)eRegisters::R1: return "R1";
				case (word)eRegisters::R2: return "R2";
				case (word)eRegisters::R3: return "R3";
				case (word)eRegisters::R4: return "R4";
				case (word)eRegisters::R5: return "R5";
				case (word)eRegisters::R6: return "R6";
				case (word)eRegisters::R7: return "R7";
				case (word)eRegisters::R8: return "R8";
				case (word)eRegisters::R9: return "R9";
				case (word)eRegisters::R10: return "R10";
				case (word)eRegisters::R11: return "R11";
				case (word)eRegisters::R12: return "R12";
				case (word)eRegisters::R13: return "R13";
				case (word)eRegisters::R14: return "R14";
				case (word)eRegisters::R15: return "R15";
				case (word)eRegisters::R16: return "R16";
				case (word)eRegisters::R17: return "R17";
				case (word)eRegisters::R18: return "R18";
				case (word)eRegisters::R19: return "R19";
				case (word)eRegisters::R20: return "R20";
				case (word)eRegisters::R21: return "R21";
				case (word)eRegisters::R22: return "R22";
				case (word)eRegisters::R23: return "R23";
				case (word)eRegisters::R24: return "R24";
				case (word)eRegisters::R25: return "R25";
				case (word)eRegisters::R26: return "R26";
				case (word)eRegisters::R27: return "R27";
				case (word)eRegisters::R28: return "R28";
				case (word)eRegisters::R29: return "R29";
				case (word)eRegisters::R30: return "R30";
				case (word)eRegisters::R31: return "R31";
				default: return Utils::intToHexStr(__reg, false);
			}
			return "NP";
		}

		void Utils::get_terminal_size(int& width, int& height)
		{
			#if defined(_WIN32)
				CONSOLE_SCREEN_BUFFER_INFO csbi;
				GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
				width = (int)(csbi.srWindow.Right-csbi.srWindow.Left+1);
				height = (int)(csbi.srWindow.Bottom-csbi.srWindow.Top+1);
			#elif defined(__linux__)
				struct winsize w;
				ioctl(fileno(stdout), TIOCGWINSZ, &w);
				width = (int)(w.ws_col);
				height = (int)(w.ws_row);
			#endif // Windows/Linux
		}
	}
}