#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include "String.hpp"
#include <vector>
#include <map>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <algorithm>

#define AUTO__OASM_MEM_ALLOC 16384
#define AUTO__OASM_HEAP_SIZE 3096
#define AUTO__OASM_STACK_SIZE 2048
#define AUTO__OASM_INST_PER_TICK 16
#define EMPTY_CODE std::vector<_int32>()
#define oasm_nullptr 0

namespace Omnia
{
    namespace oasm
    {
        typedef unsigned short _uint16;
        typedef signed short _int16;
        typedef unsigned int _uint32;
        typedef signed int _int32;
        typedef char _byte;
        typedef unsigned char _ubyte;

        typedef unsigned int MemAddress;

        enum class eByte
        {
            One     = 0x01,
            Two     = 0x02,
            Three   = 0x03,
            Four    = 0x04,
            All     = 0x00
        };
        enum class eInstructionSet
        {
            Invalid     =   0xF0F0,

            mem         =   0x0001,
            com         =   0x0002,
            inc         =   0x0003,
            cmp         =   0x0004,
            jmp         =   0x0005,
            cat         =   0x0006,
            dec         =   0x0007,
            add         =   0x0008,
            sub         =   0x0009,
            mul         =   0x000A,
            div         =   0x000B,
            mov         =   0x000C,
            flg         =   0x000D,

            call        =   0x000E,
            push        =   0x000F,
            pop         =   0x0010,
            ret         =   0x0011,

            req_local   =   0x3000,
            req_heap    =   0x3001,
            req_stack   =   0x3002,
            alloc_h     =   0x3100,
            free_h      =   0x3101,
            end         =   0xFFFF
        };
        enum class eFlags
        {
            NormalAssignment            =   0x0000,
            IndirectAssignment          =   0x0001,
            DereferenceAssignment       =   0x0002,
            ReferenceAssign             =   0x0003,
            HeapAllocation              =   0x0004,

            DereferenceDestination      =   0x1000,
            ResetFlags                  =   0xFFFFFF,
            RestoreFlags                =   0x0000,
            StackEmpty                  =   0x1002,

            JumpUnconditional           =   0x0000,
            JumpLess                    =   0x0001,
            JumpLssEquals               =   0x0002,
            JumpGreater                 =   0x0003,
            JumpGreaterEquals           =   0x0004,
            JumpEquals                  =   0x0005,
            JumpNotEquals               =   0x0006,

            PointerPointerCompare       =   0x0000,
            PointerValueCompare         =   0x0001,
            ValuePointerCompare         =   0x0002,
            ValueValueCompare           =   0x0003,

            CommandStringParam          =   0x0001,

            StringIntConcat             =   0x0010,
            StringStringConcat          =   0x0011,
            StringCharConcat            =   0x0012,
            StringConstStrConcat        =   0x0013,

            StringAssign                =   0x0020,
            VariableOffset              =   0x0021,
            ConstantOffset              =   0x0022,
            NoOffset                    =   0x0000,

            OldStringReadMode           =   0x0030,
            CompactStringReadMode       =   0x0031,

            StackPop                    =   0x0000,
            StackRead                   =   0x0040
        };
        enum class eMemState
        {
            Free = 0,
            Allocated,
            Reserved
        };
        enum class eMemCellType
        {
            Register = 0,
            Normal,
            Stack,
            Heap
        };
        enum class eRegisters
        {
            NP          =   0, //Null Pointer
            RV          =   1, //Return value
            CF          =   2, //Compare Flag
            IP          =   3, //Instruction Pointer
            DR          =   4, //Division Reminder
            ES          =   5, //Exit Status
            FL          =   6, //Flags
            SP          =   7, //Stack Pointer

            R0          =   8,
            R1          =   9,
            R2          =   10,
            R3          =   11,
            R4          =   12,
            R5          =   13,
            R6          =   14,
            R7          =   15,
            R8          =   16,
            R9          =   17,
            R10         =   18,
            R11         =   19,
            R12         =   20,
            R13         =   21,
            R14         =   22,
            R15         =   23,
            R16         =   24,
            R17         =   25,
            R18         =   26,
            R19         =   27,
            R20         =   28,
            R21         =   29,
            R22         =   30,
            R23         =   31,

            MemStart    =   32
        };
        enum class eCompareState
        {
            Less            =   0x0000,
            Greater         =   0x0001,
            Equals          =   0x0002
        };
        enum class eComCodes
        {
            PrintIntToConsole      =    0x0002,
            PrintStringToConsole   =    0x0003,
            PrintNewLineToConsole  =    0x0004,
            StringLength           =    0x0005,
            GetArgsData            =    0x0006,
            ReadStringInput        =    0x0007,
            ReadIntInput           =    0x0008,
            External               =    0x9000,

            ExtCom1                =    0x9001,
            ExtCom2                =    0x9002,
            ExtCom3                =    0x9003,
            ExtCom4                =    0x9004,
            ExtCom5                =    0x9005,
            ExtCom6                =    0x9006,
            ExtCom7                =    0x9007,
            ExtCom8                =    0x9008,
            ExtCom9                =    0x9009,
            ExtCom10               =    0x900A,
            ExtCom11               =    0x900B,
            ExtCom12               =    0x900C,
            ExtCom13               =    0x900D,
            ExtCom14               =    0x900E,
            ExtCom15               =    0x900F,
            ExtCom16               =    0x9010,
            ExtCom17               =    0x9011,
            ExtCom18               =    0x9012,
            ExtCom19               =    0x9013,
            ExtCom20               =    0x9014
        };
        enum class eExitCodes
        {
            ForceQuit           =   0xA000,
            ExitSuccess         =   0x000F
        };
        enum class eOperandType
        {
            Invalid = 0,
            Register,
            Variable,
            Integer,
            String,
            Char,
            VariablePointer,
            VariableRef,
            RegisterRef
        };
        enum class eVersion
        {
            Iteration = 0x0100011A
        };

        enum class eAssemblerDirectives
        {
            Include = 0,
            IncludeGuard,
            CloseIncludeGuard,
            SubRoutine,
            CloseSubRoutine,
            Low,
            Alias,
            Label,
            Var,
            MemorySetup,

            Invalid
        };
        enum class eAssemblerErrors
        {
            NoError = 0,
            WrongReqLocal,
            WrongReqHeap,
            WrongReqStack,
            ReqLocalMissing,
            ReqHeapMissing,
            ReqStackMissing,
            WrongMemInstruction,
            WrongCatInstruction,
            UnknownSymbol,
            UnknownRegister,
            BadMemAssign,
            BadCat,
            CatNoStringSrc,
            StrAssignToReg,
            DereferenceReg,
            BadAllocH,
            BadEnd,
            BadFreeH,
            UnknownPointer,
            BadFlg,
            BadDec,
            BadInc,
            BadCom,
            WrongJmpInstruction,
            BadJmp,
            UnknownLabel,
            WrongCmpInstruction,
            BadCmp,
            RegOrValCmp,
            UnknownInstruction,
            WrongAddInstruction,
            WrongSubInstruction,
            WrongMulInstruction,
            WrongDivInstruction,
            BadAdd,
            BadSub,
            BadMul,
            BadDiv,
            FlgValNumeric,
            UnknownFlgVal,
            LowDirectiveInt,
            OpenSRLeft,
            UnknownDirective
        };
        enum class eRuntimeErrors
        {
            RequestMemSetFail = 1000,
            CopyProcFail,
            CopyProcOutOfBounds,
            ACCVmemPermission,
            ACCVmemProtected,
            ACCVmemStack,
            ACCVsetPermission,
            ACCVsetProtected,
            ACCVsetReadOnly,
            ACCVsetStack,
            AssignFault,
            CMPFlagUnknown,
            UnknownCommand,
            DivZero,
            JMPFlagUnknown,
            CATFlagUnknown,
            HeapAllocFail,
            StackOverflow,
            PopReg,
            SPDiscrepancy,
            EntryPointMissing,
            EntryPointInvalid
        };
        enum class ePreProcessorErrors
        {
            FailedToOpenFile,
            WrongInclDirective,
            WrongAliasDir,
            EmptyFile
        };

        enum class ePPMC
        {
            NoCommand = 0,
            GenNextEndIfLabel,
            GetLastEndIfLabel,
            GenNextEndLoopLabel,
            GetLastEndLoopLabel,
            GenNextLoopLabel,
            GetLastLoopLabel,
            SkipLastEndLoop,
            SkipLastLoop
        };

        class Utils
        {
            public:
                static inline bool isHex(String hex)
                {
                    _string s = hex.toLowerCase().cpp();
                    return s.compare(0, 2, "0x") == 0 &&
                           s.size() > 2 &&
                           s.find_first_not_of("0123456789abcdef", 2) == std::string::npos;
                }
                static inline bool isInt(String str)
                {
                    return Utils::isHex(str) || str.isNumeric();
                }
                static inline _int32 strToInt(String str)
                {
                    if (!Utils::isInt(str)) return 0;
                    return (_int32)strtol(str.trim().toLowerCase().c_str(), NULL, 0);
                }
                static inline bool readFile(String fileName, std::vector<String>& lines)
                {
                    _string line;
                    std::ifstream file(fileName.cpp());
                    if (file.fail()) return false;
                    lines.clear();
                    while (std::getline(file, line))
                        lines.push_back(String(line));
                    return true;
                }
                static inline String replaceAllVarName(String str, String search, String replace)
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
                    return String(s);
                }
                static inline _int32 setb(_int32 n, eByte b, _ubyte byte)
                {
                    if (b == eByte::One) return (n & 0xFFFFFF00) | byte;
                    if (b == eByte::Two) return (n & 0xFFFF00FF) | (byte << 8);
                    if (b == eByte::Three) return (n & 0xFF00FFFF) | (byte << 16);
                    if (b == eByte::Four) return (n & 0x00FFFFFF) | (byte << 24);
                    if (b == eByte::All) return (_int32)byte;
                    return 0;
                }
                static inline _ubyte getb(_int32 n, eByte b)
                {
                    if (b == eByte::One) return (_ubyte)(n & 0x000000FF);
                    if (b == eByte::Two) return (_ubyte)((n & 0x0000FF00) >> 8);
                    if (b == eByte::Three) return (_ubyte)((n & 0x00FF0000) >> 16);
                    if (b == eByte::Four) return (_ubyte)((n & 0xFF000000) >> 24);
                    return 0;
                }
                static inline _int32 sets(_int32 n, bool low, _int16 s)
                {
                    if (low) return (n & 0xFFFF0000) | s;
                    else return (n & 0x0000FFFF) | s;
                }
                static _int16 gets(_int32 n, bool low)
                {
                    if (low) return (_int16)(n & 0x0000FFFF);
                    else return (_int16)(n & 0xFFFF0000) >> 16;
                }
                static inline std::vector<_int32> strToMemCells(String str)
                {
                    std::vector<_int32> result;
                    _int32 cell = 0;
                    char c = 0;
                    _uint16 j = 4;
                    for (_uint16 i = 0; i < str.length(); i++, j--)
                    {
                        c = str[i];
                        cell = Utils::setb(cell, (eByte)j, (_ubyte)((_int32)c));
                        if (j == 1)
                        {
                            j = 5;
                            result.push_back(cell);
                            cell = 0;
                        }
                    }
                    if (j > 0)
                        result.push_back(cell);
                    return result;
                }
                static inline String intToHexStr(_int32 i, bool prefix = true)
                {
                    char buff[9];
                    sprintf(buff, "%08X", i);
                    String m = "";
                    for (_uint16 j = 0; j < 8; j++)
                        m = m.add(buff[j]);
                    if (prefix) return String("0x").add(m);
                    return m;
                }
                static inline String instFromCode(_int32 code)
                {
                    switch ((eInstructionSet)code)
                    {
                        case eInstructionSet::mem: return "mem";
                        case eInstructionSet::com: return "com";
                        case eInstructionSet::inc: return "inc";
                        case eInstructionSet::cmp: return "cmp";
                        case eInstructionSet::jmp: return "jmp";
                        case eInstructionSet::cat: return "cat";
                        case eInstructionSet::dec: return "dec";
                        case eInstructionSet::add: return "add";
                        case eInstructionSet::sub: return "sub";
                        case eInstructionSet::mul: return "mul";
                        case eInstructionSet::div: return "div";
                        case eInstructionSet::flg: return "flg";
                        case eInstructionSet::call: return "call";
                        case eInstructionSet::push: return "push";
                        case eInstructionSet::pop: return "pop";
                        case eInstructionSet::ret: return "ret";
                        case eInstructionSet::req_local: return "req_local";
                        case eInstructionSet::req_heap: return "req_heap";
                        case eInstructionSet::req_stack: return "req_stack";
                        case eInstructionSet::alloc_h: return "alloc_h";
                        case eInstructionSet::free_h: return "free_h";
                        case eInstructionSet::end: return "end";
                        default: return "INVALID";
                    }
                }
        };

        class StringBuilder
        {
            public:
                inline StringBuilder(void) { m_data = ""; }
                inline StringBuilder(String str) { m_data = str; }
                inline StringBuilder(int i) { m_data = String::intToStr(i); }
                inline StringBuilder(long int i) { m_data = String::intToStr(i); }
                inline StringBuilder(float f) { m_data = String::floatToStr(f); }
                //inline StringBuilder(Object& o) { m_data = o.toString(); }
                inline StringBuilder(StringBuilder& sb) { m_data = sb.get(); }
                
                inline StringBuilder& add(String str) { m_data = m_data.add(str); return *this; }
                inline StringBuilder& add(int i) { m_data = m_data.addInt(i); return *this; }
                inline StringBuilder& add(long int i) { m_data = m_data.addInt(i); return *this; }
                inline StringBuilder& add(float f) { m_data = m_data.add(String::floatToStr(f)); return *this; }
                //inline StringBuilder& add(Object& o) { m_data = m_data.add(o.toString()); return *this; }
                inline StringBuilder& add(StringBuilder sb) { m_data = sb.get(); return *this; }

                inline String get(void) { return m_data; }

            private:
                String m_data;
        };
    }
}

#endif
