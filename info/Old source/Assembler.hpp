#ifndef __ASSEMBLER__H__
#define __ASSEMBLER__H__

#include "Types.hpp"
#include "IOManager.hpp"

namespace Omnia
{
    namespace oasm
    {
        class PendingLabel
        {
            public:
                inline PendingLabel(String n, MemAddress a, _uint32 l, String ll) { name = n; addr = a; lineN = l; line = ll; pending = true; }

                String name;
                MemAddress addr;
                bool pending;
                _uint32 lineN;
                String line;
        };

        class PreProcessorOptions
        {
            public:
                inline PreProcessorOptions(void)
                {
                    passes = 2;
                }

                _uint16 passes;
        };

        class Macro
        {
            public:
                inline Macro(void) { valid = false; }
                Macro(String line);
                String expand(String line);
                inline bool isValid(void) { return valid; }

            
            public:
                std::vector<String> params;
                String expansion;
                String name;
            private:
                bool valid;
        };

        class PreProcessor : public IOReciever
        {
            public:
                inline static PreProcessor& instance(void) { return s_instance; }
                std::vector<String> open(String fileName, PreProcessorOptions options = PreProcessorOptions());
                inline bool hasAlias(String alias) { return (m_aliases.count(alias.cpp()) != 0); }

            private:
                inline PreProcessor(void) {  }

            private:
                std::vector<String> process(std::vector<String> lines, PreProcessorOptions options);
                std::vector<String> resolveIncludes(std::vector<String> mainFile, String curFile);
                std::vector<String> resolveIncludes_r(std::vector<String> mainFile, String curFile);
                std::vector<String> removeComments(std::vector<String> lines);
                std::vector<String> resolveAliases(std::vector<String> lines);
                std::vector<String> resolveMacros(std::vector<String> lines);
                void error(ePreProcessorErrors err, String msg, bool skipFileInfo = false);

            private:
                String _line;
                _uint32 lineNumber;
                String currentFile;
                std::vector<String> m_includeGuards;
                std::map<_string, _string> m_aliases;
                PreProcessorOptions m_options;
                std::vector<Macro> m_macros;

            public:
                static PreProcessor s_instance;

        };

        inline PreProcessor PreProcessor::s_instance;

        class Assembler : public IOReciever
        {
            public:
                inline static Assembler& instance(void) { return s_instance; }
                std::vector<_int32> assemble(std::vector<String> lines);
                std::vector<_int32> assembleFromFile(String fileName);
                bool assembleToFile(String inputFile, String outputFile);

            private:
                inline Assembler(void) { init(); }
                void init(void);
                void error(eAssemblerErrors err, String msg, bool skipFileInfo = false);
                bool isRegister(String vname, eRegisters& reg);
                eRegisters regFromInt(_uint16 i);
                bool isInstruction(String inst, eInstructionSet& ins, _int32& flags);
                bool isDirective(String dir, eAssemblerDirectives& directive);
                void findLabel(String op, std::vector<_int32>& code, std::map<_string, MemAddress>& labels);
                void makeLabel(String op, std::vector<_int32>& code, std::map<_string, MemAddress>& labels);
                void resolvePendingLabels(String lab, std::vector<_int32>& code, std::map<_string, MemAddress>& labels);
                std::vector<_int32> buildCodeStart(std::vector<String> lines);
                inline bool isVariable(String name) { name = name.substr(1).trim(); return m_allocs.count(name.cpp()) != 0; }
                bool parseOperand(String op, std::vector<_int32>& val, _int32& flg, eByte& byte, eByte flgByte = eByte::All, eOperandType* opType = nullptr, _int16* _off = nullptr);
                void setVariables(std::vector<String>& lines);

            private:
                std::map<_string, MemAddress> m_allocs;
                MemAddress m_nextAlloc;
                String _line;
                _uint32 lineNumber;
                std::vector<PendingLabel> m_pendingLabels;
                std::vector<_int32> m_initCode;
                bool m_debugBuild;
                
            public:
                static Assembler s_instance;

                friend class Disassembler;
        };

        inline Assembler Assembler::s_instance;

        class Disassembler : public IOReciever
        {
            public:
                inline static Disassembler& instance(void) { return s_instance; }
                std::vector<String> disassemble(std::vector<_int32>& code, bool print = true);
                bool parseInstruction(MemAddress addr, std::vector<_int32>& code, std::vector<String>& lines, bool print = true);

            private:
                inline Disassembler(void) {  }

            private:
                static Disassembler s_instance;
        };
    }
}

#endif