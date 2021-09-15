#include "Common.hpp"
#include "Debugger.hpp"

namespace Omnia
{
	namespace common
	{
		Process::Process(void)
		{
			m_stackAddr = oasm_nullptr;
			m_heapAddr = oasm_nullptr;
			m_codeAddr = oasm_nullptr;
			m_heapAllocated = false,
			m_stackAllocated = false;
			m_codeAllocated = false;
			m_codeSize = 0;
			m_stackSize = 0;
			m_heapSize = 0;
			m_processFinished = false;
		}




		bool ECM::addHandler(word code, ExtComHandler& ech)
		{
			if (hasHandler(code)) return false;
			m_comHandlers[code] = &ech;
			return true;
		}

		bool ECM::hasHandler(word code)
		{
			return m_comHandlers.count(code) != 0;
		}

		bool ECM::execHandler(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
		{
			if (!hasHandler(code)) return false;
			outData = BitEditor((word)0);
			return m_comHandlers[code]->handleCommand(code, param, iomgr, outData);
		}




		void SR_CallTree::call(OmniaString __lbl_name)
		{
			tCallData __cd;
			__cd.space =  Utils::duplicateChar(' ', currentTab++ * 2);
			__cd.inst = "call ";
			__cd.label = __lbl_name;
			__cd.indent = currentTab - 1;
			__cd.closed = false;
			__cd.tickCounter = 0;
			__cd.setID(SR_CallTree::s_next_id++);
			labelStack.push_back(std::pair<uint32, OmniaString>(__cd.getID(), __lbl_name));
			callList.push_back(__cd);
		}

		void SR_CallTree::ret(void)
		{
			if (currentTab == 0) return;
			tCallData __cd;
			__cd.space = Utils::duplicateChar(' ', --currentTab * 2);
			__cd.inst = "ret  ";
			__cd.label = labelStack[labelStack.size() - 1].second;
			__cd.setID(labelStack[labelStack.size() - 1].first);
			for (auto& __call : callList)
			{
				if (__call.getID() == __cd.getID())
				{
					__call.closed = true;
					break;
				}
			}
			__cd.closed = true;
			STDVEC_REMOVE(labelStack, labelStack.size() - 1);
			__cd.indent = currentTab;
			callList.push_back(__cd);
		}

		bool SR_CallTree::getCurrentCall(OmniaString& outLabelName)
		{
			if (labelStack.size() == 0) return false;
			outLabelName = labelStack[labelStack.size() - 1].second;
			return true;
		}

		void SR_CallTree::print(OutputManager& out, word __line_w, word __lines)
		{
			if (callList.size() == 0) return;
			Omnia::oasm::Debugger::instance().printTitle("SUB-ROUTINE CALL TREE", __line_w);
			out.tc_reset();
			StringBuilder __sb;
			word __right_in_border_col = 80;
			if (__lines == 0) __lines = callList.size();
			uint16 __start = 0, __end = 0;
			bool __blank_space = false;
			if (__lines > callList.size())
			{
				__start = 0;
				__end = callList.size() - 1;
				__blank_space = true;
			}
			else
			{
				__start = callList.size() - __lines;
				__end = callList.size() - 1;
			}
			for (uint16 __i = __start; __i <= __end; __i++)
			{
				auto& __call = callList[__i];
				__sb = StringBuilder();
				out.fc_blue().bc_white();
				__sb.add(" ");
				out.print("│");
				out.tc_reset();
				__sb.add(" ");
				out.print(" ");
				out.fc_white();
				for (uint8 i = 1; i <= __call.indent; i++)
				{
					__sb.add("  ");
					out.print("│ ");
				}
				if (__call.inst == "call ")
				{
					__sb.add(" ");
					out.print("┌").fc_green();
				}
				else if (__call.inst == "ret  ")
				{
					__sb.add(" ");
					out.print("└").fc_red();
				}
				__sb.add(__call.inst);
				out.print(__call.inst);
				out.fc_brightCyan();
				if (__call.inst == "call ")
					out.fc_brightCyan();
				else if (__call.inst == "ret  ")
					out.fc_yellow();
				__sb.add(__call.label);
				out.print(__call.label);
				
				if (__call.inst == "call ") out.fc_white();
				else out.fc_brightGrey();
				//out.fc_brightGrey();
				out.print(Utils::duplicateChar('.', __right_in_border_col - __sb.get().length()));
				__sb.add(Utils::duplicateChar(' ', __right_in_border_col - __sb.get().length()));
				out.fc_blue().bc_white();
				__sb.add(" ");
				out.print("│");
				out.tc_reset();

				out.bc_brightYellow().fc_red();
				__sb.add(" ");
				out.print("(");


				//ID
				out.fc_grey();
				__sb.add("call-ID");
				out.print("call-ID");
				out.fc_red();
				__sb.add(" ");
				out.print("=");
				if (__call.inst == "call ") out.fc_green();
				else out.fc_red();
				__sb.add(Utils::intToHexStr((word)__call.getID()));
				out.print(Utils::intToHexStr((word)__call.getID()));

				if (__call.inst == "call ")
				{
					out.fc_red();
					__sb.add("  ");
					out.print(", ");


					//Tick-count
					out.fc_grey();
					__sb.add("cycle-count");
					out.print("cycle-count");
					out.fc_red();
					__sb.add(" ");
					out.print("=");
					out.fc_blue();
					__sb.add((int32)__call.tickCounter);
					out.print((int32)__call.tickCounter);
				}
				out.fc_red();
				__sb.add(" ");
				out.print(")");
				out.bc_brightYellow();
				out.print(Utils::duplicateChar(' ', __line_w - __sb.get().length() - 1));
				out.fc_blue().bc_white();
				out.print("│");
				out.tc_reset();
				out.newLine();
			}
			if (__blank_space)
			{
				for (uint16 __i = 0; __i < __lines - callList.size(); __i++)
				{
					out.fc_blue().bc_white();
					out.print("│");
					out.print(Utils::duplicateChar(' ', __line_w - 2));
					out.fc_blue().bc_white();
					out.print("│");
					out.newLine();
				}
			}
			out.fc_blue().bc_white();
			out.print("│");
			out.bc_magenta().fc_blue();
			OmniaString __c_trace_txt = " Current call trace: ";
			out.print(__c_trace_txt);
			word __cur = __c_trace_txt.length();
			word __depth = 0;
			out.bc_blue();
			out.print(" ");
			__cur++;
			for (auto& __call : labelStack)
			{
				out.fc_brightYellow();
				out.print(__call.second);
				__cur += __call.second.length();
				if (++__depth < labelStack.size())
				{
					out.fc_magenta();
					out.print("/");
					__cur++;
				}
			}
			out.print(Utils::duplicateChar(' ', __line_w - __cur - 2));
			out.fc_blue().bc_white();
			out.print("│");
			out.newLine();
			out.tc_reset();
		}

		void SR_CallTree::tick(void)
		{
			for (auto& __call : callList)
			{
				if (!__call.closed && __call.inst == "call ") __call.tickCounter++;
			}
		}




		bool SymbolTable::isLabel(MemAddress __addr, OmniaString& outLabel)
		{
			if (m_labels.count(__addr) != 0)
			{
				outLabel = m_labels[__addr];
				return true;
			}
			return false;
		}

		bool SymbolTable::isLabel(OmniaString __sym)
		{
			for (auto& __lbl : m_labels)
			{
				if (__lbl.second == __sym) return true;
			}
			return false;
		}

		bool SymbolTable::isReserve(MemAddress __addr, OmniaString& outVarName)
		{
			if (m_reserves.count(__addr) != 0)
			{
				outVarName = m_reserves[__addr];
				return true;
			}
			return false;
		}

		bool SymbolTable::isReserve(OmniaString __sym)
		{
			for (auto& __res : m_reserves)
			{
				if (__res.second == __sym) return true;
			}
			return false;
		}



		
		ErrorCode CompileErrorReciever::printError(ErrorCode __err, OutputManager& out, OmniaString __extra_info, OmniaString __line, OmniaString __file, int32 __line_number, bool __print_end_line)
		{
			bool print_borders = (__extra_info.trim() != "" || __line.trim() != "" || __file.trim() != "" || __line_number > 0);
			int32 tx, ty;
			Utils::get_terminal_size(tx, ty);
			OmniaString __str_err = Utils::intToHexStr((word)__err);
			if (print_borders)
				Utils::printTitle("ERROR", out, tx);
			out.fc_red().print("*** Code ").print(__str_err).print(": ");
			Utils::message(__error_map[__err], out, eMsgType::Error);
			if (__extra_info.trim() != "")
				out.fc_brightGrey().print("Token:  ").fc_brightCyan().print(__extra_info).newLine();
			if (__file.trim() != "")
				out.fc_brightWhite().print("In File ").fc_magenta().print(__file).newLine();
			if (__line_number > 0)
				out.fc_brightGrey().print("    Line:  ").fc_cyan().print((int32)__line_number);
			if (__line.trim() != "")
				out.print("    ").fc_brightWhite().print(__line).newLine();
			if (__print_end_line && print_borders)
				out.fc_brightGrey().print(Utils::duplicateChar('-', tx)).newLine();
			out.tc_reset();
			return __err;
		}





		void ErrorReciever::pushError(ErrorCode __err_code, OutputManager& out, OmniaString __extra_text, MemAddress __addr, word __op_code)
		{
			if (m_callback != nullptr)
			{
				m_callback->pushError(__err_code, out, __extra_text, __addr, __op_code);
				return;
			}
			m_errorQueue.push_back(__err_code);
			if (Flags::isset(FLG__PRINT_ERROR_ON_PUSH))
			{
				OmniaString __err_text = __error_map[__err_code];
				out.print(Utils::duplicateChar('=', 50)).newLine();
				out.print("Error ").print(Utils::intToHexStr(__err_code));
				out.print(": ").print(__err_text).newLine();
				if (__extra_text.trim() == "" && __addr == oasm_nullptr && __op_code == (word)eInstructionSet::no_op)
				{
					out.print(Utils::duplicateChar('=', 50)).newLine();
					return;
				}
				out.print("Extra info: ").newLine();
				if (__extra_text.trim() != "")
					out.tab().print(__extra_text).newLine();
				if (__addr != oasm_nullptr)
				{
					out.tab().print("Address: ").print(Utils::intToHexStr(__addr));
					if (__addr < D__HEAP_SPACE_START)
						out.print(" (-CODE-)").newLine();
					else if (__addr < D__STACK_SPACE_START)
						out.print(" (-HEAP-)").newLine();
					else if (__addr < D__LIB_SPACE_START)
						out.print(" (-STACK-)").newLine();
					else if (__addr < D__MEMORY_SIZE)
						out.print(" (-LIBRARY-)").newLine();
				}
				if (__op_code != (word)eInstructionSet::no_op)
					out.tab().print("Instruction: ").print(Utils::intToHexStr(__op_code)).print("(-").print(Utils::mapInstruction((eInstructionSet)__op_code)).print("-)").newLine();
				out.print(Utils::duplicateChar('=', 50)).newLine();
			}
		}

		ErrorCode ErrorReciever::popError(void)
		{
			if (__empty()) return D__NO_ERROR;
			ErrorCode __tmp = m_errorQueue[m_errorQueue.size() - 1];
			STDVEC_REMOVE(m_errorQueue, m_errorQueue.size() - 1);
			return __tmp;
		}

		std::vector<ErrorCode> ErrorReciever::flushErrorQueue(void)
		{
			std::vector<ErrorCode> __tmp = m_errorQueue;
			m_errorQueue.clear();
			return __tmp;
		}
	}
}