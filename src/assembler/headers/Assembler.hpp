#ifndef __ASSEMBLER_HPP__
#define __ASSEMBLER_HPP__

#include "Common.hpp"
#include "IOManager.hpp"

namespace Omnia
{
	namespace oasm
	{
        class PendingLabel
        {
            public:
                inline PendingLabel(OmniaString n, MemAddress a, uint32 l, OmniaString ll) { name = n; addr = a; lineN = l; line = ll; pending = true; }

                OmniaString name;
                MemAddress addr;
                bool pending;
                uint32 lineN;
                OmniaString line;
        };

        class PreProcessorOptions
        {
            public:
                inline PreProcessorOptions(void)
                {
                    passes = 2;
                }

                uint16 passes;
        };

        class Macro : public Validable
        {
            public:
                inline Macro(void) { invalidate(); }
                Macro(OmniaString line);
                OmniaString expand(OmniaString line);

            
            public:
                std::vector<OmniaString> params;
                OmniaString expansion;
                OmniaString name;
        };

        class PreProcessor : public IOReciever, public ErrorReciever
        {
            public:
                inline static PreProcessor& instance(void) { return s_instance; }
                std::vector<OmniaString> open(OmniaString fileName, PreProcessorOptions options = PreProcessorOptions());
                inline bool hasAlias(OmniaString alias) { return (m_aliases.count(alias.cpp()) != 0); }
                inline bool hasReserved(OmniaString res) { return (m_reserves.count(res.cpp()) != 0); }
                inline bool hasDefine(OmniaString def) { return (m_defines.count(def.cpp()) != 0); }

            private:
                inline PreProcessor(void) {  }

            private:
                std::vector<OmniaString> process(std::vector<OmniaString> lines, PreProcessorOptions options);
                std::vector<OmniaString> resolveIncludes(std::vector<OmniaString> mainFile, OmniaString curFile);
                std::vector<OmniaString> resolveIncludes_r(std::vector<OmniaString> mainFile, OmniaString curFile);
                std::vector<OmniaString> removeComments(std::vector<OmniaString> lines);
                std::vector<OmniaString> resolveAliases(std::vector<OmniaString> lines);
                std::vector<OmniaString> resolveMacros(std::vector<OmniaString> lines);
				std::vector<OmniaString> resolveCommandDirective(std::vector<OmniaString> lines);
				std::vector<OmniaString> resolveDataDirective(std::vector<OmniaString> lines);
				std::vector<OmniaString> resolveDefines(std::vector<OmniaString> lines);

                void error(ePreProcessorErrors err, OmniaString msg, bool skipFileInfo = false);

            private:
                OmniaString _line;
                uint32 lineNumber;
                OmniaString currentFile;
				word m_reserveCount;
                std::vector<OmniaString> m_includeGuards;
				std::vector<OmniaString> m_dataSection;
                std::map<_string, _string> m_aliases;
				std::map<_string, MemAddress> m_reserves;
				std::vector<OmniaString> m_struct_defs;
				std::map<_string, std::pair<OmniaString, bool>> m_defines;
				std::vector<bool> m_def_stack;
				bool m_skip_rest_of_branches;
                PreProcessorOptions m_options;
                std::vector<Macro> m_macros;
				word m_nextTopInst;

            public:
                static PreProcessor s_instance;
				SymbolTable m_symTable;

				friend class Assembler;

        };

        inline PreProcessor PreProcessor::s_instance;
		
		class Assembler : public IOReciever, public ErrorReciever
		{
			public:
				inline static Assembler& instance(void) { return *Assembler::s_instance; }
				int64 run(int argc, char** argv);

				bool createExecutableFile(OmniaString __outputFile, TMemoryList __program);
				bool createDebugTableFile(OmniaString __outputFile);
				TMemoryList assemble(std::vector<OmniaString>& __source);
				TMemoryList assemble(OmniaString __source_file_path);
				std::vector<OmniaString> resolveKeyWords(std::vector<OmniaString> lines);
                inline static bool isKeyword(OmniaString __kw, word& outKeyword)
				{
					if (Assembler::m_keyWords.count(__kw.cpp()) != 0)
					{
						outKeyword = m_keyWords[__kw.cpp()];
						return true;
					}
					return false;
				}
				inline bool isLabel(OmniaString __label, MemAddress& outAddr)
				{
					if (m_labels.count(__label.cpp()) != 0)
					{
						outAddr = m_labels[__label.cpp()];
						return true;
					}
					return false;
				}

			private:
				std::map<_string, MemAddress> m_labels;

				bool p__dbg_symbol_table;
				bool p__dbg_save_code;
				OmniaString p__input_file_path;
				OmniaString p__output_file_path;
				OmniaString p__output_file_dbg_table;

				static Assembler* s_instance;

				inline static std::map<const _string, const word> m_keyWords = {
				//ADDRESSING MODES
					{ "Addr_Const", (word)eAddressingModes::ConstToAddr },
					{ "Ptr_Const", (word)eAddressingModes::ConstToPtr },
					{ "Reg_Const", (word)eAddressingModes::ConstToReg },
					{ "RegPtr_Const", (word)eAddressingModes::ConstToRegPtr },
					
					{ "Addr_Addr", (word)eAddressingModes::AddrToAddr },
					{ "Ptr_Addr", (word)eAddressingModes::AddrToPtr },
					{ "Reg_Addr", (word)eAddressingModes::AddrToReg },
					{ "RegPtr_Addr", (word)eAddressingModes::AddrToRegPtr },

					{ "Addr_Ptr", (word)eAddressingModes::PtrToAddr },
					{ "Ptr_Ptr", (word)eAddressingModes::PtrToPtr },
					{ "Reg_Ptr", (word)eAddressingModes::PtrToReg },
					{ "RegPtr_Ptr", (word)eAddressingModes::PtrToRegPtr },

					{ "Addr_Ref", (word)eAddressingModes::RefToAddr },
					{ "Ptr_Ref", (word)eAddressingModes::RefToPtr },
					{ "Reg_Ref", (word)eAddressingModes::RefToReg },
					{ "RegPtr_Ref", (word)eAddressingModes::RefToRegPtr },

					{ "Addr_RegPtr", (word)eAddressingModes::RegPtrToAddr },
					{ "Ptr_RegPtr", (word)eAddressingModes::RegPtrToPtr },
					{ "Reg_RegPtr", (word)eAddressingModes::RegPtrToReg },
					{ "RegPtr_RegPtr", (word)eAddressingModes::RegPtrToRegPtr },

					{ "Addr_Reg", (word)eAddressingModes::RegToAddr },
					{ "Ptr_Reg", (word)eAddressingModes::RegToPtr },
					{ "Reg_Reg", (word)eAddressingModes::RegToReg },
					{ "RegPtr_Reg", (word)eAddressingModes::RegToRegPtr },

					{ "Single_Const", (word)eAddressingModes::SingleOp_const },
					{ "Single_Addr", (word)eAddressingModes::SingleOp_addr },
					{ "Single_Reg", (word)eAddressingModes::SingleOp_reg },
					{ "Single_Ptr", (word)eAddressingModes::SingleOp_ptr },
					{ "Single_RegPtr", (word)eAddressingModes::SingleOp_regPtr },
					{ "Single_Ref", (word)eAddressingModes::SingleOp_ref },

					{ "Const_Const", (word)eAddressingModes::ConstConst },
					{ "Const_Addr", (word)eAddressingModes::ConstAddr },
					{ "Const_Ptr", (word)eAddressingModes::ConstPtr },
					{ "Const_Ref", (word)eAddressingModes::ConstRef },
					{ "Const_RegPtr", (word)eAddressingModes::ConstRegPtr },
					{ "Const_Reg", (word)eAddressingModes::ConstReg },


				//INSTRUCTIONS
					{ "no_op", (word)eInstructionSet::no_op },
					{ "req", (word)eInstructionSet::req },
					{ "end", (word)eInstructionSet::end },
					{ "reserve", (word)eInstructionSet::reserve },
					{ "free_s", (word)eInstructionSet::free_s },
					{ "alloc", (word)eInstructionSet::alloc },
					{ "free", (word)eInstructionSet::free },
					{ "realloc", (word)eInstructionSet::realloc },
					{ "cmd", (word)eInstructionSet::cmd },
					{ "flg", (word)eInstructionSet::flg },
					{ "flg_m", (word)eInstructionSet::flg_m },
					{ "offset", (word)eInstructionSet::offset },

					{ "mem", (word)eInstructionSet::mem },
					{ "mem_m", (word)eInstructionSet::mem_m },
					{ "push", (word)eInstructionSet::push },
					{ "push_m", (word)eInstructionSet::push_m },
					{ "pop", (word)eInstructionSet::pop },
					{ "pop_m", (word)eInstructionSet::pop_m },
					{ "pop_r", (word)eInstructionSet::pop_r },
					{ "pop_r_m", (word)eInstructionSet::pop_r_m },
					{ "lda_str", (word)eInstructionSet::lda_str },
					{ "str_cpy", (word)eInstructionSet::str_cpy },
					{ "add_str", (word)eInstructionSet::add_str },

					{ "inc", (word)eInstructionSet::inc },
					{ "inc_m", (word)eInstructionSet::inc_m },
					{ "dec", (word)eInstructionSet::dec },
					{ "dec_m", (word)eInstructionSet::dec_m },
					{ "add", (word)eInstructionSet::add },
					{ "add_m", (word)eInstructionSet::add_m },
					{ "sub", (word)eInstructionSet::sub },
					{ "sub_m", (word)eInstructionSet::sub_m },
					{ "mul", (word)eInstructionSet::mul },
					{ "mul_m", (word)eInstructionSet::mul_m },
					{ "div", (word)eInstructionSet::div },
					{ "div_m", (word)eInstructionSet::div_m },
					{ "cmp", (word)eInstructionSet::cmp },
					{ "cmp_m", (word)eInstructionSet::cmp_m },

					{ "jmp", (word)eInstructionSet::jmp },
					{ "call", (word)eInstructionSet::call },
					{ "ret", (word)eInstructionSet::ret },
					{ "ret_m", (word)eInstructionSet::ret_m },

					{ "and", (word)eInstructionSet::_and },
					{ "and_m", (word)eInstructionSet::and_m },
					{ "or", (word)eInstructionSet::_or },
					{ "or_m", (word)eInstructionSet::or_m },
					{ "not", (word)eInstructionSet::_not },
					{ "not_m", (word)eInstructionSet::not_m },
					{ "bit", (word)eInstructionSet::bit },
					{ "mask", (word)eInstructionSet::mask },


				//FLAGS
					{ "NO_FLAG", (word)eFlags::no_flag },

					{ "REQ_ALL", (word)eFlags::req_all },
					{ "REQ_STACK", (word)eFlags::req_stack },
					{ "REQ_HEAP", (word)eFlags::req_heap },

					{ "JMP_UNCONDITIONAL", (word)eFlags::jmp_unconditional },
					{ "JMP_GREATER", (word)eFlags::jmp_greater },
					{ "JMP_LESS", (word)eFlags::jmp_less },
					{ "JMP_EQUALS", (word)eFlags::jmp_equals },
					{ "JMP_NOT_EQUALS", (word)eFlags::jmp_not_eq },
					{ "JMP_GREATER_OR_EQUALS", (word)eFlags::jmp_greater_eq },
					{ "JMP_LESS_OR_EQUALS", (word)eFlags::jmp_less_eq },

					{ "ADD_CONST_STREAM_TO_STR", (word)eFlags::add_str_const_stream },
					{ "ADD_STR_PTR_TO_STR", (word)eFlags::add_str_str_ptr },
					{ "ADD_INT_TO_STR", (word)eFlags::add_str_int },
					{ "ADD_CHAR_TO_STR", (word)eFlags::add_str_char },


				//REGISTERS
					{ "NP", (word)eRegisters::NP },
					{ "CF", (word)eRegisters::CF },
					{ "IP", (word)eRegisters::IP },
					{ "ES", (word)eRegisters::ES },
					{ "SP", (word)eRegisters::SP },
					{ "RV", (word)eRegisters::RV },
					{ "DR", (word)eRegisters::DR },
					{ "FL", (word)eRegisters::FL },

					{ "S1", (word)eRegisters::S1 },
					{ "S2", (word)eRegisters::S2 },
					{ "S3", (word)eRegisters::S3 },
					{ "S4", (word)eRegisters::S4 },
					{ "S5", (word)eRegisters::S5 },
					{ "S6", (word)eRegisters::S6 },
					{ "S7", (word)eRegisters::S7 },
					{ "S8", (word)eRegisters::S8 },

					{ "R0", (word)eRegisters::R0 },
					{ "R1", (word)eRegisters::R1 },
					{ "R2", (word)eRegisters::R2 },
					{ "R3", (word)eRegisters::R3 },
					{ "R4", (word)eRegisters::R4 },
					{ "R5", (word)eRegisters::R5 },
					{ "R6", (word)eRegisters::R6 },
					{ "R7", (word)eRegisters::R7 },
					{ "R8", (word)eRegisters::R8 },
					{ "R9", (word)eRegisters::R9 },
					{ "R10", (word)eRegisters::R10 },
					{ "R11", (word)eRegisters::R11 },
					{ "R12", (word)eRegisters::R12 },
					{ "R13", (word)eRegisters::R13 },
					{ "R14", (word)eRegisters::R14 },
					{ "R15", (word)eRegisters::R15 },
					{ "R16", (word)eRegisters::R16 },
					{ "R17", (word)eRegisters::R17 },
					{ "R18", (word)eRegisters::R18 },
					{ "R19", (word)eRegisters::R19 },
					{ "R20", (word)eRegisters::R20 },
					{ "R21", (word)eRegisters::R21 },
					{ "R22", (word)eRegisters::R22 },
					{ "R23", (word)eRegisters::R23 },
					{ "R24", (word)eRegisters::R24 },
					{ "R25", (word)eRegisters::R25 },
					{ "R26", (word)eRegisters::R26 },
					{ "R27", (word)eRegisters::R27 },
					{ "R28", (word)eRegisters::R28 },
					{ "R29", (word)eRegisters::R29 },
					{ "R30", (word)eRegisters::R30 },
					{ "R31", (word)eRegisters::R31 },


				//COM CODES
					{ "PrintIntToConsole", (word)eComCodes::PrintIntToConsole },
					{ "PrintStringToConsole", (word)eComCodes::PrintStringToConsole },
					{ "PrintNewLineToConsole", (word)eComCodes::PrintNewLineToConsole },
					{ "StringLength", (word)eComCodes::StringLength },
					{ "ReadStringInput", (word)eComCodes::ReadStringInput },
					{ "ReadIntInput", (word)eComCodes::ReadIntInput },
					{ "PrintCharToConsole", (word)eComCodes::PrintCharToConsole },
					{ "Sleep", (word)eComCodes::Sleep },
					{ "GetRunningTime", (word)eComCodes::GetRunningTime },
					{ "TimeDiff", (word)eComCodes::TimeDiff },
					{ "TD_LOAD_TIME", (word)eComCodes::p_TimeDiff_load },
					{ "TD_GET_DIFF", (word)eComCodes::p_TimeDiff_load },

					{ "TU_SECONDS", (word)eTimeUnits::Seconds },
					{ "TU_MILLISECONDS", (word)eTimeUnits::Milliseconds },
					{ "TU_MICROSECONDS", (word)eTimeUnits::Microseconds },
					{ "TU_NANOSECONDS", (word)eTimeUnits::Nanoseconds },

				
				//CONSTANTS
					{ "True", 0xFFFF },
					{ "False", 0x0000 }
				};
		};

		inline Assembler* Assembler::s_instance = new Assembler();
	}
}

#endif