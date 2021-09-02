#ifndef __DEBUGGER_HPP__
#define __DEBUGGER_HPP__

#include "Common.hpp"
#include "IOManager.hpp"

namespace Omnia
{
	namespace oasm
	{
		class Debugger : public IOReciever, public ErrorReciever
		{
			public: struct tRegChangeTracker
			{
				uint8 changeTime;
				eRegisters reg;
				BitEditor oldValue;

				tRegChangeTracker(eRegisters reg = eRegisters::NP);
				void updadte(void);
			};
			public:
				inline static Debugger& instance(void) { return *Debugger::s_instance; }
				int64 run(int argc, char** argv);
				bool loadSymTableFromFile(OmniaString __sym_table_file_path);
				bool printSourceCode(uint32 __start, uint32 __line_count, MemAddress __highlight = 0xFFFF, word __line_w = 100);
				uint32 findCurrentLine(MemAddress __off_ip);
				bool printInstructionLine(std::pair<MemAddress, OmniaString> __line, word __line_w, bool __hl, bool __rhl);
				void printTitle(OmniaString __title, word __line_length);
				void setColorFromTimeGradient(OutputManager& out, uint8 __tc);
				void printMemoryBlock(TMemoryList_c& __mem, MemAddress start, uint8 rows, OmniaString title, int32 __line_length);

			private:
				std::map<_string, MemAddress> m_labels;
				OmniaString m_cmd_input;
				BufferedOutput m_vm_buff;
				static Debugger* s_instance;
				SymbolTable m_sym_table;
				bool m_use_sym_table;
				OmniaString m_prompt;
				std::vector<OmniaString> m_decompiledCode;
				bool m_decompiled;
				uint32 m_currentSourceLine;
				std::vector<tRegChangeTracker> m_regChangeTable;
		};

		inline Debugger* Debugger::s_instance = new Debugger();
	}
}

#endif