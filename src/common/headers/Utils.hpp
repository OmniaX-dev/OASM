#ifndef __DATATYPES__HPP__
#define __DATATYPES__HPP__

#include "Defines.hpp"
#include "Enums.hpp"
#include "OmniaString.hpp"
#include <memory>
#include <vector>
#include <map>
#include <fstream>
#include <stdio.h>
#include <algorithm>
#include <bitset>

/*
#ifdef LINUX_OS
    // kbhit
    #include <stdio.h>
    #include <sys/ioctl.h> // For FIONREAD
    #include <termios.h>
    #include <stdbool.h>

    static int kbhit(void)
    {
        static bool initflag = false;
        static const int STDIN = 0;

        if (!initflag)
        {
            // Use termios to turn off line buffering
            struct termios term;
            tcgetattr(STDIN, &term);
            term.c_lflag &= ~ICANON;
            tcsetattr(STDIN, TCSANOW, &term);
            setbuf(stdin, NULL);
            initflag = true;
        }

        int nbbytes;
        ioctl(STDIN, FIONREAD, &nbbytes);  // 0 is STDIN
        return nbbytes;
    }
#elif defined(WINDOWS_OS)
    #include <conio.h>
#endif
*/

#if defined(_WIN32) || defined(_WIN64)
#   define NO_ANSI_ESCAPE_SEQUENCES
#endif

#if defined(__CYGWIN__)
#   undef __STRICT_ANSI__
#   include <iostream>
#   include <sstream>
#   define __STRICT_ANSI__
#else
#   include <iostream>
#   include <sstream>
#endif
#include "TermColor.hpp"



namespace Omnia
{
	namespace common
	{
        class OutputManager;

        typedef unsigned short word;

        typedef unsigned short MemAddress;
        typedef signed long int ErrorCode;

		typedef unsigned char ubyte;
		typedef signed char byte;

		typedef unsigned char uint8;
		typedef signed char int8;
		typedef unsigned short uint16;
		typedef signed short int16;
		typedef unsigned int uint32;
		typedef signed int int32;
		typedef unsigned long int uint64;
		typedef signed long int int64;

        enum class eInstructionSet
        {
            no_op       =   0xF0F0,
            req         =   0xF0F1,
            end         =   0xF0F2,
            reserve     =   0xF0F3,
            free_s      =   0xF0F4,
            alloc       =   0xF0F5,
            free        =   0xF0F6,
            realloc     =   0xF0F7,
            cmd         =   0xF0F8,
            flg         =   0xF0F9,
            flg_m       =   0xF0FA,
            offset      =   0xF0FB,

            mem         =   0x1000,
            mem_m       =   0x1001,
            push        =   0x1002,
            push_m      =   0x1003,
            pop         =   0x1004,
            pop_m       =   0x1005,
            pop_r       =   0x1006,
            pop_r_m     =   0x1007,
            lda_str     =   0x1008,
            str_cpy     =   0x1009,
            add_str     =   0x100A,

            inc         =   0x1100,
            inc_m       =   0x1101,
            dec         =   0x1102,
            dec_m       =   0x1103,
            add         =   0x1104,
            add_m       =   0x1105,
            sub         =   0x1106,
            sub_m       =   0x1107,
            mul         =   0x1108,
            mul_m       =   0x1109,
            div         =   0x110A,
            div_m       =   0x110B,
            cmp         =   0x110C,
            cmp_m       =   0x110D,
            
            jmp         =   0x1200,
            call        =   0x1201,
            ret         =   0x1202,
            ret_m       =   0x1203,
            
            _and        =   0x1400,
            and_m       =   0x1401,
            _or         =   0x1402,
            or_m        =   0x1403,
            _not        =   0x1404,
            not_m       =   0x1405,
            bit         =   0x1406,
            mask        =   0x1407
        };
        enum class eAddressingModes
        {
            ConstToAddr         =   0x0000,
            ConstToPtr          =   0x0001,
            ConstToReg          =   0x0002,
            ConstToRegPtr       =   0x0003,
            
            AddrToAddr          =   0x0004,
            AddrToPtr           =   0x0005,
            AddrToReg           =   0x0006,
            AddrToRegPtr        =   0x0007,
            
            PtrToAddr           =   0x0008,
            PtrToPtr            =   0x0009,
            PtrToReg            =   0x000A,
            PtrToRegPtr         =   0x000B,
            
            RefToAddr           =   0x000C,
            RefToPtr            =   0x000D,
            RefToReg            =   0x000E,
            RefToRegPtr         =   0x000F,
            
            RegPtrToAddr        =   0x0010,
            RegPtrToPtr         =   0x0011,
            RegPtrToReg         =   0x0012,
            RegPtrToRegPtr      =   0x0013,
            
            RegToAddr           =   0x0014,
            RegToPtr            =   0x0015,
            RegToReg            =   0x0016,
            RegToRegPtr         =   0x0017,
            
            SingleOp_const      =   0x0018,
            SingleOp_addr       =   0x0019,
            SingleOp_reg        =   0x001A,
            SingleOp_ptr        =   0x001B,
            SingleOp_regPtr     =   0x001C,
            SingleOp_ref        =   0x001D,

            ConstConst          =   0x001E,
            ConstAddr           =   0x001F,
            ConstPtr            =   0x0020,
            ConstRef            =   0x0021,
            ConstRegPtr         =   0x0022,
            ConstReg            =   0x0023,

            b_2ByteMode_mask    =   0b0000000000000000,
            b_LSB_LSB_mask      =   0b0001000000000000,
            b_LSB_MSB_mask      =   0b0010000000000000,
            b_MSB_LSB_mask      =   0b0011000000000000,
            b_MSB_MSB_mask      =   0b0100000000000000,
            b_SingleOp_LSB_mask =   0b0101000000000000,
            b_SingleOp_MSB_mask =   0b0110000000000000,
            b_2B_LSB_mask       =   0b0111000000000000,
            b_2B_MSB_mask       =   0b1000000000000000,
            b_LSB_2B_mask       =   0b1001000000000000,
            b_MSB_2B_mask       =   0b1010000000000000,

            Invalid             =   0xFFFF
        };
        enum class eSingleAddrModes
        {
            MSB = 0,
            LSB,
            _2B
        };
        enum class eFlags
        {
            no_flag                 =   0b000000000000000,

            req_all                 =   0b0000000000000011,
            req_stack               =   0b0000000000000001,
            req_heap                =   0b0000000000000010,

            jmp_unconditional       =   0b0000000000111111,
            jmp_greater             =   0b0000000000000001,
            jmp_less                =   0b0000000000000010,
            jmp_equals              =   0b0000000000000100,
            jmp_not_eq              =   0b0000000000001000,
            jmp_greater_eq          =   0b0000000000010000,
            jmp_less_eq             =   0b0000000000100000,

            add_str_const_stream    =   0b0000000000000001,
            add_str_str_ptr         =   0b0000000000000010,
            add_str_int             =   0b0000000000000100,
            add_str_char            =   0b0000000000001000
        };
        enum class eMemState
        {
            Free = 0,
            Allocated,
            Used,
            Code,
            Reserved
        };
        enum class eMemCellType
        {
            Normal = 0,
            Stack,
            Heap,
            Library
        };
        enum class eMemCellFlag
        {
            NoFlag = 0,
            HeapBlockStart,
            StackBlockStart,
            HeapPtr,
            LocalSpace,
            ConstHeapPtr,
            ConstLocalSpace,
            ConstCell,
            NullPointer,
            UsedSingleHeapCell
        }; 
        enum class eRegisters
        {
            NP          =   0x0000, //Null Pointer
            CF          =   0x0001, //Compare Flag
            IP          =   0x0002, //Instruction Pointer
            ES          =   0x0003, //Exit Status
            SP          =   0x0004, //Stack Pointer
            RV          =   0x0005, //Return value
            DR          =   0x0006, //Division Reminder
            FL          =   0x0007, //Flags
            
            S1          =   0x0008, //Special Register
            S2          =   0x0009, //Special Register
            S3          =   0x000A, //Special Register
            S4          =   0x000B, //Special Register
            S5          =   0x000C, //Special Register
            S6          =   0x000D, //Special Register
            S7          =   0x000E, //Special Register
            S8          =   0x000F, //Special Register

            R0          =   0x0010,
            R1          =   0x0011,
            R2          =   0x0012,
            R3          =   0x0013,
            R4          =   0x0014,
            R5          =   0x0015,
            R6          =   0x0016,
            R7          =   0x0017,
            R8          =   0x0018,
            R9          =   0x0019,
            R10         =   0x001A,
            R11         =   0x001B,
            R12         =   0x001C,
            R13         =   0x001D,
            R14         =   0x001E,
            R15         =   0x001F,
            R16         =   0x0020,
            R17         =   0x0021,
            R18         =   0x0022,
            R19         =   0x0023,
            R20         =   0x0024,
            R21         =   0x0025,
            R22         =   0x0026,
            R23         =   0x0027,
            R24         =   0x0028,
            R25         =   0x0029,
            R26         =   0x002A,
            R27         =   0x002B,
            R28         =   0x002C,
            R29         =   0x002D,
            R30         =   0x002E,
            R31         =   0x002F,

            Count
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
            ReadStringInput        =    0x0006,
            ReadIntInput           =    0x0007,
            PrintCharToConsole     =    0x0008,
            Sleep                  =    0x0009,
            GetRunningTime         =    0x000A,
            TimeDiff               =    0x000B,
            RefreshScreen          =    0x000C,
            SetVideoMode           =    0x000D,
            PlotChar               =    0x000E,
            GetScreenW             =    0x000F,
            GetScreenH             =    0x0010,
            Draw                   =    0x0011,
            Random                 =    0x0012,
            GetAsyncKey            =    0x0013,

            p_TimeDiff_load        =    0xFFFF,
            p_TimeDiff_calc        =    0xFFFE
        };
        enum class eExitCodes
        {
            ForceQuit           =   0xA000,
            ExitSuccess         =   0x000F
        };
        enum class eVersion //TODO: Remove definitions from header
        {
            Major = __MAJOR_VER__,
            Minor = __MINOR_VER__,
            Build = __BUILD_NUMBER__
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
		enum class eBit
		{
			Low         = 0b0000000000000000,
			High        = 0b1111111111111111,
			One         = 0b0000000000000001,
			Two         = 0b0000000000000010,
			Three       = 0b0000000000000100,
			Four        = 0b0000000000001000,
			Five        = 0b0000000000010000,
			Six         = 0b0000000000100000,
			Seven       = 0b0000000001000000,
			Eight       = 0b0000000010000000,
			Nine        = 0b0000000100000000,
			Ten         = 0b0000001000000000,
			Eleven      = 0b0000010000000000,
			Twelve      = 0b0000100000000000,
			Thirteen    = 0b0001000000000000,
			Fourteen    = 0b0010000000000000,
			Fifteen     = 0b0100000000000000,
			Sixteen     = 0b1000000000000000,

			Count		= 18
		};
        enum class eBitMasks
        {
            MSB = 0b1111111100000000,
            LSB = 0b0000000011111111,
            Invert = 0xFFFF
        };
        enum class eMsgType
        {
            Info = 0,
            Success,
            Error,
            Special,
            Warning,
            Version
        };
        enum class eGuiBlock
        {
            None = 0,
            Heap,
            Stack,
            Registers,
            Libraries,
            Code,
            Output,
            Source,
            CallTree,
            Log
        };
        enum class eGuiBlockPosition
        {
            Top = 0,
            Middle,
            Bottom,
            Extra
        };
        enum class eDebuggerMode
        {
            Normal = 0,
            StepByStep
        };
        enum class eTimeUnits
        {
            Seconds = 0,
            Milliseconds,
            Microseconds,
            Nanoseconds
        };
        enum class eVideoModes
        {
            Console = 0,
            AsciiGrid
        };
        enum class eOasmColors
        {
            Grey = 0,
            Red,
            Green,
            Yellow,
            Blue,
            Magenta,
            Cyan,
            White,
	        BrightGrey,
            BrightRed,
            BrightGreen,
            BrightYellow,
            BrightBlue,
            BrightMagenta,
            BrightCyan,
            BrightWhite
        };
        enum class eExternSymType
        {
            Invalid = 0,
            Reserve,
            SubRoutine,
            Array
        };


        class Protectable
        {
            public:
                inline Protectable(void) { m_safe_mode = true; }
                inline bool protectedMode(void) const { return m_safe_mode; }
            public: //TODO: Set back to private, very important
                inline virtual void enableProtectedMode(void) { m_safe_mode = true; }
                inline virtual void disableProtectedMode(void) { m_safe_mode = false; }
                inline virtual void setProtectedMode(bool __pm) { m_safe_mode = __pm; }
            protected:
                bool m_safe_mode;
        };
        class Identifiable
        {
            public:
                inline Identifiable(void) { m_uid = -1; }
                inline int getID(void) const { return m_uid; }
                inline void setID(int id) { m_uid = id; }
            private:
                int m_uid;
        };
        class Validable
        {
            public:
                inline Validable(void) { m_valid = true; }
                inline virtual bool isValid(void) const { return m_valid; }
                inline virtual bool isInvalid(void) const { return !isValid(); }
                inline virtual void invalidate(void) { m_valid = false; }
                inline virtual void validate(void) { m_valid = true; }
                inline virtual void setValid(bool valid) { m_valid = valid; }
            private:
                bool m_valid;
        };
		class StringBuilder
		{
			public:
				inline StringBuilder(void) { m_data = ""; }
				inline StringBuilder(OmniaString str) { m_data = str; }
				inline StringBuilder(int32 i) { m_data = OmniaString::intToStr(i); }
				inline StringBuilder(int64 i) { m_data = OmniaString::intToStr(i); }
				inline StringBuilder(float f) { m_data = OmniaString::floatToStr(f); }
				inline StringBuilder(char c) { m_data = OmniaString().add(c); }
				inline StringBuilder(StringBuilder& sb) { m_data = sb.get(); }
				
				inline StringBuilder& add(OmniaString str) { m_data = m_data.add(str); return *this; }
				inline StringBuilder& add(int32 i) { m_data = m_data.addInt(i); return *this; }
				inline StringBuilder& add(int64 i) { m_data = m_data.addInt(i); return *this; }
				inline StringBuilder& add(float f) { m_data = m_data.add(OmniaString::floatToStr(f)); return *this; }
				inline StringBuilder& add(char c) { m_data = m_data.add(c); return *this; }
				inline StringBuilder& add(StringBuilder& sb) { m_data = sb.get(); return *this; }

				inline OmniaString get(void) { return m_data; }

			private:
				OmniaString m_data;
		};
        class Utils
        {
            public:
                static inline const OmniaString getVerionString(void) { return Utils::VERSION_STR; }
                static inline const uint64 getStartTime(void) { return Utils::s_startTime_ms; }
                static void init(void);
                static bool isHex(OmniaString hex);
                static bool isBin(OmniaString bin);
                static bool isInt(OmniaString str);
                static int32 strToInt(OmniaString str);
                static bool readFile(OmniaString fileName, std::vector<OmniaString>& lines);
                static OmniaString replaceAllVarName(OmniaString str, OmniaString search, OmniaString replace);
                static OmniaString intToHexStr(word i, bool prefix = true);
                static OmniaString intToBinStr(word i, bool prefix = true);
                static OmniaString duplicateChar(unsigned char c, uint16 count);
                static void printMemoryBlock(MemAddress start, MemAddress end, OmniaString text, OutputManager& out, MemAddress highlight = oasm_nullptr, uint8 inst_len = 0 );
                static void printRegisters(OutputManager& out);
                static OmniaString mapInstruction(eInstructionSet __inst);
                static OmniaString mapAddressingMode(eAddressingModes __mode);
                static OmniaString mapMaskParam(word __m_param);
                static OmniaString mapRegister(word __reg);
                static void get_terminal_size(int& width, int& height);
                static void sleep(uint32 __time, eTimeUnits __unit = eTimeUnits::Milliseconds);
                static uint64 getRunningTime_ms(void);
                static void hideCursor(bool hide = true);
                static void moveConsoleCursor(int x, int y);
                static void message(OmniaString __msg, OutputManager& out, eMsgType __type = eMsgType::Info);
                static void printTitle(OmniaString __title, OutputManager& out, word __line_length);

            private:
                inline static OmniaString VERSION_STR = "";
                inline static uint64 s_startTime_ms;
        };
	}
}

#endif
