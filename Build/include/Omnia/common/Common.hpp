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
				Process(void);
				inline static Process& invalidProc(void) { return *s_invalidProc; }
				inline bool isInvalidProc(void) { return getID() == Process::INVALID_PROC_ID && isInvalid(); }
				inline bool done(void) { return m_processFinished; }
				inline void stop(void) { m_processFinished = true; }

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
				bool addHandler(word code, ExtComHandler& ech);
				bool hasHandler(word code);
				bool execHandler(word code, BitEditor param, IOReciever& iomgr, BitEditor& outData);

            private:
                inline ECM(void) {  }

            private:
                std::map<word, ExtComHandler*> m_comHandlers;
                static ECM s_instance;
        };

        inline ECM ECM::s_instance;

		class SR_CallTree
		{
			public: struct tCallData : public Identifiable
			{
				OmniaString space;
				OmniaString inst;
				OmniaString label;
				MemAddress address;
				uint32 tickCounter;
				uint8 indent;
				bool closed;
			};
			public:
				inline SR_CallTree(void) { currentTab = 0; }
				void call(OmniaString __lbl_name);
				void ret(void);
				void print(OutputManager& out, word __line_w, word __lines = 0);
				void tick(void);
				bool getCurrentCall(OmniaString& outLabelName);


			public:
				word currentTab;
				std::vector<tCallData> callList;
				std::vector<std::pair<uint32, OmniaString>> labelStack;

				inline static uint32 s_next_id = 0;
		};

		class SymbolTable
		{
			public:
				bool isLabel(MemAddress __addr, OmniaString& outLabel);
				bool isLabel(OmniaString __sym);
				bool isReserve(MemAddress __addr, OmniaString& outVarName);
				bool isReserve(OmniaString __sym);

			public:
				std::map<MemAddress, OmniaString> m_labels;
				std::map<MemAddress, OmniaString> m_reserves;
				std::map<MemAddress, OmniaString> m_source;
				SR_CallTree m_callTree;

		};

		class CompileErrorReciever
		{
			public:
				inline CompileErrorReciever(void) {  }
				virtual ErrorCode printError(ErrorCode __err, OutputManager& out, OmniaString __extra_info = "", OmniaString __line = "", OmniaString __file = "", int32 __line_number = -1, bool __print_end_line = true);

			public:
				inline static std::map<const ErrorCode, const OmniaString> __error_map = {
					{ D__NO_ERROR, "No Error." },
					{ D__ASSEMBLER_ERR_NO_INPUT_FILE, "Error: No input file specified." },
					{ D__ASSEMBLER_ERR_NO_OUTPUT_FILE, "Error: No output file specified." },
					{ D__ASSEMBLER_ERR_FAILED_TO_CREATE_EXEC, "Error: Failed to create executable file." },
					{ D__ASSEMBLER_ERR_FAILED_TO_CREATE_DBG_FILE, "Error: Failed to create debug-file." },
					{ D__ASSEMBLER_ERR_INVALID_ASSEMBLER_TOKEN, "Error: Invalid statement." },
					{ D__ASSEMBLER_ERR_INVALID_LABEL_NAME, "Error: Invalid label definition." },
					{ D__ASSEMBLER_ERR_UNKNOWN_SYMBOL, "Error: Unknown symbol." },
					{ D__ASSEMBLER_ERR_NON_CONST_ARRAY_INDEX, "Error: Non-const array index." },
					{ D__ASSEMBLER_ERR_INVALID_AS_CMD_SYNTAX, "Error: Invalid assembler-command syntax." },
					{ D__ASSEMBLER_ERR_INVALID_AS_CMD_PARAM, "Error: Invalid assembler-command param." },
					{ D__ASSEMBLER_ERR_UNKNOWN_AS_CMD, "Error: Unknown assembler-command." },
					{ D__ASSEMBLER_ERR_FAILED_TO_LOAD_STATIC_LIB, "Error: Failed to load static-lib." },
					{ D__ASSEMBLER_ERR_STATIC_LIB_NOT_FOUND, "Error: static-lib not found." },
					{ D__ASSEMBLER_ERR_WRONG_ARGS_IN_EXTERN_COMMAND, "Error: Invalid number of arguments in <extern> assembler command." },
					{ D__ASSEMBLER_ERR_DISCREPANT_EXTERN_SUBROUTINES_SIZE, "Error: Extern sub-routines count not matching." },
					{ D__ASSEMBLER_ERR_EXTERN_SUBROUTINE_NOT_FOUND, "Error: Extern sub-routine not found." },
					{ D__ASSEMBLER_ERR_EXTERN_SUBROUTINE_NOT_FOUND_IN_AS_CMD, "Error: Extern sub-routine not found in <extern> assembler command." },
					{ D__ASSEMBLER_ERR_UNKNOWN_LIB_IN_EXTERN_AS_CMD, "Error: Unknown library name in <extern> assembler command." },
					{ D__ASSEMBLER_ERR_NON_INtEGER_ADDRESS_IN_EXTERN_AS_CMD, "Error: Non-Integer value passed as address in <extern> assembler command." },
					
					{ D__ASSEMBLER_ERR_FAILED_TO_OPEN_INPUT_FILE, "Error: Failed to open input file." },
					{ D__ASSEMBLER_ERR_EMPTY_IMPUT_FILE, "Error: Input file is empty." },
					{ D__ASSEMBLER_ERR_RESOLVE_INCLUDES_FAILED, "Error: Failed to resolve includes." },
					{ D__ASSEMBLER_ERR_REMOVE_COMMENTS_FAILED, "Error: Failed to remove comments." },
					{ D__ASSEMBLER_ERR_RESOLVE_DEFINES_FAILED, "Error: Failed to resolve defines." },
					{ D__ASSEMBLER_ERR_RESOLVE_ALIASES_FAILED, "Error: Failed to resolve aliases." },
					{ D__ASSEMBLER_ERR_RESOLVE_MACROS_FAILED, "Error: Failed to resolve macros." },
					{ D__ASSEMBLER_ERR_RESOLVE_CMD_DIRECTIVE_FAILED, "Error: Failed to resolve command directives." },
					{ D__ASSEMBLER_ERR_RESOLVE_DATA_DIRECTIVE_FAILED, "Error: Failed to resolve data directives." },
					{ D__ASSEMBLER_ERR_MISSING_PARENTH_IN_COMMAND_DIRECTIVE, "Error: Missing parenthesis in command directive." },
					{ D__ASSEMBLER_ERR_FEW_ARGS_IN_COMMAND_DIRECTIVE, "Error: Invalid number of arguments in command directive." },
					{ D__ASSEMBLER_ERR_MISSING_EQUALS_IN_COMMAND_DIRECTIVE, "Error: Missing equals sign in command directive." },
					{ D__ASSEMBLER_ERR_MISSING_PARENTH_IN_END_STRUCT_DIRECTIVE, "Error: Missing parenthesis in end-struct directive." },
					{ D__ASSEMBLER_ERR_MISSING_EQUALS_IN_DATA_DIRECTIVE, "Error: Missing equals sign in data directive." },
					{ D__ASSEMBLER_ERR_MISSING_PARENTH_IN_RESERVE_DIRECTIVE, "Error: Missing parenthesis in reserve directive." },
					{ D__ASSEMBLER_ERR_MISSING_PARENTH_IN_ARRAY_DIRECTIVE, "Error: Missing parenthesis in array directive." },
					{ D__ASSEMBLER_ERR_WRONG_ARGS_IN_ARRAY_DIRECTIVE, "Error: Invalid number of arguments in array directive." },
					{ D__ASSEMBLER_ERR_NON_INT_ARRAY_SIZE, "Error: Non-Integer array size." },
					{ D__ASSEMBLER_ERR_MISSING_PARENTH_IN_LOAD_STRING_DIRECTIVE, "Error: Missing parenthesis in load_string directive." },
					{ D__ASSEMBLER_ERR_WRONG_ARGS_IN_LOAD_STRING_DIRECTIVE, "Error: Invalid number of arguments in load_string directive." },
					{ D__ASSEMBLER_ERR_MISSING_DOUBLE_QUOTE_IN_LOAD_STRING_DIR, "Error: Missing double-quote in load_string directive." },
					{ D__ASSEMBLER_ERR_SYM_NOT_FOUND_IN_LOAD_STRING_DIRECTIVE, "Error: Invalid variable in load_string directive." },
					{ D__ASSEMBLER_ERR_MISSING_PARENTH_IN_STRUCT_DIRECTIVE, "Error: Missing parenthesis in struct directive." },
					{ D__ASSEMBLER_ERR_MISSING_EQUALS_IN_ALIAS_DIRECTIVE, "Error: Missing equals sign in alias directive." },
					{ D__ASSEMBLER_ERR_INVALID_MACRO_SYNTAX, "Error: Invalid macro syntax." },
					{ D__ASSEMBLER_ERR_MISSING_SQUARE_BRACKET_IN_INCLUDE_DIR, "Error: Missing square-bracket in include directive." },
					{ D__ASSEMBLER_ERR_UNABLE_TO_READ_FILE_IN_INCLUDE_DIRECTIVE, "Error: Unable to read file in include directive." }
				};
		};
		
		class ErrorReciever
		{
			public:
				inline ErrorReciever(void) : m_callback(nullptr) { }
				inline const ErrorCode getLastErrorCode(void) { return (!__empty() ? m_errorQueue[m_errorQueue.size() - 1] : D__NO_ERROR); }
				inline std::vector<ErrorCode> getErrorQueue(void) { return m_errorQueue; }
				inline void redirectErrorsTo(ErrorReciever& __callback) { m_callback = &__callback; }
				inline void disableRedirect(void) { m_callback = nullptr; }

			protected:
				inline bool __empty(void) { return m_errorQueue.size() == 0; }
				inline virtual void pushError(ErrorCode __err_code) {  }
				virtual void pushError(ErrorCode __err_code, OutputManager& out, OmniaString __extra_text = "", MemAddress __addr = oasm_nullptr, word __op_code = (word)eInstructionSet::no_op);
				ErrorCode popError(void);
				std::vector<ErrorCode> flushErrorQueue(void);

			protected:
				std::vector<ErrorCode> m_errorQueue;
				ErrorReciever* m_callback;

			public:
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