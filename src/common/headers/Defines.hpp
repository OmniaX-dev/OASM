#ifndef __DEFINES__HPP__
#define __DEFINES__HPP__

#ifdef _WIN32
	#define WINDOWS_OS
	#ifdef _WIN32_WINNT
		#undef _WIN32_WINNT
	#endif
	#define _WIN32_WINNT 0x0501
#else
	#define LINUX_OS
#endif

//Basic constants
#if !defined(__BUILD_NUMBER__)
	#define __BUILD_NUMBER__ 0
#endif
#if !defined(NULL)
	#define NULL 0
#endif
//#define __INT_CMP_VAL -32768
#define PI 3.1415926535898
#define __INVALID_OBJ true

//Default values
#define AUTO__MAX_FLAGS 100
#define AUTO__MAX_ERRORS 200

//Namespace constants
#define NS_MEMBER_NOT_FOUND "NS_MNF"
#define NS_GET_METHOD_NOT_OVERLOADED "NS_GMNO"
#define NS_NOT_FOUND "NS_NF"

//Flag constants
#define FLG__PRINT_ERROR_ON_PUSH 0

//Macro functions
#define ERROR_DATA() OmniaString(CPP_STR(__LINE__)), OmniaString(__FILE__)
#define CPP_STR(val) OmniaString::intToStr(val).cpp()
#define STR_BOOL(b) (b ? "true" : "false")
#define INT_BOOL(i) (i == 0 ? false : true)
#define ZERO(n) (n > 0 ? n : 0)
#define FRAND() ((float)(rand() % 10000)) / 10000.0f
#define RANDOM(min, max) rand() % (max - min + 1) + min
#define LERP(n1, n2, f) (n2 - n1) * f + n1
#define CAP(n, max) (n > max ? max : n)
#define CAPD(n, min) (n < min ? min : n)
#define CAPB(n, min, max) (n < min ? min : (n > max ? max : n))
#define MAX(n1, n2) (n1 > n2 ? n1 : n2)
#define MIN(n1, n2) (n1 < n2 ? n1 : n2)
#define PROPORTION(w, x, y) ((x * w) / y)
#define STDVEC_REMOVE(vec, index) vec.erase(vec.begin() + index)
#define STDVEC_CONTAINS(vec, elem) (std::find(vec.begin(), vec.end(), elem) != vec.end())

//Memory management macros
#define new_sh(type) std::make_shared<type>
#define sh_ptr(type) std::shared_ptr<type>
#define new_un(type) std::make_unique<type>
#define un_ptr(type) std::unique_ptr<type>

#define oasm_nullptr 0x0000

#include "ErrorDefines.hpp"

#define D__DEFAULT_IPC 12

#define __gen_bit_mask(_start_bit, _end_bit) ((-1 << (_end_bit - _start_bit)) ^ 0xFFFF) << (word)abs(_start_bit - 1)
#define __return_if_current_proc_invalid(__return_value) \
					Process& __proc = VirtualMachine::instance().getCurrentProcess(); \
					if (__proc.isInvalidProc()) { \
						pushError(D__RAM_ERR__IO_FAILED_INVALID_PROC, *VirtualMachine::instance().getOutputHandler(), "VirtualRAM Error"); \
						return __return_value; \
					}
#define __return_and_set_ip(__return_val, __ip_val) \
					_reg.rw_IP() = __ip_val; \
					return __return_val;
#define __return_if_const_op1(__return_val) \
					if (m_const_op1) { \
						pushError(D__CPU_ERR__CONST_OP1); \
						_reg.rw_IP() = m_old_pc_val; \
						return __return_val; \
					} 
#define p_inst(__inst) (word)eInstructionSet::__inst
#define p_addr(__addr) (word)eAddressingModes::__addr
#define p_reg(__reg) (word)eRegisters::__reg
#define p_flg(__flg) (word)eFlags::__flg
#define p_cmd(__cmd) (word)eComCodes::__cmd

#define D__MEMORY_SIZE 16384
#define D__HEAP_SIZE 2048
#define D__STACK_SIZE 2048
#define D__LIB_MEM_SIZE 4096

#define D__VIDEO_MEMORY_SIZE 8192

#define D__MEMORY_START 0x0001
#define D__HEAP_SPACE_START D__MEMORY_SIZE - D__HEAP_SIZE - D__STACK_SIZE - D__LIB_MEM_SIZE
#define D__STACK_SPACE_START D__MEMORY_SIZE - D__STACK_SIZE - D__LIB_MEM_SIZE
#define D__LIB_SPACE_START D__MEMORY_SIZE - D__LIB_MEM_SIZE

#define D__NORMAL_INST_MODE 0x00
#define D__MASKED_INST_MODE 0x01

#define TMemCellList std::vector<Omnia::oasm::hw::RAM::MemCell>
#define TMemoryList std::vector<Omnia::common::BitEditor>
#define TMemCellList_c const std::vector<Omnia::oasm::hw::RAM::MemCell>
#define TMemoryList_c const std::vector<Omnia::common::BitEditor>

#endif
