#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include "Flags.hpp"

#include "BitEditor.hpp"
#include "Defines.hpp"
#include "IOManager.hpp"
#include <map>

namespace Omnia
{
	using namespace common;
	namespace common
	{
		class Process : public Validable, public Identifiable
		{
			public:
				Process(void)
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
				inline static Process& invalidProc(void) { return *s_invalidProc; }
				bool isInvalidProc(void) { return getID() == Process::INVALID_PROC_ID && isInvalid(); }
				bool done(void) { return m_processFinished; }
				void stop(void) { m_processFinished = true; }

			private:
				inline Process(int32 id) { setID(id); invalidate(); }

			public: //TODO: Replace with getters and setters
				MemAddress m_stackAddr;
				MemAddress m_heapAddr;
				MemAddress m_codeAddr;
				bool m_stackAllocated;
				bool m_heapAllocated;
				bool m_codeAllocated;
				word m_codeSize;
				word m_stackSize;
				word m_heapSize;
				bool m_processFinished;
				TMemoryList m_code;
			private:
				static Process* s_invalidProc;
			public:
				inline static const int32 INVALID_PROC_ID = -8888;
		};

		inline Process* Process::s_invalidProc = new Process(Process::INVALID_PROC_ID);

		class ExtComHandler
        {
            public:
                inline virtual bool handleCommand(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData) { return false; }
        };

        class ECM
        {
            public:
                static inline ECM& instance(void) { return s_instance; }

				inline bool addHandler(word code, ExtComHandler& ech)
				{
					if (hasHandler(code)) return false;
					m_comHandlers[code] = &ech;
					return true;
				}

				inline bool hasHandler(word code)
				{
					return m_comHandlers.count(code) != 0;
				}

				inline bool execHandler(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData)
				{
					if (!hasHandler(code)) return false;
					outData = BitEditor((word)0);
					return m_comHandlers[code]->handleCommand(code, param, iomgr, outData);
				}

            private:
                inline ECM(void) {  }

            private:
                std::map<word, ExtComHandler*> m_comHandlers;
                static ECM s_instance;
        };

        inline ECM ECM::s_instance;

		class SymbolTable
		{
			public:
				inline bool isLabel(MemAddress __addr, OmniaString& outLabel)
				{
					if (m_labels.count(__addr) != 0)
					{
						outLabel = m_labels[__addr];
						return true;
					}
					return false;
				}
				inline bool isReserve(MemAddress __addr, OmniaString& outVarName)
				{
					if (m_reserves.count(__addr) != 0)
					{
						outVarName = m_reserves[__addr];
						return true;
					}
					return false;
				}

			public:
				std::map<MemAddress, OmniaString> m_labels;
				std::map<MemAddress, OmniaString> m_reserves;
				std::vector<OmniaString> m_source;

		};
		
		class ErrorReciever
		{
			public:
				inline ErrorReciever(void) { }
				inline const ErrorCode getLastErrorCode(void) { return (!__empty() ? m_errorQueue[m_errorQueue.size() - 1] : D__NO_ERROR); }
				inline std::vector<ErrorCode> getErrorQueue(void) { return m_errorQueue; }

			protected:
				inline bool __empty(void) { return m_errorQueue.size() == 0; }
				inline virtual void pushError(ErrorCode __err_code) {  }
				inline void pushError(ErrorCode __err_code, OutputManager& out, OmniaString __extra_text = "", MemAddress __addr = oasm_nullptr, word __op_code = (word)eInstructionSet::no_op)
				{
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
						if (__op_code != (word)eInstructionSet::no_op) //TODO: Eventually map op_codes to text
							out.tab().print("Instruction: ").print(Utils::intToHexStr(__op_code)).print("(-").print(Utils::mapInstruction((eInstructionSet)__op_code)).print("-)").newLine();
						out.print(Utils::duplicateChar('=', 50)).newLine();
					}
				}
				inline ErrorCode popError(void)
				{
					if (__empty()) return D__NO_ERROR;
					ErrorCode __tmp = m_errorQueue[m_errorQueue.size() - 1];
					STDVEC_REMOVE(m_errorQueue, m_errorQueue.size() - 1);
					return __tmp;
				}
				inline std::vector<ErrorCode> flushErrorQueue(void)
				{
					std::vector<ErrorCode> __tmp = m_errorQueue;
					m_errorQueue.clear();
					return __tmp;
				}

			protected:
				std::vector<ErrorCode> m_errorQueue;

				inline static std::map<const ErrorCode, const OmniaString> __error_map = {
					{ D__NO_ERROR, "" },

					{ D__EXIT_ERR__NO_FRONTEND_SPECIFIED, "Unable to start system: No frontend specified." },

					{ D__CPU_ERR__FETCH_STEP_FAILED, "Fetch phase failed." },
					{ D__CPU_ERR__DECODE_STEP_FAILED, "Decode phase failed." },
					{ D__CPU_ERR__EXECUTE_STEP_FAILED, "Execute phase failed." },
					{ D__CPU_ERR__UNKNOWN_INSTRUCTION, "Unknown instruction." },
					{ D__CPU_ERR__UNKNOWN_FLAG, "Unknown flag" },
					{ D__CPU_ERR__HEAP_ALLOC_FAILED, "Heap allocation failed." },
					{ D__CPU_ERR__STACK_ALLOC_FAILED, "Stack allocation failed." },
					{ D__CPU_ERR__MAP_ADDR_MODE_FAILED, "Unable to map Addressing-mode." },
					{ D__CPU_ERR__WRITE_REG_FAILED, "Unable to write to register." },
					{ D__CPU_ERR__WRITE_FAILED, "Unable to write to memory." },
					{ D__CPU_ERR__INST_ERR_STEP_2, "Error on fetch-step 2." },
					{ D__CPU_ERR__INST_ERR_STEP_3, "Error on fetch-step 3." },
					{ D__CPU_ERR__INST_ERR_STEP_4, "Error on fetch-step 4." },
					{ D__CPU_ERR__INST_ERR_STEP_5, "Error on fetch-step 5." },
					{ D__CPU_ERR__READ_REG_FAILED, "Unable to read from register." },
					{ D__CPU_ERR__READ_FAILED, "Unable to read from memory." },
					{ D__CPU_ERR__MAP_OP2_ADDR_MODE_FAILED, "Unable to decode op2's Addressing-mode." },
					{ D__CPU_ERR__CONST_OP1, "Const_val op1 not allowed for this instruction." },
					{ D__CPU_ERR__DIVISION_BY_ZERO, "Division by 0." },
					{ D__CPU_ERR__MAP_OP1_ADDR_MODE_FAILED, "Unable to decode op1's Addressing-mode." },
					{ D__CPU_ERR__STACK_OVERFLOW, "Stack overflow." },
					{ D__CPU_ERR__POP_FLUSH_AND_READ, "Cannot set pop_flush flag with pop_r* instructions." },
					{ D__CPU_ERR__UNKNOWN_CMD_CODE, "Unknown command code passed to cmd instruction." },
					{ D__CPU_ERR__REQUEST_PTR_FAILED, "Failed to request Heap pointer." },
					{ D__CPU_ERR__FREE_PTR_FAILED, "Failed to free Heap pointer." },
					{ D__CPU_ERR__WRITE_PTR_FAILED, "Unable to write to heap pointer." },
					{ D__CPU_ERR__READ_PTR_FAILED, "Unable to read from heap pointer." },
					{ D__CPU_ERR__REALLOC_SIZE_TOO_SMALL, "Unable to realloc heap stream to smaller (or equals) size." },
					{ D__CPU_ERR__OFFSET_HEAP_INVALID_PROC, "Failed to offset heap address: invalid current process." },
					{ D__CPU_ERR__OFFSET_HEAP_NOT_REQUESTED, "Failed to offset heap address: heap block not requested." },
					{ D__CPU_ERR__FREE_SINGLE_FAILED, "Failed to free single heap cell." },
					{ D__CPU_ERR__RESERVE_SINGLE_FAILED, "Failed to reserve single heap cell." },
					{ D__CPU_ERR__READ_STR_STREAM_FAILED, "Failed to read string from stream." },
					{ D__CPU_ERR__WRITE_STR_STREAM_FAILED, "Failed to write string to stream." },
					{ D__CPU_ERR__OFFSET_CODE_INVALID_PROC, "Failed to offset code address: invalid current process." },
					{ D__CPU_ERR__OFFSET_CODE_NOT_REQUESTED, "Failed to offset heap address: code block not allocated." },

					{ D__RAM_ERR__IO_FAILED_INVALID_PROC, "I/O failed: Invalid current process." },
					{ D__RAM_ERR__READ_FAILED_RESERVED_CELL, "Read failed: Trying to access reserved memory cell." },
					{ D__RAM_ERR__READ_FAILED_DISCREPANT_IDS, "Read failed: Current process ID is different from cell Process ID." },
					{ D__RAM_ERR__WRITE_FAILED_CONST, "Write failed: Trying to write to const memory cell." },
					{ D__RAM_ERR__WRITE_FAILED_RESERVED_CELL, "Write failed: Trying to access reserved memory cell." },
					{ D__RAM_ERR__WRITE_FAILED_DISCREPANT_IDS, "Write failed: Current process ID is different from cell Process ID." },
					{ D__RAM_ERR__WRITE_M_UNKNOWN_PARAM, "Write_m failed: Unknown mask parameter." },
					{ D__RAM_ERR__READ_M_UNKNOWN_PARAM, "Read_m failed: Unknown mask parameter." },
					{ D__RAM_ERR__WRITE_USED, "Write failed: Trying to write to used memory cell." },
					{ D__RAM_ERR__WRITE_CODE, "Write failed: Trying to write to code space." },
					{ D__RAM_ERR__FREE_NULLPTR, "Attempt to free null pointer." },
					{ D__RAM_ERR__FREE_INVALID_PTR, "Attempt to free invalid heap pointer." },
					{ D__RAM_ERR__FREE_DISCREPANT_IDS, "Unable to free heap pointer:  Current process ID is different from cell Process ID." },
					{ D__RAM_ERR__WRITE_FAILED_UNALLOCATED, "Write failed: Trying to write to unallocated memory cell." }
				};
		};
	}
}

#endif