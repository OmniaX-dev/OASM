#include "Assembler.hpp"
#include "StringTokens.hpp"
#include <fstream>

namespace Omnia
{
    namespace oasm
    {
        std::vector<String> PreProcessor::open(String fileName, PreProcessorOptions options)
        {
            std::vector<String> lines;
            if (!Utils::readFile(fileName, lines))
            {
                error(ePreProcessorErrors::FailedToOpenFile, String("Failed to open source file: ").add(fileName), true);
                return std::vector<String>();
            }
            if (lines.size() == 0)
            {
                error(ePreProcessorErrors::EmptyFile, String("Empty source file provided: ").add(fileName), true);
                return std::vector<String>();
            }
            currentFile = fileName;
            return process(lines, options);
        }

        std::vector<String> PreProcessor::process(std::vector<String> lines, PreProcessorOptions options)
        {
            m_options = options;
            std::vector<String> finalCode = resolveIncludes(lines, currentFile);
            finalCode = removeComments(finalCode);
            finalCode = resolveAliases(finalCode);
            finalCode = resolveMacros(finalCode);
            return finalCode;
        }

        std::vector<String> PreProcessor::resolveAliases(std::vector<String> lines)
        {
            std::vector<String> code;
            lineNumber = 0;
            currentFile = "Processed_file";
            for (auto& l : lines) //Aliases round
            {
                _line = l;
                lineNumber++;
                l = l.trim();
                if (!l.toLowerCase().startsWith(".alias "))
                {
                    code.push_back(l);
                    continue;
                }
                String line = l.substr(7).trim();
                if (!line.contains("="))
                {
                    error(ePreProcessorErrors::WrongAliasDir, "Wrong .alias directive.");
                    return std::vector<String>();
                }
                String alias = line.substr(0, line.indexOf("=")).trim();
                line = line.substr(line.indexOf("=") + 1).trim();
                m_aliases[String("#").add(alias).cpp()] = line.cpp();
            }
            for (_uint16 i = 0; i < m_options.passes; i++)
            {
                for (auto& line : code) //Replacing aliases
                {
                    for (auto& al : m_aliases)
                        line = Utils::replaceAllVarName(line, String(al.first), String(al.second));
                }
            }
            return code;
        }

        std::vector<String> PreProcessor::resolveMacros(std::vector<String> lines)
        {
            std::vector<String> code;
            lineNumber = 0;
            currentFile = "Processed_file";
            for (auto& l : lines) //Macros round
            {
                _line = l;
                lineNumber++;
                l = l.trim();
                if (!l.toLowerCase().startsWith(".macro "))
                {
                    code.push_back(l);
                    continue;
                }
                String line = l.substr(7).trim();
                Macro mac(line);
                if (!mac.isValid())
                {
                    //TODO: Error
                    return std::vector<String>();
                }
                m_macros.push_back(mac);
            }
            for (_uint16 i = 0; i < m_options.passes; i++)
            {
                _int32 l = 0;
                for (auto& line : code) //Replacing macros
                {
                    for (auto& m : m_macros)
                    {
                        String mname = String("@").add(m.name);
                        mname = mname.add("(");
                        bool found = false;
                        while (line.contains(mname))
                        {
                            _int32 cp = line.indexOf(")");
                            _int32 ep = line.indexOf(mname);
                            if (ep + mname.length() > cp) break;
                            String plist = line.substr(ep + mname.length(), cp).trim();
                            String line_p1 = line.substr(0, ep);
                            String line_p2 = line.substr(cp + 1);
                            line = line_p1.add(m.expand(plist));
                            line = line.add(line_p2);
                            found = true;
                        }
                        if (!found) continue;
                        StringTokens st = line.tokenize("\"&#\"");
                        if (st.count() < 2) continue;
                        _int32 el = 0;
                        line = "";
                        while (st.hasNext())
                            code.insert(code.begin() + (l + el++), st.next());
                    }
                    l++;
                }
            }
            return code;
        }

        std::vector<String> PreProcessor::removeComments(std::vector<String> lines)
        {
            std::vector<String> code;
            bool multiLineComment = false, newRound = false;
            for (auto& line : lines)
            {
                line = line.trim();
                if (line.startsWith("//") && !multiLineComment) continue;
                if (line.contains("//") && !multiLineComment) line = line.substr(0, line.indexOf("//")).trim();
                if (line.contains("/*") && !multiLineComment)
                {
                    if (line.contains("*/"))
                    {
                        String lp1 = line.substr(0, line.indexOf("/*"));
                        String lp2 = line.substr(line.indexOf("*/") + 2);
                        line = lp1.add(lp2);
                    }
                    else
                    {
                        line = line.substr(0, line.indexOf("/*")).trim();
                        if (line.trim() != "") code.push_back(line);
                        multiLineComment = true;
                    }
                }
                else if (line.contains("*/") && multiLineComment)
                {
                    line = line.substr(line.indexOf("*/") + 2);
                    multiLineComment = false;
                }
                if (multiLineComment) continue;
                if (line.contains("/*")) newRound = true;
                if (line.trim() != "") code.push_back(line);
            }
            if (newRound) code = removeComments(code);
            return code;
        }

        std::vector<String> PreProcessor::resolveIncludes(std::vector<String> mainFile, String curFile)
        {
            std::vector<String> ilines = resolveIncludes_r(mainFile, curFile);
            std::vector<String> nlines;
            bool include_skip = false;
            _uint32 grdCount = 0;
            for (auto& line : ilines)
            {
                line = line.trim();
                if (line == "") continue;
                if (line.toLowerCase().startsWith(".include_guard(") && line.endsWith(")"))
                {
                    if (include_skip)
                    {
                        grdCount++;
                        continue;
                    }
                    line = line.substr(line.indexOf("(") + 1, line.indexOf(")")).trim();
                    if (std::find(m_includeGuards.begin(), m_includeGuards.end(), line) != m_includeGuards.end())
                    {
                        include_skip = true;
                        grdCount++;
                        continue;
                    }
                    m_includeGuards.push_back(line);
                    continue;
                }
                else if (line.toLowerCase() == ".close_include_guard")
                {
                    if (!include_skip) continue;
                    grdCount--;
                    if (grdCount > 0)
                        continue;
                    else
                    {
                        include_skip = false;
                        grdCount = 0;
                        continue;
                    }
                }
                if (include_skip) continue;
                nlines.push_back(line);
            }
            return nlines;
        }
        
        std::vector<String> PreProcessor::resolveIncludes_r(std::vector<String> mainFile, String curFile)
        {
            _line = "";
            lineNumber = 0;
            currentFile = curFile;
            std::vector<String> incl;
            std::vector<String> tmp;
            for (auto& l : mainFile) //Includes round
            {
                _line = l;
                lineNumber++;
                if (!l.toLowerCase().trim().startsWith(".include "))
                {
                    tmp.push_back(l);
                    continue;
                }
                String line = l.substr(9).trim();
                if (!line.startsWith("[") || !line.endsWith("]"))
                {
                    error(ePreProcessorErrors::WrongInclDirective, "Wrong .include directive.", true);
                    return std::vector<String>();
                }
                line = line.substr(1, line.length() - 1).trim();
                if (!Utils::readFile(line, incl))
                {
                    error(ePreProcessorErrors::FailedToOpenFile, String("File in .include directive could not be found.").add(line), true);
                    return std::vector<String>();
                }
                if (incl.size() > 0)
                {
                    std::vector<String> nincl = resolveIncludes_r(incl, line);
                    tmp.insert(tmp.end(), nincl.begin(), nincl.end());
                }
            }
            return tmp;
        }
        
        void PreProcessor::error(ePreProcessorErrors err, String msg, bool skipFileInfo)
        {
            m_out->newLine().print("PreProcessor Error ").print((_uint32)err).newLine().print(msg).newLine();
            if (!skipFileInfo)
            {
                m_out->print("File: ").print(currentFile).newLine();
                m_out->print("Line ").print(lineNumber + 1).newLine();
                m_out->tab().print(_line).newLine();
            }
        }


        Macro::Macro(String line)
        {
            valid = false;
            line = line.trim();
            if (line == "") return;
            if (!line.contains("(") || !line.contains(")")) return;
            name = line.substr(0, line.indexOf("(")).trim();
            if (name == "") return;
            String p = line.substr(line.indexOf("(") + 1, line.indexOf(")")).trim();
            expansion = line.substr(line.indexOf(")") + 1).trim();
            if (expansion == "") return;
            if (p != "")
            {
                StringTokens st = p.tokenize(",", true);
                while (st.hasNext()) params.push_back(st.next());
            }
            valid = true;
        }

        String Macro::expand(String line)
        {
            if (!isValid()) return "";
            if (line == "" || expansion == "") return "";
            String exp = expansion.trim();
            StringTokens st = line.tokenize(",", true);
            for (auto param : params)
            {
                if (!st.hasNext()) return "";
                String p = String("$(").add(param);
                p = p.add(")");
                exp = exp.replaceAll(p, st.next());
            }
            return exp.trim();
        }


        bool Assembler::assembleToFile(String inputFile, String outputFile)
        {
            std::vector<_int32> code = assembleFromFile(inputFile);
            if (code.size() == 0)
            {
                m_out->print("Error compiling.").newLine(); //TODO: Error
                return false;
            }
            std::ofstream writeFile;
            writeFile.open(outputFile.cpp(), std::ios::out | std::ios::binary);
            writeFile.write((char*)(&code[0]), code.size() * sizeof(_int32));
            return true;
        }

        void Assembler::error(eAssemblerErrors err, String msg, bool skipFileInfo)
        {
            m_out->newLine().print("Assembler Error ").print((_uint32)err).newLine().print(msg).newLine();
            if (!skipFileInfo)
            {
                m_out->print("Line ").print(lineNumber + 1).newLine();
                m_out->tab().print(_line).newLine();
            }
        }

        std::vector<_int32> Assembler::assembleFromFile(String fileName)
        {
            std::vector<String> lines = PreProcessor::instance().open(fileName);
            if (lines.size() == 0) return std::vector<_int32>();
            return assemble(lines);
        }

        eRegisters Assembler::regFromInt(_uint16 i)
        {
            switch (i)
            {
            case 0: return eRegisters::R0;
            case 1: return eRegisters::R1;
            case 2: return eRegisters::R2;
            case 3: return eRegisters::R3;
            case 4: return eRegisters::R4;
            case 5: return eRegisters::R5;
            case 6: return eRegisters::R6;
            case 7: return eRegisters::R7;
            case 8: return eRegisters::R8;
            case 9: return eRegisters::R9;
            case 10: return eRegisters::R10;
            case 11: return eRegisters::R11;
            case 12: return eRegisters::R12;
            case 13: return eRegisters::R13;
            case 14: return eRegisters::R14;
            case 15: return eRegisters::R15;
            case 16: return eRegisters::R16;
            case 17: return eRegisters::R17;
            case 18: return eRegisters::R18;
            case 19: return eRegisters::R19;
            case 20: return eRegisters::R20;
            case 21: return eRegisters::R21;
            case 22: return eRegisters::R22;
            case 23: return eRegisters::R23;
            default: return eRegisters::NP;
            }
        }

        bool Assembler::isInstruction(String inst, eInstructionSet& ins, _int32& flags)
        {
            inst = inst.toLowerCase().trim();
            ins = eInstructionSet::Invalid;
            if (inst.startsWith("j"))
            {
                if (inst == "jmp")
                {
                    ins = eInstructionSet::jmp;
                    flags = Utils::setb(flags, eByte::One, (_ubyte)eFlags::JumpUnconditional);
                }
                else if (inst == "jeq")
                {
                    ins = eInstructionSet::jmp;
                    flags = Utils::setb(flags, eByte::One, (_ubyte)eFlags::JumpEquals);
                }
                else if (inst == "jne")
                {
                    ins = eInstructionSet::jmp;
                    flags = Utils::setb(flags, eByte::One, (_ubyte)eFlags::JumpNotEquals);
                }
                else if (inst == "jl")
                {
                    ins = eInstructionSet::jmp;
                    flags = Utils::setb(flags, eByte::One, (_ubyte)eFlags::JumpLess);
                }
                else if (inst == "jg")
                {
                    ins = eInstructionSet::jmp;
                    flags = Utils::setb(flags, eByte::One, (_ubyte)eFlags::JumpGreater);
                }
                else if (inst == "jge")
                {
                    ins = eInstructionSet::jmp;
                    flags = Utils::setb(flags, eByte::One, (_ubyte)eFlags::JumpGreaterEquals);
                }
                else if (inst == "jle")
                {
                    ins = eInstructionSet::jmp;
                    flags = Utils::setb(flags, eByte::One, (_ubyte)eFlags::JumpLssEquals);
                }
            }
            else if (inst == "mem") ins = eInstructionSet::mem;
            else if (inst == "com") ins = eInstructionSet::com;
            else if (inst == "inc") ins = eInstructionSet::inc;
            else if (inst == "cmp") ins = eInstructionSet::cmp;
            else if (inst == "cat") ins = eInstructionSet::cat;
            else if (inst == "dec") ins = eInstructionSet::dec;
            else if (inst == "add") ins = eInstructionSet::add;
            else if (inst == "sub") ins = eInstructionSet::sub;
            else if (inst == "mul") ins = eInstructionSet::mul;
            else if (inst == "div") ins = eInstructionSet::div;
            else if (inst == "mov") ins = eInstructionSet::mov;
            else if (inst == "flg") ins = eInstructionSet::flg;
            else if (inst == "call") ins = eInstructionSet::call;
            else if (inst == "push") ins = eInstructionSet::push;
            else if (inst == "pop") ins = eInstructionSet::pop;
            else if (inst == "ret") ins = eInstructionSet::ret;
            else if (inst == "end") ins = eInstructionSet::end;
            else if (inst == "req_local") ins = eInstructionSet::req_local;
            else if (inst == "req_heap") ins = eInstructionSet::req_heap;
            else if (inst == "req_stack") ins = eInstructionSet::req_stack;
            else if (inst == "alloc_h") ins = eInstructionSet::alloc_h;
            else if (inst == "free_h") ins = eInstructionSet::free_h;
            else if (ins == eInstructionSet::Invalid) return false;
            return true;
        }

        bool Assembler::isDirective(String dir, eAssemblerDirectives& directive)
        {
            dir = dir.trim().toLowerCase();
            directive = eAssemblerDirectives::Invalid;
            if (!dir.startsWith(".")) return false;
            dir = dir.substr(1).trim();
            if (dir == "include") directive = eAssemblerDirectives::Include;
            if (dir == "include_guard") directive = eAssemblerDirectives::IncludeGuard;
            if (dir == "close_include_guard") directive = eAssemblerDirectives::CloseIncludeGuard;
            if (dir == "sub_routine") directive = eAssemblerDirectives::SubRoutine;
            if (dir == "close_sub_routine") directive = eAssemblerDirectives::CloseSubRoutine;
            if (dir == "alias") directive = eAssemblerDirectives::Alias;
            if (dir == "label") directive = eAssemblerDirectives::Label;
            if (dir == "low") directive = eAssemblerDirectives::Low;
            if (dir == "var") directive = eAssemblerDirectives::Var;
            if (dir.startsWith("memory_setup")) directive = eAssemblerDirectives::MemorySetup;
            if (directive == eAssemblerDirectives::Invalid) return false;
            return true;
        }

        bool Assembler::isRegister(String vname, eRegisters& reg)
        {
            for (_uint16 i = 0; i < 24; i++)
            {
                String tmp = "%r";
                tmp = tmp.addInt(i);
                if (tmp.trim() == vname.toLowerCase().trim())
                {
                    reg = regFromInt(i);
                    return reg != eRegisters::NP;
                }
            }
            if (vname.toLowerCase().trim() == "%np")
            {
                reg = eRegisters::NP;
                return true;
            }
            if (vname.toLowerCase().trim() == "%ip")
            {
                reg = eRegisters::IP;
                return true;
            }
            if (vname.toLowerCase().trim() == "%dr")
            {
                reg = eRegisters::DR;
                return true;
            }
            if (vname.toLowerCase().trim() == "%rv")
            {
                reg = eRegisters::RV;
                return true;
            }
            if (vname.toLowerCase().trim() == "%cf")
            {
                reg = eRegisters::CF;
                return true;
            }
            if (vname.toLowerCase().trim() == "%fl")
            {
                reg = eRegisters::FL;
                return true;
            }
            if (vname.toLowerCase().trim() == "%es")
            {
                reg = eRegisters::ES;
                return true;
            }
            if (vname.toLowerCase().trim() == "%sp")
            {
                reg = eRegisters::SP;
                return true;
            }
            return false;
        }

        void Assembler::findLabel(String op, std::vector<_int32>& code, std::map<_string, MemAddress>& labels)
        {
            if (labels.count(op.cpp()) == 0)
            {
                m_pendingLabels.push_back(PendingLabel(op, code.size(), lineNumber, _line));
                code.push_back(0);
            }
            else
                code.push_back((_int32)labels[op.cpp()]);
        }

        void Assembler::makeLabel(String op, std::vector<_int32>& code, std::map<_string, MemAddress>& labels)
        {
            labels[op.cpp()] = code.size();
            resolvePendingLabels(op, code, labels);
        }
        
        void Assembler::resolvePendingLabels(String lab, std::vector<_int32>& code, std::map<_string, MemAddress>& labels)
        {
            for (auto& pl : m_pendingLabels)
            {
                if (pl.pending && pl.name == lab)
                {
                    code[pl.addr] = (_int32)labels[lab.cpp()];
                    pl.pending = false;
                }
            }
        }

        void Assembler::init(void)
        {
            m_nextAlloc = (MemAddress)eRegisters::MemStart;
            m_allocs.clear();
            _line = "";
            lineNumber = 0;
        }

        std::vector<_int32> Assembler::buildCodeStart(std::vector<String> lines)
        {
            std::vector<_int32> code;
            bool found = false;
            _int32 stackSize = 0;
            _int32 localSize = 0;
            _int32 heapSize = 0;
            code.push_back(0); //allocating 1 memory cell for code size
            for (_uint32 i = 0; i < lines.size(); i++)
            {
                _line = lines[i];
                lineNumber = i + 1;
                String line = lines[i].trim().toLowerCase();
                if (!line.startsWith(".memory_setup")) continue;
                found = true;
                line = line.substr(13).trim();
                if (!line.startsWith("(") || !line.endsWith(")"))
                {
                    //TODO: Error
                    return EMPTY_CODE;
                }
                line = line.substr(1, line.length() - 1).trim();
                StringTokens st = line.tokenize(",", true);
                if (st.count() != 3)
                {
                    //TODO: Error
                    return EMPTY_CODE;
                }
                while (st.hasNext())
                {
                    line = st.next();
                    if (!line.contains("="))
                    {
                        //TODO: Error
                        return EMPTY_CODE;
                    }
                    String mem = line.substr(0, line.indexOf("=")).trim();
                    line = line.substr(line.indexOf("=") + 1).trim();
                    if (!Utils::isInt(line))
                    {
                        //TODO: Error
                        return EMPTY_CODE;
                    }
                    if (mem == "stack") stackSize = Utils::strToInt(line);
                    else if (mem == "heap") heapSize = Utils::strToInt(line);
                    else if (mem == "local") localSize = Utils::strToInt(line);
                    else
                    {
                        //TODO: Error
                        return EMPTY_CODE;
                    }
                }
            }
            if (!found)
            {
                //TODO: Error
                return EMPTY_CODE;
            }
            code.push_back((_int32)eInstructionSet::req_local);
            code.push_back(localSize);
            code.push_back((_int32)eInstructionSet::req_heap);
            code.push_back(heapSize);
            code.push_back((_int32)eInstructionSet::req_stack);
            code.push_back(stackSize);
            code.push_back(0); //allocating 1 memory cell for entry point
            if (m_debugBuild) code.push_back(1); //TODO: More specific debug options
            else code.push_back(0);
            lineNumber = 0;
            return code;
        }

        void Assembler::setVariables(std::vector<String>& lines)
        {
            m_initCode.clear();
            StringTokens st;
            String token = "";
            for (auto& line : lines)
            {
                line = line.trim();
                if (!line.toLowerCase().startsWith(".var ")) continue;
                line = line.substr(5).trim();
                st = line.tokenize(",", true);
                while (st.hasNext())
                {
                    token = st.next();
                    String val = "";
                    bool assign = false;
                    if (token.contains("="))
                    {
                        String vname = token.substr(0, token.indexOf("=")).trim();
                        val = token.substr(token.indexOf("=") + 1).trim();
                        token = vname;
                        assign = true;
                    }
                    if (!isVariable(String("$").add(token.trim()))) m_allocs[token.cpp()] = m_nextAlloc++;
                    else
                    {
                        //TODO: Error, variable redeclaration
                        return;
                    }
                    if (assign)
                    {
                        _int32 flg = 0;
                        eByte byte;
                        std::vector<_int32> _val;
                        if (!parseOperand(val, _val, flg, byte, eByte::One, nullptr))
                        {
                            //TODO: Error
                            return;
                        }
                        m_initCode.push_back((_int32)eInstructionSet::mem);
                        m_initCode.push_back(flg);
                        m_initCode.push_back(0); //TODO: Offset param
                        m_initCode.push_back(m_nextAlloc - 1);
                        m_initCode.insert(m_initCode.end(), _val.begin(), _val.end());
                    }
                }
                line = "";
            }
        }

        bool Assembler::parseOperand(String op, std::vector<_int32>& val, _int32& flg, eByte& byte, eByte flgByte, eOperandType* opType, _int16* _off)
        {
            val.clear();
            op = op.trim();
            byte = eByte::All;
            if (op.contains(" ") && !op.endsWith("\"") && !op.endsWith("'"))
            {
                String opf = op.substr(0, op.indexOf(" ")).trim();
                String opd = op.substr(op.indexOf(" ") + 1).trim();
                if (!Utils::isInt(opf))
                {
                    error(eAssemblerErrors::FlgValNumeric, "Invaòid flag value: must be numeric.");
                    if (opType != nullptr) *opType = eOperandType::Invalid;
                    return false;
                }
                op = opd;
                byte = (eByte)Utils::strToInt(opf);
            }
            String off_param = "";
            if (op.indexOf("[") > 0 && op.indexOf("[") == op.lastIndexOf("["))
            {
                _uint32 io = op.indexOf("[");
                String tmp = op.substr(io).trim();
                if (tmp.endsWith("]"))
                    off_param = tmp.substr(1, tmp.length() - 1).trim();
                if (off_param != "") op = op.substr(0, io).trim();
                m_out->print(off_param).newLine().print(" ").print(op).newLine();
            }
            if (Utils::isInt(op))
            {
                val.push_back(Utils::strToInt(op));
                flg = Utils::setb((_int32)flg, flgByte, (_ubyte)eFlags::NormalAssignment);
                if (opType != nullptr) *opType = eOperandType::Integer;
                return true;
            }
            if (isVariable(op))
            {
                val.push_back((_int32)m_allocs[op.substr(1).trim().cpp()]);
                flg = Utils::setb((_int32)flg, flgByte, (_ubyte)eFlags::IndirectAssignment);
                if (opType != nullptr) *opType = eOperandType::Variable;
                if (_off != nullptr && off_param != "")
                {
                    std::vector<_int32> _val;
                    _int32 _flg = 0;
                    eByte _by;
                    eOperandType _opt;
                    if (!parseOperand(off_param, _val, _flg, _by, eByte::All, &_opt, nullptr))
                    {
                        //TODO: Error
                        return false;
                    }
                    if (_opt != eOperandType::Variable && _opt != eOperandType::Register && _opt != eOperandType::Integer)
                    {
                        //TODO: Error
                        return false;
                    }
                    if (val.size() != 1)
                    {
                        //TODO: Error
                        return false;
                    }
                    *_off = (_int16)_val[0];
                    if (_opt == eOperandType::Integer) flg = Utils::setb(flg, eByte::Four, (_ubyte)eFlags::ConstantOffset);
                    else flg = Utils::setb(flg, eByte::Four, (_ubyte)eFlags::VariableOffset);
                }
                return true;
            }
            eRegisters reg;
            if (isRegister(op, reg))
            {
                val.push_back((_int32)reg);
                flg = Utils::setb((_int32)flg, flgByte, (_ubyte)eFlags::IndirectAssignment);
                if (opType != nullptr) *opType = eOperandType::Register;
                return true;
            }
            if (op.startsWith("'") && op.endsWith("'") && op.length() == 3)
            {
                val.push_back((_int32)(op[1]));
                flg = Utils::setb((_int32)flg, flgByte, (_ubyte)eFlags::NormalAssignment);
                if (opType != nullptr) *opType = eOperandType::Char;
                return true;
            }
            if (op.startsWith("\"") && op.endsWith("\""))
            {
                op = op.substr(1, op.length() - 1);
                val = Utils::strToMemCells(op.add('\0'));
                val.insert(val.begin(), (_int32)op.length());
                flg = Utils::setb((_int32)flg, eByte::One, (_ubyte)eFlags::NormalAssignment);
                flg = Utils::setb((_int32)flg, eByte::Two, (_ubyte)eByte::All);
                flg = Utils::setb((_int32)flg, eByte::Three, (_ubyte)eByte::All);
                flg = Utils::setb((_int32)flg, eByte::Four, (_ubyte)eFlags::StringAssign);
                if (opType != nullptr) *opType = eOperandType::String;
                return true;
            }
            if (op.startsWith("[") && op.endsWith("]"))
            {
                op = op.substr(1, op.length() - 1);
                if (isVariable(op))
                {
                    val.push_back(m_allocs[op.substr(1).trim().cpp()]);
                    flg = Utils::setb((_int32)flg, flgByte, (_ubyte)eFlags::DereferenceAssignment);
                    if (opType != nullptr) *opType = eOperandType::VariablePointer;
                    return true;
                }
                if (opType != nullptr) *opType = eOperandType::Invalid;
                return false;
            }
            if (op.startsWith("*"))
            {
                op = op.substr(1).trim();
                if (isVariable(op))
                {
                    val.push_back(m_allocs[op.substr(1).trim().cpp()]);
                    flg = Utils::setb((_int32)flg, flgByte, (_ubyte)eFlags::ReferenceAssign);
                    if (opType != nullptr) *opType = eOperandType::VariableRef;
                    return true;
                }
                eRegisters reg;
                if (isRegister(op, reg))
                {
                    val.push_back((_int32)reg);
                    flg = Utils::setb((_int32)flg, flgByte, (_ubyte)eFlags::ReferenceAssign);
                    if (opType != nullptr) *opType = eOperandType::RegisterRef;
                    return true;
                }
                if (opType != nullptr) *opType = eOperandType::Invalid;
                return false;
            }
            if (opType != nullptr) *opType = eOperandType::Invalid;
            return false;
        }

        std::vector<_int32> Assembler::assemble(std::vector<String> lines)
        {
            init();
            std::vector<_int32> code = buildCodeStart(lines);
            if (code.size() == 0) return EMPTY_CODE;
            std::map<_string, MemAddress> labels;
            bool open_subroutine = false;
            String sr_name = "";
            MemAddress sr_jmp_addr = 0;
            _uint32 grdCount = 0;
            _uint32 i = lineNumber;
            MemAddress ep_addr = 0;
            String line = "";
            eAssemblerDirectives dir;
            eInstructionSet ins;
            lines.insert(lines.begin(), String(".var __oasm_version__ = ").addInt((_int32)eVersion::Iteration));
            setVariables(lines);
            for ( ; i < lines.size(); i++)
            {
                _int32 flags = 0, value = 0;
                line = lines[i].trim();
                _line = line;
                lineNumber = i;
                if (line == "") continue;
                String inst = "";
                if (line.contains(" "))
                {
                    inst = line.substr(0, line.indexOf(" ")).trim();
                    line = line.substr(line.indexOf(" ") + 1).trim();
                }
                else
                {
                    inst = line;
                    line = "";
                }
                if (isDirective(inst, dir))
                {
                    switch (dir)
                    {
                        case eAssemblerDirectives::Low:
                        {
                            std::vector<_int32> val;
                            _int32 tmp = 0;
                            eByte tmpb;
                            StringTokens st = line.tokenize(",", true);
                            while (st.hasNext())
                            {
                                String token = st.next();
                                isInstruction(token, ins, flags);
                                if (ins != eInstructionSet::Invalid)
                                    code.push_back((_int32)ins);
                                else if (parseOperand(token, val, tmp, tmpb))
                                    code.insert(code.end(), val.begin(), val.end());
                                else if (token.startsWith("."))
                                    findLabel(token.substr(1).trim(), code, labels);
                                else
                                {
                                    error(eAssemblerErrors::LowDirectiveInt, "Type error: Invalid operand for .low directive.");
                                    return std::vector<_int32>();
                                }
                            }
                        }
                        continue;
                        case eAssemblerDirectives::SubRoutine:
                        {
                            if (open_subroutine) continue; //TODO: Maybe error
                            code.push_back((_int32)eInstructionSet::jmp);
                            code.push_back((_int32)eFlags::JumpUnconditional);
                            code.push_back(0);
                            sr_jmp_addr = code.size() - 1;
                            makeLabel(line, code, labels);
                            sr_name = line;
                            open_subroutine = true;
                            if (sr_name == "__main")
                            {
                                ep_addr = (MemAddress)code.size();
                                if (m_initCode.size() > 0)
                                {
                                    for (_uint16 j = 0; j < m_initCode.size(); j++)
                                        code.push_back(m_initCode[j]);
                                }
                            }
                        }
                        continue;
                        case eAssemblerDirectives::CloseSubRoutine:
                        {
                            if (!open_subroutine) continue; //TODO: Maybe error
                            open_subroutine = false;
                            code[sr_jmp_addr] = code.size();
                            sr_jmp_addr = 0;
                            sr_name = "";
                        }
                        continue;
                        case eAssemblerDirectives::Label:
                        {
                            makeLabel(line, code, labels);
                        }
                        case eAssemblerDirectives::MemorySetup: continue;
                        default: continue;
                    }
                }
                else if (isInstruction(inst, ins, flags))
                {

                    bool derefDest = false;
                    switch (ins)
                    {
                        case eInstructionSet::mem:
                        case eInstructionSet::mov: //Done
                        {
                            if (line == "")
                            {
                                error(eAssemblerErrors::WrongMemInstruction, "Invalid <mem> instruction.");
                                return std::vector<_int32>();
                            }
                            String op1 = line.substr(0, line.indexOf("=")).trim();
                            String op2 = line.substr(line.indexOf("=") + 1).trim();
                            std::vector<_int32> val;
                            _int32 flg = 0;
                            eByte byte;
                            _int16 _off = 0;
                            if (!parseOperand(op1, val, flg, byte, eByte::All, nullptr, &_off))
                            {
                                //TODO: Error
                                std::cout << "err1 " << op1.cpp() << "\n";
                                return std::vector<_int32>();
                            }
                            if (val.size() != 1)
                            {
                                //TODO: Error
                                std::cout << "err2\n";
                                return std::vector<_int32>();
                            }
                            if (flg != (_int32)eFlags::IndirectAssignment && flg != (_int32)eFlags::ReferenceAssign)
                            {
                                //TODO: Error
                                std::cout << "err3\n";
                                return std::vector<_int32>();
                            }
                            if (flg == (_int32)eFlags::ReferenceAssign)
                            {
                                derefDest = true;
                                code.push_back((_int32)eInstructionSet::flg);
                                code.push_back((_int32)eFlags::DereferenceDestination);
                            }
                            code.push_back((_int32)ins);
                            MemAddress fladdr = (MemAddress)code.size();
                            code.push_back(0);
                            MemAddress ofaddr = (MemAddress)code.size();
                            _int32 of = 0;
                            of = Utils::sets(of, true, _off);
                            code.push_back(0); //TODO: Offset parameter
                            _int32 flags = 0;
                            flags = Utils::setb(flags, eByte::Two, (_ubyte)byte);
                            code.push_back(val[0]);
                            _off = 0;
                            if (!parseOperand(op2, val, flags, byte, eByte::One, nullptr, &_off))
                            {
                                //TODO: Error
                                std::cout << op2.cpp() << " err4\n";
                                return std::vector<_int32>();
                            }
                            if (val.size() != 1) code.insert(code.end(), val.begin(), val.end());
                            else
                            {
                                flags = Utils::setb(flags, eByte::Three, (_ubyte)byte);
                                code.push_back(val[0]);
                            }
                            if (derefDest)
                            {
                                code.push_back((_int32)eInstructionSet::flg);
                                code.push_back((_int32)eFlags::RestoreFlags);
                            }
                            of = Utils::sets(of, false, _off);
                            code[ofaddr] = of;
                            code[fladdr] = flags;
                        }
                        continue;
                        case eInstructionSet::com: //Done
                        {
                            bool param = true;
                            if (!line.contains(",")) param = false;
                            String com = "", par = "";
                            if (param)
                            {
                                com = line.substr(0, line.indexOf(",")).trim();
                                par = line.substr(line.indexOf(",") + 1).trim();
                            }
                            else
                                com = line;
                            if (!Utils::isInt(com))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            code.push_back((_int32)ins);
                            MemAddress fladdr = (MemAddress)code.size();
                            code.push_back(0);
                            _int32 _com = Utils::strToInt(com);
                            code.push_back(_com);
                            if (param)
                            {
                                std::vector<_int32> val;
                                eByte byte;
                                _int32 flg = 0;
                                if (!parseOperand(par, val, flg, byte))
                                {
                                    //TODO: Error
                                    return EMPTY_CODE;
                                }
                                if (val.size() != 1)
                                {
                                    //TODO: Error
                                    return EMPTY_CODE;
                                }
                                if (_com == (_int32)eComCodes::PrintIntToConsole)
                                    flags = Utils::setb(flags, eByte::Two, (_int32)byte);
                                code[fladdr] = flags;
                                code.push_back(val[0]);
                            }
                        }
                        continue;
                        case eInstructionSet::inc:
                        case eInstructionSet::dec: //Done
                        {
                            std::vector<_int32> val;
                            eByte byte;
                            if (!parseOperand(line, val, flags, byte, eByte::One))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (val.size() != 1)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            flags = Utils::setb(flags, eByte::Two, (_int32)byte);
                            code.push_back((_int32)ins);
                            code.push_back(flags);
                            code.push_back(val[0]);
                        }
                        continue;
                        case eInstructionSet::cmp: //Done
                        {
                            if (!line.contains(","))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            std::vector<_int32> val;
                            eByte byte;
                            String op1 = line.substr(0, line.indexOf(",")).trim();
                            String op2 = line.substr(line.indexOf(",") + 1).trim();
                            eOperandType opt;
                            if (!parseOperand(op1, val, flags, byte, eByte::All, &opt))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (val.size() != 1)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (opt != eOperandType::Variable && opt != eOperandType::Register && opt != eOperandType::Integer && opt != eOperandType::Char)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            bool ptr1 = opt == eOperandType::Variable || opt == eOperandType::Register;
                            code.push_back((_int32)ins);
                            MemAddress fladdr = (MemAddress)code.size();
                            code.push_back(0);
                            code.push_back(val[0]);
                            flags = 0;
                            flags = Utils::setb(flags, eByte::Two, (_ubyte)byte);
                            val.clear();
                            if (!parseOperand(op2, val, flags, byte, eByte::All, &opt))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (val.size() != 1)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (opt != eOperandType::Variable && opt != eOperandType::Register && opt != eOperandType::Integer && opt != eOperandType::Char)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            bool ptr2 = opt == eOperandType::Variable || opt == eOperandType::Register;
                            flags = Utils::setb(flags, eByte::Three, (_ubyte)byte);
                            code.push_back(val[0]);
                            _byte ct;
                            if (ptr1 && ptr2) ct = (_byte)eFlags::PointerPointerCompare;
                            else if (ptr1 && !ptr2) ct = (_byte)eFlags::PointerValueCompare;
                            else if (!ptr1 && ptr2) ct = (_byte)eFlags::ValuePointerCompare;
                            else if (!ptr1 && !ptr2) ct = (_byte)eFlags::ValueValueCompare;
                            flags = Utils::setb(flags, eByte::One, ct);
                            code[fladdr] = flags;
                        }
                        continue;
                        case eInstructionSet::jmp: //Done
                        {
                            if (line == "")
                            {
                                error(eAssemblerErrors::WrongJmpInstruction, "Wrong <jmp> instruction.");
                                return std::vector<_int32>();
                            }
                            code.push_back((_int32)ins);
                            code.push_back(flags);
                            findLabel(line, code, labels);
                        }
                        continue;
                        case eInstructionSet::cat: //Done
                        {
                            if (!line.contains(" ") || !line.contains(","))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            String flg = line.substr(0, line.indexOf(" ")).trim();
                            line = line.substr(line.indexOf(" ") + 1).trim();
                            if (!Utils::isInt(flg))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            flags = Utils::strToInt(flg);
                            String op1 = line.substr(0, line.indexOf(",")).trim();
                            String op2 = line.substr(line.indexOf(",") + 1).trim();
                            eByte byte = eByte::All;
                            _int32 tmp = 0;
                            std::vector<_int32> val;
                            if (!parseOperand(op1, val, tmp, byte, eByte::One))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (val.size() != 1)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            code.push_back((_int32)ins);
                            MemAddress fladdr = (MemAddress)code.size();
                            code.push_back(0);
                            code.push_back(val[0]);
                            val.clear();
                            tmp = 0;
                            byte = eByte::All;
                            if (!parseOperand(op2, val, tmp, byte, eByte::One))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (val.size() > 1)
                                code.insert(code.end(), val.begin(), val.end());
                            else
                            {
                                code.push_back(val[0]);
                                flags = Utils::setb(flags, eByte::One, Utils::getb(tmp, eByte::One));
                                flags = Utils::setb(flags, eByte::Three, (_ubyte)byte);
                            }
                            code[fladdr] = flags;
                        }
                        continue;
                        case eInstructionSet::add:
                        case eInstructionSet::sub:
                        case eInstructionSet::mul:
                        case eInstructionSet::div: //Done
                        {
                            if (!line.contains(","))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            std::vector<_int32> val;
                            eByte byte;
                            String op1 = line.substr(0, line.indexOf(",")).trim();
                            String op2 = line.substr(line.indexOf(",") + 1).trim();
                            if (!parseOperand(op1, val, flags, byte))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (val.size() != 1)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            code.push_back((_int32)ins);
                            MemAddress fladdr = (MemAddress)code.size();
                            code.push_back(0);
                            code.push_back(val[0]);
                            flags = 0;
                            flags = Utils::setb(flags, eByte::Two, (_ubyte)byte);
                            val.clear();
                            if (!parseOperand(op2, val, flags, byte))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (val.size() != 1)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            flags = Utils::setb(flags, eByte::Three, (_ubyte)byte);
                            code.push_back(val[0]);
                            code[fladdr] = flags;
                        }
                        continue;
                        case eInstructionSet::flg: //Done
                        {
                            if (!Utils::isInt(line))
                            {
                                //TODO: Error
                                return std::vector<_int32>();
                            }
                            code.push_back((_int32)ins);
                            code.push_back(Utils::strToInt(line));
                        }
                        continue;
                        case eInstructionSet::call: //Done
                        {
                            if (!line.contains("(") || !line.contains(")"))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            String sub = line.substr(0, line.indexOf("(")).trim();
                            line = line.substr(line.indexOf("(") + 1, line.length() - 1).trim();
                            if (sub == "")
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            code.push_back((_int32)ins);
                            code.push_back(0);
                            findLabel(sub, code, labels);
                            if (line == "") code.push_back(0);
                            else
                            {
                                std::vector<_int32> val;
                                eByte byte;
                                eOperandType opt;
                                StringTokens st = line.tokenize(",", true);
                                code.push_back((_int32)st.count());
                                String token = "";
                                while (st.hasNext()) st.next();
                                while (st.hasPrevious())
                                {
                                    token = st.previous();
                                    flags = 0;
                                    if (!parseOperand(token, val, flags, byte, eByte::All, &opt))
                                    {
                                        //TODO: Error
                                        return EMPTY_CODE;
                                    }
                                    if (val.size() != 1)
                                    {
                                        //TODO: Error
                                        return EMPTY_CODE;
                                    }
                                    code.push_back(flags);
                                    code.push_back(val[0]);
                                }
                            }
                        }
                        continue;
                        case eInstructionSet::push: //Done
                        {
                            std::vector<_int32> val;
                            eByte byte;
                            if (!parseOperand(line, val, flags, byte, eByte::One))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (val.size() != 1)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            flags = Utils::setb(flags, eByte::Two, (_int32)byte);
                            code.push_back((_int32)ins);
                            code.push_back(flags);
                            code.push_back(val[0]);
                        }
                        continue;
                        case eInstructionSet::pop: //Done
                        {
                            String sflg = "0x00";
                            if (line.contains(" "))
                            {
                                sflg = line.substr(0, line.indexOf(" ")).trim();
                                line = line.substr(line.indexOf(" ") + 1).trim();
                            }
                            _ubyte iflg = 0;
                            if (Utils::isInt(sflg)) iflg = (_ubyte)Utils::strToInt(sflg);
                            flags = 0;
                            flags = Utils::setb(flags, eByte::Four, iflg);
                            eRegisters reg;
                            if (isRegister(line, reg))
                            {
                                code.push_back((_int32)ins);
                                code.push_back(flags);
                                code.push_back((_int32)reg);
                            }
                            else
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                        }
                        continue;
                        case eInstructionSet::ret: //Done
                        {
                            std::vector<_int32> val;
                            eByte byte;
                            eOperandType opt;
                            if (!parseOperand(line, val, flags, byte, eByte::One, &opt))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            if (val.size() != 1)
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            flags = Utils::setb(flags, eByte::Two, (_ubyte)byte);
                            code.push_back((_int32)ins);
                            code.push_back(flags);
                            code.push_back(val[0]);
                        }
                        continue;
                        case eInstructionSet::end: //Done
                        {
                            if (line == "")
                            {
                                code.push_back((_int32)ins);
                                code.push_back(0);
                            }
                            else if (Utils::isInt(line))
                            {
                                code.push_back((_int32)ins);
                                code.push_back(Utils::strToInt(line));
                            }
                            else
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                        }
                        continue;
                        case eInstructionSet::alloc_h: //Done
                        {
                            if (Utils::isInt(line))
                            {
                                String tmp_vname = "__tmp__alloc_h__size";
                                if (!isVariable(String("$").add(tmp_vname)))
                                    m_allocs[tmp_vname.cpp()] = m_nextAlloc++;
                                code.push_back((_int32)eInstructionSet::mem);
                                code.push_back(0);
                                code.push_back(0); //TODO: Offset param
                                code.push_back((_int32)m_allocs[tmp_vname.cpp()]);
                                code.push_back(Utils::strToInt(line));
                                code.push_back((_int32)ins);
                                code.push_back((_int32)m_allocs[tmp_vname.cpp()]);
                            }
                            else if (isVariable(line))
                            {
                                code.push_back((_int32)ins);
                                code.push_back((_int32)m_allocs[line.substr(1).trim().cpp()]);
                            }
                            else
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                        }
                        continue;
                        case eInstructionSet::free_h: //Done
                        {
                            if (!isVariable(line))
                            {
                                //TODO: Error
                                return EMPTY_CODE;
                            }
                            code.push_back((_int32)ins);
                            code.push_back((_int32)m_allocs[line.substr(1).trim().cpp()]);
                        }
                        continue;
                        default: continue;
                    }
                }
                else
                {
                    error(eAssemblerErrors::UnknownDirective, "Unknown directive.");
                    return std::vector<_int32>();
                }
            }
            if (open_subroutine)
            {
                error(eAssemblerErrors::OpenSRLeft, String("Open Sub-routine left: ").add(sr_name));
                return std::vector<_int32>();
            }
            bool p = false;
            for (auto& pl : m_pendingLabels) //Unresolved labels check
            {
                if (pl.pending)
                {
                    _line = pl.line;
                    lineNumber = pl.lineN;
                    error(eAssemblerErrors::UnknownLabel, String("Unresolved pending Label: ").add(pl.name));
                    p = true;
                }
            }
            if (p) return std::vector<_int32>();
            code[0] = (_int32)code.size();
            code[7] = (_int32)ep_addr;
            if (code[2] == 0) code[2] = m_allocs.size();
            m_out->print("Assembled correctly.").newLine();
            return code;
        }


        std::vector<String> Disassembler::disassemble(std::vector<_int32>& code, bool print)
        {
            std::vector<String> lines;
            MemAddress i = 0;
            String line = String("//code size = ").add(lines[i++]);
            lines.push_back(line);
            line = String("req_local ").add(code[++i]);
            i++;
            lines.push_back(line);
            line = String("req_heap ").add(code[++i]);
            i++;
            lines.push_back(line);
            line = String("req_stack ").add(code[++i]);
            i++;
            lines.push_back(line);
            line = String("//entry_point address = ").add(code[i++]);
            lines.push_back(line);
            for ( ; i < code.size(); i++)
            {
                _int32 ins = code[i++];
                _int32 flags = code[i++];
                switch ((eInstructionSet)ins)
                {
                    case eInstructionSet::mem:
                    case eInstructionSet::mov:
                    {
                        _int32 offset = code[i++];
                        line = ".low mem, ";
                        line = line.add(Utils::intToHexStr(flags));
                        line = line.add(", ");
                        line = line.addInt(offset);
                        line = line.add(Utils::intToHexStr(code[i++]));
                        line = line.add(Utils::intToHexStr(code[i++]));
                        lines.push_back(line);
                    }
                    continue;
                    case eInstructionSet::com:
                    {
                    }
                    continue;
                    case eInstructionSet::inc:
                    case eInstructionSet::dec:
                    {
                    }
                    continue;
                    case eInstructionSet::cmp:
                    {
                    }
                    continue;
                    case eInstructionSet::jmp:
                    {
                    }
                    continue;
                    case eInstructionSet::cat:
                    {
                    }
                    continue;
                    case eInstructionSet::add:
                    case eInstructionSet::sub:
                    case eInstructionSet::mul:
                    case eInstructionSet::div:
                    {
                    }
                    continue;
                    case eInstructionSet::flg:
                    {
                    }
                    continue;
                    case eInstructionSet::call:
                    {
                    }
                    continue;
                    case eInstructionSet::push:
                    {
                    }
                    continue;
                    case eInstructionSet::pop:
                    {
                    }
                    continue;
                    case eInstructionSet::ret:
                    {
                    }
                    continue;
                    case eInstructionSet::end:
                    {
                    }
                    continue;
                    case eInstructionSet::alloc_h:
                    {
                    }
                    continue;
                    case eInstructionSet::free_h:
                    {
                    }
                    continue;
                    default: continue;
                }
            }
            return lines;
        }

        bool Disassembler::parseInstruction(MemAddress addr, std::vector<_int32>& code, std::vector<String>& lines, bool print)
        {
            _int32 flags = 0;
            _int32 inst = code[addr++];
            switch ((eInstructionSet)inst)
            {
                case eInstructionSet::mem:
                case eInstructionSet::mov: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::com: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::inc:
                case eInstructionSet::dec: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::cmp: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::jmp: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::cat: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::add:
                case eInstructionSet::sub:
                case eInstructionSet::mul:
                case eInstructionSet::div: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::flg: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::call: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::push: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::pop: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::ret: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::end: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::alloc_h: //Done
                {
                    flags = code[addr++];
                }
                return true;
                case eInstructionSet::free_h: //Done
                {
                    flags = code[addr++];
                }
                return true;
                default: return false;
            }
            return false;
        }
    }
}