#ifndef __INTERPRETER_HPP__
#define __INTERPRETER_HPP__

#include "Common.hpp"
#include "IOManager.hpp"
#include "Keyboard.hpp"

namespace Omnia
{
	namespace oasm
	{
		//-----------------------------------------------CMD HANDLERS-----------------------------------------------
		class EC_PrintChar_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_PrintString_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_PrintNewLine_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_PrintInt_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_Sleep_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_GetRunningTime_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_TimeDiff_cmd : public ExtComHandler
        {
            public:
				inline EC_TimeDiff_cmd(void) { m_time = Utils::getRunningTime_ms(); }
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
			
			private:
				unsigned long int m_time;
        };
		class EC_RefreshScreen_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_Draw_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_SetVideoMode_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_PlotChar_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_GetScreenW_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_GetScreenH_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_Random_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		class EC_GetAsyncKey_cmd : public ExtComHandler
        {
            public:
                bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);
        };
		//----------------------------------------------------------------------------------------------------------



		class Interpreter : public IOReciever, public ErrorReciever
		{
			public:
				inline static Interpreter& instance(void) { return *Interpreter::s_instance; }
				int64 run(int argc, char** argv);
				bool loadFromFile(OmniaString __oex_file, TMemoryList& outProgram);
				inline bool __is_dbg_call(void) { return p__debugger_call; }

			private:
				static Interpreter* s_instance;

				bool p__step_exec;
				bool p__print_memory;
				bool p__assemble;
				bool p__debugger_call;
				OmniaString p__input_file_path;

				Process proc;

				//Command handlers
				EC_PrintChar_cmd __ec_printChar_cmd;
				EC_PrintString_cmd __ec_prinString_cmd;
				EC_PrintNewLine_cmd __ec_prinSNewLine_cmd;
				EC_PrintInt_cmd __ec_printInt_cmd;
				EC_Sleep_cmd __ec_sleep_cmd;
				EC_GetRunningTime_cmd __ec_getRunningTime_cmd;
				EC_TimeDiff_cmd __ec_timeDiff_cmd;
				EC_RefreshScreen_cmd __ec_refreshScreen_cmd;
				EC_SetVideoMode_cmd __ec_setVideoMode_cmd;
				EC_PlotChar_cmd __ec_plotChar_cmd;
				EC_GetScreenW_cmd __ec_getScreenW_cmd;
				EC_GetScreenH_cmd __ec_getScreenH_cmd;
				EC_Draw_cmd __ec_draw_cmd;
				EC_Random_cmd __ec_random_cmd;
				EC_GetAsyncKey_cmd __ec_getAsyncKey_cmd;

				friend class Debugger;
		};

		inline Interpreter* Interpreter::s_instance = new Interpreter();

		namespace hw
		{
			class RAM;
			class REG;
			class GPU;
			
			class CPU : public ErrorReciever, public Protectable
			{
				public:
					inline CPU(void) { create(D__DEFAULT_IPC); }
					inline CPU(uint8 ipc) { create(ipc); }
					CPU& create(uint8 ipc);

					bool fetch(RAM& _ram, REG& _reg);
					bool decode(RAM& _ram, REG& _reg);
					bool execute(RAM& _ram, REG& _reg);
					void printMemory(OutputManager& out, word __stack_rows = 4, word __heap_rows = 4, word __code_rows = 8, bool __print_instruction_data = false);
					bool pushToStack(BitEditor __data);
					MemAddress offsetHeapAddress(MemAddress __local_addr);
					MemAddress offsetCodeAddress(MemAddress __local_addr);
					inline uint8 getIPC(void) { return m_ipc; }
					inline void setIPC(uint8 ipc) { m_ipc = ipc; }
					bool isBreakPoint(MemAddress __addr, uint8 __inst_size);

					bool clock_tick(void);

					inline bool __break_point_signal(void) { return m_break_point_signal; }
					inline MemAddress __break_point_address(void) { return m_break_point_addr; }

					inline void setStepExecution(bool __se) { m_step_execution = __se; }
					inline bool stepExecutionEnabled(void) { return m_step_execution; }
					inline void addBreakPoint(MemAddress __brp_addr) { m_break_points.push_back(__brp_addr); }
					inline MemAddress getLastInstructionAddr(void) { return m_old_pc_val; }
					inline TMemoryList getDecodedInstruction(void) { return m_decoded_inst; }
					inline eVideoModes getVideoMode(void) { return m_video_mode; }
					inline void setVideoMode(eVideoModes __vm) { m_video_mode = __vm; }

					void pushError(ErrorCode __err_code);

					inline static CPU& instance(void) { return *CPU::s_instance; }

				private:
					bool map_addr_mode(RAM& _ram, REG& _reg, eAddressingModes mode, BitEditor op1, BitEditor op2, BitEditor& outOp1, BitEditor& outOp2, bool& __reg_op1);
					void decode_addr_mode(void);
					BitEditor map_op2_to_m_param(BitEditor op2);
					BitEditor map_op1_to_m_param(BitEditor op1);
					BitEditor __logic_op(BitEditor op1, BitEditor op2, eInstructionSet inst);
					bool __next_single_heap_cell(MemAddress& outAddr);
					bool __free_single_heap_cell(MemAddress __local_addr);
					MemAddress __localize_real_addr(MemAddress __real_addr);

				private:
					uint8 						m_ipc;
					MemAddress 					m_old_pc_val;
					uint8 						m_inst_mode;
					bool						m_single_op_inst;
					bool						m_process_end;
					uint8						m_inst_size;
					word						m_m_param;
					eSingleAddrModes			m_op1_addr_mode;
					eSingleAddrModes			m_op2_addr_mode;
					bool						m_const_op1;
					bool						m_pop_r_flg;

					bool						m_step_execution;
					uint8						m_current_ipc;
					OmniaString					m_cmd_command;
					bool						m_variable_inst_size;
					word						m_heap_reserve_count;
					MemAddress					m_next_single_heap;
					std::vector<MemAddress>		m_break_points;
					bool						m_break_point_signal;
					MemAddress					m_break_point_addr;
					TMemoryList					m_decoded_inst;
					word						m_offset;
					eVideoModes					m_video_mode;

					word 						m_raw_1;
					word 						m_raw_2;
					word 						m_raw_3;
					word 						m_raw_4;
					word 						m_raw_5;
					word 						m_raw_6;

					eInstructionSet 			m_dec_opCode;
					eAddressingModes 			m_dec_addrMode;
					eFlags 						m_dec_flags;
					BitEditor 					m_dec_op1;
					BitEditor 					m_dec_op2;
					BitEditor 					m_dec_extra;
					BitEditor					m_dec_bitmask;

					ErrorCode 					m_err_1;
					ErrorCode 					m_err_2;
					ErrorCode 					m_err_3;
					ErrorCode 					m_err_4;
					ErrorCode 					m_err_5;
					ErrorCode 					m_err_6;

					static CPU* 				s_instance;

					friend class Debugger;
			};
			inline CPU* CPU::s_instance = new CPU();

			class GPU : public ErrorReciever, public Protectable
			{
				public:
					GPU(void);
					inline static GPU& instance(void) { return *GPU::s_instance; }
					bool clock_tick(void);
					void plotChar(word __x, word __y, char __c, eOasmColors __bg_col, eOasmColors __fg_col);
					inline word getScreenW(void) { return (word)m_screen_x; }
					inline word getScreenH(void) { return (word)m_screen_y; }
					void clearScreenBuffer(void);
					void mapBgColor(eOasmColors __bg);
					void mapFgColor(eOasmColors __fg);

				private:
					TMemoryList m_vram;
					int32 m_screen_x;
					int32 m_screen_y;
					MemAddress m_screen_buffer_addr;

					static GPU* s_instance;
					friend class Debugger;
			};
			inline GPU* GPU::s_instance = new GPU();

			class RAM : public ErrorReciever, public Protectable
			{
				public: class MemCell
				{
					public:
						inline MemCell(void) : proc(&Process::invalidProc()), type(eMemCellType::Normal), state(eMemState::Free), flag(eMemCellFlag::NoFlag) {  }

					public:
						Process* proc;
						eMemCellType type;
						eMemState state;
						eMemCellFlag flag;
				};
				public:
					inline RAM(void) { create((uint16)D__MEMORY_SIZE); }
					inline RAM(uint16 size) { create(size); }
					RAM& create(uint16 size);

					bool request(word size, MemAddress& outAddr, eMemCellType type, Process& proc);
					bool requestPtr(word size, MemAddress& outAddr);
					bool freePtr(MemAddress addr);

					bool read(MemAddress addr, BitEditor& outData);
					bool read_m(MemAddress addr, BitEditor& outData, word __m_param);
					bool write(MemAddress addr, BitEditor data);
					bool write_m(MemAddress addr, BitEditor data, word __m_param);
					bool writeToPtr(MemAddress ptr, std::vector<BitEditor> data, bool __safe = false);
					bool readFromPtr(MemAddress ptr, std::vector<BitEditor>& outData, bool __safe = false);
					bool readStringFromStream(MemAddress __stream_addr, OmniaString& outStr);
					bool writeStringToStream(MemAddress __stream_addr, OmniaString str);
					inline TMemoryList_c& getAsReadOnly(void) { return m_memory; }
					inline TMemCellList_c& getReadOnlyMemState(void) { return m_memCells; }

					//TODO: TEMP
					inline void set(Process& proc, MemAddress addr, eMemCellType type = eMemCellType::Normal, eMemState state = eMemState::Allocated, eMemCellFlag flag = eMemCellFlag::NoFlag)
					{
						m_memCells[addr].proc = &proc;
						m_memCells[addr].type = type;
						m_memCells[addr].state = state;
						m_memCells[addr].flag = flag;
					}

					inline static RAM& instance(void) { return *RAM::s_instance; }

					friend class CPU;
					friend class Debugger;

				private:
					MemAddress requestFrom(MemAddress start, word size, MemAddress max, bool __alloc = true);
					bool allocate(Process& proc, MemAddress start, word size, word __start_value = 0xFFFF, eMemCellFlag __ptr_flag = eMemCellFlag::HeapBlockStart);

				private:
					TMemoryList m_memory;
					TMemCellList m_memCells;

					static RAM* s_instance;
			};
			inline RAM* RAM::s_instance = new RAM();

			class REG : public ErrorReciever, public Protectable
			{
				public:
					inline REG(void) { create(); }
					REG& create(void);

					inline BitEditor NP(void) { return m_regs[(word)eRegisters::NP]; }
					inline BitEditor RV(void) { return m_regs[(word)eRegisters::RV]; }
					inline BitEditor CF(void) { return m_regs[(word)eRegisters::CF]; }
					inline BitEditor IP(void) { return m_regs[(word)eRegisters::IP]; }
					inline BitEditor DR(void) { return m_regs[(word)eRegisters::DR]; }
					inline BitEditor ES(void) { return m_regs[(word)eRegisters::ES]; }
					inline BitEditor FL(void) { return m_regs[(word)eRegisters::FL]; }
					inline BitEditor SP(void) { return m_regs[(word)eRegisters::SP]; }

					inline BitEditor S1(void) { return m_regs[(word)eRegisters::S1]; }
					inline BitEditor S2(void) { return m_regs[(word)eRegisters::S2]; }
					inline BitEditor S3(void) { return m_regs[(word)eRegisters::S3]; }
					inline BitEditor S4(void) { return m_regs[(word)eRegisters::S4]; }
					inline BitEditor S5(void) { return m_regs[(word)eRegisters::S5]; }
					inline BitEditor S6(void) { return m_regs[(word)eRegisters::S6]; }
					inline BitEditor S7(void) { return m_regs[(word)eRegisters::S7]; }
					inline BitEditor S8(void) { return m_regs[(word)eRegisters::S8]; }

					inline bool read(eRegisters reg, BitEditor& outData) { return __read((word)reg, outData); }
					inline bool read_m(eRegisters reg, BitEditor& outData, word __m_param) { return __read_m((word)reg, outData, __m_param); }
					inline bool write(eRegisters reg, BitEditor data) { return __write((word)reg, data); }
					inline bool write_m(eRegisters reg, BitEditor data, word __m_param) { return __write_m((word)reg, data, __m_param); }
					bool __read(word addr, BitEditor& outData);
					bool __read_m(word addr, BitEditor& outData, word __m_param);
					bool __write(word addr, BitEditor data);
					bool __write_m(word addr, BitEditor data, word __m_param);
					inline TMemoryList_c& getAsReadOnly(void) { return m_regs; }

					inline static REG& instance(void) { return *REG::s_instance; }

				private: //TODO: Temp, set back to private
					inline BitEditor& rw_RV(void) { return m_regs[(word)eRegisters::RV]; }
					inline BitEditor& rw_CF(void) { return m_regs[(word)eRegisters::CF]; }
					inline BitEditor& rw_IP(void) { return m_regs[(word)eRegisters::IP]; }
					inline BitEditor& rw_DR(void) { return m_regs[(word)eRegisters::DR]; }
					inline BitEditor& rw_ES(void) { return m_regs[(word)eRegisters::ES]; }
					inline BitEditor& rw_FL(void) { return m_regs[(word)eRegisters::FL]; }
					inline BitEditor& rw_SP(void) { return m_regs[(word)eRegisters::SP]; }

					inline BitEditor& rw_S1(void) { return m_regs[(word)eRegisters::S1]; }
					inline BitEditor& rw_S2(void) { return m_regs[(word)eRegisters::S2]; }
					inline BitEditor& rw_S3(void) { return m_regs[(word)eRegisters::S3]; }
					inline BitEditor& rw_S4(void) { return m_regs[(word)eRegisters::S4]; }
					inline BitEditor& rw_S5(void) { return m_regs[(word)eRegisters::S5]; }
					inline BitEditor& rw_S6(void) { return m_regs[(word)eRegisters::S6]; }
					inline BitEditor& rw_S7(void) { return m_regs[(word)eRegisters::S7]; }
					inline BitEditor& rw_S8(void) { return m_regs[(word)eRegisters::S8]; }

				private:
					TMemoryList m_regs;

					static REG* s_instance;

					friend class CPU;
					friend class Debugger;
			};
			inline REG* REG::s_instance = new REG();

		}

		class VirtualMachine : public IOReciever, public Protectable
		{
			public:
				VirtualMachine(void);
				inline static VirtualMachine& instance(void) { return *VirtualMachine::s_instance; }
				inline static hw::RAM& getRAM(void) { return VirtualMachine::m_ram; }
				inline static hw::REG& getREG(void) { return VirtualMachine::m_reg; }
				inline static hw::CPU& getCPU(void) { return VirtualMachine::m_cpu; }
				inline static hw::GPU& getGPU(void) { return VirtualMachine::m_gpu; }
				Process& getCurrentProcess(void);
				inline static Keyboard& getKeyboardHandler(void) { return VirtualMachine::m_keyboard; }

				//TODO: temp
				inline void setCurrentProc(Process& proc) { m_currentProc = &proc; }

			private:
				Process* m_currentProc;

				inline static hw::CPU& m_cpu = hw::CPU::instance();
				inline static hw::GPU& m_gpu = hw::GPU::instance();
				inline static hw::RAM& m_ram = hw::RAM::instance();
				inline static hw::REG& m_reg = hw::REG::instance();
				inline static Keyboard m_keyboard = Keyboard();
				static VirtualMachine* s_instance;

				friend class Debugger;
		};

		inline VirtualMachine* VirtualMachine::s_instance = new VirtualMachine();
	}
}

#endif