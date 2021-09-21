#ifndef __DEBUGGER_HPP__
#define __DEBUGGER_HPP__

#include "Common.hpp"
#include "IOManager.hpp"
#include "Interpreter.hpp"

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
				bool loaded;

				tRegChangeTracker(eRegisters reg = eRegisters::NP);
				void updadte(void);
			};
			public: struct tMemChangeTracker
			{
				uint8 changeTime;
				MemAddress addr;
				BitEditor oldValue;
				bool loaded;

				tMemChangeTracker(MemAddress addr);
				void updadte(void);
			};
			public: struct tErrorData
			{
				ErrorCode code;
				OmniaString extraText;
				MemAddress address;
				word instruction;

				inline tErrorData(void)
				{
					code = D__NO_ERROR;
					extraText = "";
					address = oasm_nullptr;
					instruction = (word)eInstructionSet::no_op;
				}
			};
			public:
				inline static Debugger& instance(void) { return *Debugger::s_instance; }
				int64 run(int argc, char** argv);

				bool loadSymTableFromFile(OmniaString __sym_table_file_path);
				uint32 findCurrentLine(MemAddress __off_ip);

				void setColorFromTimeGradient(OutputManager& out, uint8 __tc);
				
				bool printSourceCode(uint32 __start, uint32 __line_count, MemAddress __highlight = 0xFFFF);
				bool printInstructionLine(std::pair<MemAddress, OmniaString> __line, bool __hl, bool __rhl);
				void printTitle(OmniaString __title, word __line_length);
				void printSeparator(unsigned char c = '=');
				void fillRestOfLine(char __c, int8 __neg_offset = 0, bool __new_line = true);
				uint16 message(OmniaString __msg_text, eMsgType __type = eMsgType::Info, bool __log = true, bool __force_print = false, bool __new_line = true);

				void printScriptOutput(uint16 __line_count);
				void printError(uint32 __err_index = 0);
				void printMemoryBlock(MemAddress start, uint8 rows, OmniaString title);
				void printRegisters(void);
				void printSourceView(uint8 __lines);
				void printInfoView(void);
				void printLog(void);
				void printFullGui(Process& proc);

				void inputPrompt(void);
				void reset(void);

				bool topLevelPrompt(void);
				void runProcess(Process& proc);
				void stepMode(Process& proc);
				void draw(Process& proc);

				void pushError(ErrorCode __err_code, OutputManager& out, OmniaString __extra_text = "", MemAddress __addr = oasm_nullptr, word __op_code = (word)eInstructionSet::no_op);
				inline Debugger(void) : vm(VirtualMachine::instance()), ram(vm.getRAM()), reg(vm.getREG()), gpu(vm.getGPU()), cpu(vm.getCPU()) {  }

			private:
				VirtualMachine& vm;
				hw::RAM& ram;
				hw::REG& reg;
				hw::GPU& gpu;
				hw::CPU& cpu;
				int32 __line_length;
				int32 __console_height;
				OmniaString outPrompt;
				OmniaString p__input_file_path;
				OmniaString p__input_sym_table_path;
				std::vector<OmniaString> proc_out;
				std::vector<OmniaString> __proc_out_buffer;
				TMemoryList __dec_inst;
				uint32 __total_tick_count;
				std::vector<tErrorData> m_err_data;
				uint8 m_cursor;
				MemAddress __ip;
				MemAddress __off_ip;
				bool cmd__step;
				bool __skip_next_input_round;
				bool __done;
				bool m_error;
				bool m_force_quit;
				bool tick_res;
				bool exec_tick;
				bool __br_sig;
				bool __first_cmd_step;
				bool __add_br_on_end;
				bool m_use_sym_table;
				bool m_show_data;
				bool m_print_end_msg;
				bool m_allow_diff_version;
				bool m_skip_next_draw;
				bool m_disable_protected_mode;
				eDebuggerMode m_mode;
				Process* m_process;

				uint8 m_g_source_lines;
				uint8 m_g_out_lines;
				uint8 m_g_heap_lines;
				uint8 m_g_stack_lines;
				uint8 m_g_code_lines;
				uint8 m_g_lib_lines;
				uint8 m_g_tree_lines;
				uint8 m_g_log_lines;
				eGuiBlock m_gui_block_top;
				eGuiBlock m_gui_block_middle;
				eGuiBlock m_gui_block_bottom;
				eGuiBlock m_gui_block_extra;
				eGuiBlockPosition m_gui_error_on_block;
				eGuiBlockPosition m_gui_data_under;

				char** m_as_args;
				std::map<_string, MemAddress> m_labels;
				OmniaString m_cmd_input;
				BufferedOutput m_vm_buff;
				static Debugger* s_instance;
				SymbolTable m_sym_table;
				OmniaString m_prompt;
				std::vector<OmniaString> m_decompiledCode;
				uint32 m_currentSourceLine;
				std::vector<tRegChangeTracker> m_regChangeTable;
				std::vector<tMemChangeTracker> m_memChangeTable;
				std::vector<std::pair<eMsgType, OmniaString>> m_msg_log;
		};

		inline Debugger* Debugger::s_instance = new Debugger();
	}
}

#endif