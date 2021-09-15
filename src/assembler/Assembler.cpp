#include "Assembler.hpp"
#include <iostream>

#include <fstream>

namespace Omnia
{
	namespace oasm
	{
		std::vector<OmniaString> PreProcessor::open(OmniaString fileName, PreProcessorOptions options)
        {
            std::vector<OmniaString> lines;
            if (!Utils::readFile(fileName, lines))
            {
                //TODO: Repllace error
                error(ePreProcessorErrors::FailedToOpenFile, OmniaString("Failed to open source file: ").add(fileName), true);
                return std::vector<OmniaString>();
            }
            if (lines.size() == 0)
            {
                //TODO: Replace error
                error(ePreProcessorErrors::EmptyFile, OmniaString("Empty source file provided: ").add(fileName), true);
                return std::vector<OmniaString>();
            }
			_line = "";
			m_reserveCount = 0;
            m_nextTopInst = 0;
            m_skip_rest_of_branches = false;
			if (!m_includeGuards.empty()) m_includeGuards.clear();
			if (!m_aliases.empty()) m_aliases.clear();
			if (!m_reserves.empty()) m_reserves.clear();
            if (!m_dataSection.empty()) m_dataSection.clear();
            if (!m_defines.empty()) m_defines.clear();
            if (!m_def_stack.empty()) m_def_stack.clear();
            if (!m_struct_defs.empty()) m_struct_defs.clear();
            currentFile = fileName;
            return process(lines, options);
        }

        std::vector<OmniaString> PreProcessor::process(std::vector<OmniaString> lines, PreProcessorOptions options)
        {
            m_options = options;
            std::vector<OmniaString> finalCode = resolveIncludes(lines, currentFile);
            if (finalCode.size() == 0) return std::vector<OmniaString>(); //TODO: Diversify errors
            finalCode = removeComments(finalCode);
            if (finalCode.size() == 0) return std::vector<OmniaString>();
            finalCode = resolveDefines(finalCode);
            if (finalCode.size() == 0) return std::vector<OmniaString>();
            finalCode = resolveAliases(finalCode);
            if (finalCode.size() == 0) return std::vector<OmniaString>();
            finalCode = resolveMacros(finalCode);
            if (finalCode.size() == 0) return std::vector<OmniaString>();
			finalCode = resolveCommandDirective(finalCode);
            if (finalCode.size() == 0) return std::vector<OmniaString>();
			finalCode = resolveDataDirective(finalCode);
            if (finalCode.size() == 0) return std::vector<OmniaString>();
            m_dataSection.insert(m_dataSection.begin(), StringBuilder("reserve,         Single_Const,       ").add(Utils::intToHexStr(m_reserveCount)).get());
            m_dataSection.insert(m_dataSection.begin(), ":__load__:");
            m_dataSection.push_back("call,               Const_Const,                   __main__,                  0x0000");
            m_dataSection.push_back("end,               Single_Reg,                   RV");
			for (auto& _line : finalCode)
			{
				_line = _line.replaceAll(" ", "");
				_line = _line.replaceAll("\t", "");
				_line = _line.replaceAll("\n", "");
			}
            for (auto& _line : m_dataSection)
			{
				_line = _line.replaceAll(" ", "");
				_line = _line.replaceAll("\t", "");
				_line = _line.replaceAll("\n", "");
			}
            finalCode.insert(finalCode.begin() + m_nextTopInst++, OmniaString("call,              Const_Const,                    __load__,             0x0000"));
            return finalCode;
        }


		std::vector<OmniaString> PreProcessor::resolveCommandDirective(std::vector<OmniaString> lines)
		{
            std::vector<OmniaString> code;
            lineNumber = 0;
            currentFile = "Processed_file";

			for (auto& l : lines)
            {
                _line = l;
                lineNumber++;
                l = l.trim();
                if (!l.toLowerCase().startsWith(".command"))
                {
                    code.push_back(l);
                    continue;
                }
                OmniaString line = l.substr(8).trim();
                if (!line.startsWith("(") || !line.endsWith(")"))
                {
					//TODO: Add error
                    return std::vector<OmniaString>();
                }
                line = line.substr(1, line.length() - 1).trim();
				OmniaString::StringTokens __st = line.tokenize(",", true);
				if (__st.count() < 2)
				{
					//TODO: Add error
					return std::vector<OmniaString>();
				}
				OmniaString __cmd = __st.next().toLowerCase();
				OmniaString param = "", value = "";
				while (__st.hasNext())
				{
					param = __st.next().toLowerCase();
					if (!param.contains("="))
					{
						//TODO: Add error
						return std::vector<OmniaString>();
					}
					value = param.substr(param.indexOf("=") + 1).trim();
					param = param.substr(0, param.indexOf("=")).trim();
					if (__cmd.equals("request"))
					{
						if (param.equals("all") && Utils::isInt(value))
							code.insert(code.begin() + m_nextTopInst++, StringBuilder("req,                 REQ_ALL,                    ").add(value).get());
						else if (param.equals("stack") && Utils::isInt(value))
							code.insert(code.begin() + m_nextTopInst++, StringBuilder("req,                 REQ_STACK,                  ").add(value).get());
						else if (param.equals("heap") && Utils::isInt(value))
							code.insert(code.begin() + m_nextTopInst++, StringBuilder("req,                 REQ_HEAP,                   ").add(value).get());
					}
				}
            }
			return code;
		}

		std::vector<OmniaString> PreProcessor::resolveDataDirective(std::vector<OmniaString> lines)
		{
            std::vector<OmniaString> code;
            lineNumber = 0;
            currentFile = "Processed_file";
            bool __struct = false;
            StringBuilder __struct_code;
            OmniaString __struct_name = "";
			for (auto& l : lines)
            {
                _line = l;
                lineNumber++;
                l = l.trim();
                if (__struct)
                {
                    OmniaString line = l;
                    if (line.startsWith(".end_struct"))
                    {
                        line = line.substr(11).trim();
                        if (!line.startsWith("(") || !line.endsWith(")"))
                        {
                            //TODO: Add error
                            return std::vector<OmniaString>();
                        }
                        line = line.substr(1, line.length() - 1).trim();
                        if (line == __struct_name)
                        {
                            __struct = false;
                            __struct_name = "";
                            m_struct_defs.push_back(__struct_code.get());
                            std::cout << "\n\n\n" << m_struct_defs[m_struct_defs.size() - 1].cpp() << "\n\n\n";
                            __struct_code = StringBuilder();
                            continue;
                        }
                    }
                    __struct_code.add(l).add("\n");
                    continue;
                }
                if (!l.toLowerCase().startsWith(".data"))
                {
                    code.push_back(l);
                    continue;
                }
                OmniaString line = l.substr(5).trim();
                if (!line.startsWith("="))
                {
					//TODO: Add error
                    return std::vector<OmniaString>();
                }
                line = line.substr(1).trim();
				OmniaString::StringTokens __st;
				if (line.toLowerCase().startsWith("reserve"))
				{
					line = line.substr(7).trim();
                	if (!line.startsWith("(") || !line.endsWith(")"))
					{
						//TODO: Add error
						return std::vector<OmniaString>();
					}
                	line = line.substr(1, line.length() - 1).trim();
					__st = line.tokenize(",", true);
					OmniaString param = "", value = "";
					while (__st.hasNext())
					{
						param = __st.next();
						//m_dataSection.push_back(StringBuilder("reserve,                 Single_Reg,                     R30").get());
						m_reserves[param.cpp()] = (MemAddress)(m_reserveCount++);
                        m_symTable.m_reserves[(MemAddress)(m_reserveCount - 1)] = param;
					}
				}
                else if (line.toLowerCase().startsWith("array"))
				{
					line = line.substr(5).trim();
                	if (!line.startsWith("(") || !line.endsWith(")"))
					{
						//TODO: Add error
						return std::vector<OmniaString>();
					}
                	line = line.substr(1, line.length() - 1).trim();
					__st = line.tokenize(",", true);
                    if (__st.count() != 2)
                    {
                        //TODO: Error
                        return std::vector<OmniaString>();
                    }
					OmniaString def = __st.next();
                    OmniaString __tmp = __st.next();
                    if (!Utils::isInt(__tmp))
                    {
                        //TODO: Error
                        return std::vector<OmniaString>();
                    }

                    m_reserves[OmniaString("%").add(def).cpp()] = (MemAddress)m_reserveCount;
                    m_symTable.m_reserves[(MemAddress)m_reserveCount] = OmniaString("%").add(def);
                    m_reserveCount += Utils::strToInt(__tmp);
				}
				else if (line.toLowerCase().startsWith("load_string"))
				{
					line = line.substr(11).trim();
                	if (!line.startsWith("(") || !line.endsWith(")"))
					{
						//TODO: Add error
						return std::vector<OmniaString>();
					}
                	line = line.substr(1, line.length() - 1).trim();
					__st = line.tokenize(",", true);
					if (__st.count() != 2)
					{
						//TODO: Add error
						return std::vector<OmniaString>();
					}
					OmniaString param = __st.next();
					OmniaString __str = __st.next();
					if (!__str.endsWith("\"") || !__str.startsWith("\""))
					{
						//TODO: Add error
						return std::vector<OmniaString>();
					}
					if (!hasReserved(param))
					{
						//TODO: Add error
						return std::vector<OmniaString>();
					}
					__str = __str.substr(1, __str.length() - 1);
					TMemoryList __str_stream = BitEditor::stringToConstSream(__str);
					StringBuilder __sb("lda_str,                    ");
					for (auto& __mc : __str_stream)
						__sb.add(Utils::intToHexStr(__mc.val())).add(",");
					__str = __sb.get().substr(0, __sb.get().length() - 1);
					m_dataSection.push_back(__str);
					m_dataSection.push_back(StringBuilder("mem,     Addr_Reg,    ").add(param.cpp()).add(",    R31").get());
				}
                else if (line.toLowerCase().startsWith("struct"))
				{
                    line = line.substr(6).trim();
                    if (!line.startsWith("(") || !line.endsWith(")"))
                    {
                        //TODO: Add error
                        return std::vector<OmniaString>();
                    }
                    line = line.substr(1, line.length() - 1).trim();
                    __struct_name = line;
                    __struct = true;
                    __struct_code.add(__struct_name).add(";\n");
                    continue;
				}
            }
			return code;
		}


        std::vector<OmniaString> PreProcessor::resolveAliases(std::vector<OmniaString> lines)
        {
            std::vector<OmniaString> code;
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
                OmniaString line = l.substr(7).trim();
                if (!line.contains("="))
                {
                    error(ePreProcessorErrors::WrongAliasDir, "Wrong .alias directive.");
                    return std::vector<OmniaString>();
                }
                OmniaString alias = line.substr(0, line.indexOf("=")).trim();
                line = line.substr(line.indexOf("=") + 1).trim();
                m_aliases[OmniaString("#").add(alias).cpp()] = line.cpp();
            }
            for (uint16 i = 0; i < m_options.passes; i++)
            {
                for (auto& line : code) //Replacing aliases
                {
                    for (auto& al : m_aliases)
                        line = Utils::replaceAllVarName(line, OmniaString(al.first), OmniaString(al.second));
                }
            }
            return code;
        }

        std::vector<OmniaString> PreProcessor::resolveMacros(std::vector<OmniaString> lines)
        {
            std::vector<OmniaString> code;
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
                OmniaString line = l.substr(7).trim();
                Macro mac(line);
                if (!mac.isValid())
                {
                    //TODO: Error
                    return std::vector<OmniaString>();
                }
                m_macros.push_back(mac);
            }
            for (uint16 i = 0; i < m_options.passes; i++)
            {
                int32 l = 0;
                for (auto& line : code) //Replacing macros
                {
                    for (auto& m : m_macros)
                    {
                        OmniaString mname = OmniaString("@").add(m.name);
                        mname = mname.add("(");
                        bool found = false;
                        while (line.contains(mname))
                        {
                            int32 cp = line.indexOf(")");
                            int32 ep = line.indexOf(mname);
                            if (ep + mname.length() > cp) break;
                            OmniaString plist = line.substr(ep + mname.length(), cp).trim();
                            OmniaString line_p1 = line.substr(0, ep);
                            OmniaString line_p2 = line.substr(cp + 1);
                            line = line_p1.add(m.expand(plist));
                            line = line.add(line_p2);
                            found = true;
                        }
                        if (!found) continue;
                        OmniaString::StringTokens st = line.tokenize("\"&#\"");
                        if (st.count() < 2) continue;
                        int32 el = 0;
                        line = "";
                        while (st.hasNext())
                            code.insert(code.begin() + (l + el++), st.next());
                    }
                    l++;
                }
            }
            return code;
        }

        std::vector<OmniaString> PreProcessor::removeComments(std::vector<OmniaString> lines)
        {
            std::vector<OmniaString> code;
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
                        OmniaString lp1 = line.substr(0, line.indexOf("/*"));
                        OmniaString lp2 = line.substr(line.indexOf("*/") + 2);
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

        std::vector<OmniaString> PreProcessor::resolveIncludes(std::vector<OmniaString> mainFile, OmniaString curFile)
        {
            std::vector<OmniaString> ilines = resolveIncludes_r(mainFile, curFile);
            std::vector<OmniaString> nlines;
            bool include_skip = false;
            uint32 grdCount = 0;
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
        
        std::vector<OmniaString> PreProcessor::resolveIncludes_r(std::vector<OmniaString> mainFile, OmniaString curFile)
        {
            _line = "";
            lineNumber = 0;
            currentFile = curFile;
            std::vector<OmniaString> incl;
            std::vector<OmniaString> tmp;
            for (auto& l : mainFile) //Includes round
            {
                _line = l;
                lineNumber++;
                if (!l.toLowerCase().trim().startsWith(".include "))
                {
                    tmp.push_back(l);
                    continue;
                }
                OmniaString line = l.substr(9).trim();
                if (!line.startsWith("[") || !line.endsWith("]"))
                {
                    error(ePreProcessorErrors::WrongInclDirective, "Wrong .include directive.", true);
                    return std::vector<OmniaString>();
                }
                line = line.substr(1, line.length() - 1).trim();
                if (!Utils::readFile(line, incl))
                {
                    error(ePreProcessorErrors::FailedToOpenFile, OmniaString("File in .include directive could not be found.").add(line), true);
                    return std::vector<OmniaString>();
                }
                if (incl.size() > 0)
                {
                    std::vector<OmniaString> nincl = resolveIncludes_r(incl, line);
                    tmp.insert(tmp.end(), nincl.begin(), nincl.end());
                }
            }
            return tmp;
        }
        
        std::vector<OmniaString> PreProcessor::resolveDefines(std::vector<OmniaString> lines)
        {
            std::vector<OmniaString> code;
            lineNumber = 0;
            currentFile = "Processed_file";
            for (auto& l : lines)
            {
                _line = l;
                lineNumber++;
                l = l.trim();
                if (l.toLowerCase().startsWith(".def "))
                {
                    OmniaString line = l.substr(5).trim();
                    if (line == "")
                    {
                        m_out->print("*** Warning, no name for .def directive.").newLine();
                        //TODO: Warning, no name for .def directive
                        continue;
                    }
                    if (line.contains(" "))
                    {
                        OmniaString __def = line.substr(0, line.indexOf(" ")).trim();
                        line = line.substr(line.indexOf(" ") + 1).trim();
                        if (hasDefine(__def))
                        {
                            m_out->print("*** Warning, <").print(__def).print("> symbol already defined").newLine();
                            //TODO: Warning, redefinition
                        }
                        m_defines[__def.cpp()] = { line, true };
                        continue;
                    }
                    else
                    {
                        if (hasDefine(line))
                        {
                            m_out->print("*** Warning, <").print(line).print("> symbol already defined").newLine();
                            //TODO: Warning, redefinition
                        }
                        m_defines[line.cpp()] = { "", true };
                        continue;
                    }
                }
                else if (l.toLowerCase().startsWith(".undef "))
                {
                    OmniaString line = l.substr(7).trim();
                    if (line == "")
                    {
                        m_out->print("*** Warning, no name for .def directive.").newLine();
                        //TODO: Warning, no name for .def directive
                        continue;
                    }
                    if (!hasDefine(line))
                    {
                        m_out->print("*** Warning, <").print(line).print("> symbol undefined").newLine();
                        //TODO: Warning, undefined 
                        continue;
                    }
                    m_defines[line.cpp()].second = false;
                    continue;
                }
                else if (l.toLowerCase().startsWith(".ifdef "))
                {
                    OmniaString line = l.substr(7).trim();
                    if (line == "")
                    {
                        m_out->print("*** Warning, no name for .ifdef directive.").newLine();
                        //TODO: Warning, no name for .def directive
                        continue;
                    }
                    if (!hasDefine(line))
                    {
                        m_def_stack.push_back(false);
                        continue;
                    }
                    m_def_stack.push_back(m_defines[line.cpp()].second);
                    m_skip_rest_of_branches = m_def_stack[m_def_stack.size() - 1];
                    continue;
                }
                else if (l.toLowerCase().startsWith(".ifndef "))
                {
                    OmniaString line = l.substr(8).trim();
                    if (line == "")
                    {
                        m_out->print("*** Warning, no name for .ifndef directive.").newLine();
                        //TODO: Warning, no name for .def directive
                        continue;
                    }
                    if (!hasDefine(line))
                    {
                        m_def_stack.push_back(true);
                        continue;
                    }
                    m_def_stack.push_back(!m_defines[line.cpp()].second);
                    m_skip_rest_of_branches = m_def_stack[m_def_stack.size() - 1];
                    continue;
                }
                else if (l.toLowerCase().startsWith(".else"))
                {
                    if (m_def_stack.size() == 0)
                    {
                        m_out->print("*** Warning, dead .else directive, missing .ifdef.").newLine();
                        //TODO: Warning, undefined 
                        continue;
                    }
                    if (m_skip_rest_of_branches)
                    {
                        m_def_stack[m_def_stack.size() - 1] = false;
                        continue;
                    }
                    m_def_stack[m_def_stack.size() - 1] = !m_def_stack[m_def_stack.size() - 1];
                    continue;
                }
                else if (l.toLowerCase().startsWith(".elifdef "))
                {
                    OmniaString line = l.substr(9).trim();
                    if (m_def_stack.size() == 0)
                    {
                        m_out->print("*** Warning, dead .elifdef directive, missing .ifdef.").newLine();
                        //TODO: Warning, undefined 
                        continue;
                    }
                    if (line == "")
                    {
                        m_out->print("*** Warning, no name for .elifdef directive.").newLine();
                        //TODO: Warning, no name for .def directive
                        continue;
                    }
                    if (m_def_stack[m_def_stack.size() - 1] || m_skip_rest_of_branches)
                    {
                        m_def_stack[m_def_stack.size() - 1] = false;
                        continue;
                    }
                    if (!hasDefine(line))
                    {
                        m_def_stack[m_def_stack.size() - 1] = false;
                        continue;
                    }
                    m_def_stack[m_def_stack.size() - 1] = m_defines[line.cpp()].second;
                    continue;
                }
                else if (l.toLowerCase().startsWith(".elifndef "))
                {
                    OmniaString line = l.substr(10).trim();
                    if (m_def_stack.size() == 0)
                    {
                        m_out->print("*** Warning, dead .elifndef directive, missing .ifdef.").newLine();
                        //TODO: Warning, undefined 
                        continue;
                    }
                    if (line == "")
                    {
                        m_out->print("*** Warning, no name for .elifndef directive.").newLine();
                        //TODO: Warning, no name for .def directive
                        continue;
                    }
                    if (m_def_stack[m_def_stack.size() - 1] || m_skip_rest_of_branches)
                    {
                        m_def_stack[m_def_stack.size() - 1] = false;
                        continue;
                    }
                    if (!hasDefine(line))
                    {
                        m_def_stack[m_def_stack.size() - 1] = true;
                        continue;
                    }
                    m_def_stack[m_def_stack.size() - 1] = !m_defines[line.cpp()].second;
                    continue;
                }
                else if (l.toLowerCase().startsWith(".endif"))
                {
                    if (m_def_stack.size() == 0)
                    {
                        m_out->print("*** Warning, dead .endif directive, missing .ifdef.").newLine();
                        //TODO: Warning, undefined 
                        continue;
                    }
                    STDVEC_REMOVE(m_def_stack, m_def_stack.size() - 1);
                    m_skip_rest_of_branches = false;
                    continue;
                }
                else 
                {
                    if (m_def_stack.size() == 0 || m_def_stack[m_def_stack.size() - 1])
                    {
                        for (uint8 i = 0; i < m_options.passes; i++)
                        {
                            for (auto& __def : m_defines)
                            {
                                if (!__def.second.second) continue;
                                l = l.replaceAll(__def.first, __def.second.first);
                            }
                        }
                        code.push_back(l);
                    }
                    continue;
                }
            }
            return code;
        }

		//TODO: Replace with ErrorReciever methods
        void PreProcessor::error(ePreProcessorErrors err, OmniaString msg, bool skipFileInfo)
        {
            m_out->newLine().print("PreProcessor Error ").print((int32)err).newLine().print(msg).newLine();
            if (!skipFileInfo)
            {
                m_out->print("File: ").print(currentFile).newLine();
                m_out->print("Line ").print((int32)(lineNumber + 1)).newLine();
                m_out->tab().print(_line).newLine();
            }
        }



        Macro::Macro(OmniaString line)
        {
            invalidate();
            line = line.trim();
            if (line == "") return;
            if (!line.contains("(") || !line.contains(")")) return;
            name = line.substr(0, line.indexOf("(")).trim();
            if (name == "") return;
            OmniaString p = line.substr(line.indexOf("(") + 1, line.indexOf(")")).trim();
            expansion = line.substr(line.indexOf(")") + 1).trim();
            if (expansion == "") return;
            if (p != "")
            {
                OmniaString::StringTokens st = p.tokenize(",", true);
                while (st.hasNext()) params.push_back(st.next());
            }
            validate();
        }

        OmniaString Macro::expand(OmniaString line)
        {
            if (!isValid()) return "";
            if (line == "" || expansion == "") return "";
            OmniaString exp = expansion.trim();
            OmniaString::StringTokens st = line.tokenize(",", true);
            for (auto param : params)
            {
                if (!st.hasNext()) return "";
                OmniaString p = OmniaString("$(").add(param);
                p = p.add(")");
                exp = exp.replaceAll(p, st.next());
            }
            return exp.trim();
        }



		int64 Assembler::run(int argc, char** argv)
		{
			OutputManager &out = *getOutputHandler();
            p__dbg_symbol_table = false;
            p__dbg_save_code = false;
			p__input_file_path = "";
            p__output_file_path = "";
            p__output_file_dbg_table = "";
			if (argc > 1)
			{
				for (int i = 1; i < argc; i++)
				{
					if (OmniaString(argv[i]).trim().equals("--input-file") || OmniaString(argv[i]).trim().equals("-i"))
					{
						if (i + 1 >= argc)
						{
							out.print("Error: No input file specified.").newLine();
							return 0xFFFF; //TODO: Add error code
						}
						i++;
						p__input_file_path = OmniaString(argv[i]);
					}
                    else if (OmniaString(argv[i]).trim().equals("--output-file") || OmniaString(argv[i]).trim().equals("-o"))
					{
						if (i + 1 >= argc)
						{
							out.print("Error: No output file specified.").newLine();
							return 0xFFFA; //TODO: Add error code
						}
						i++;
						p__output_file_path = OmniaString(argv[i]);
					}
                    else if (OmniaString(argv[i]).trim().equals("--debug-table") || OmniaString(argv[i]).trim().equals("-dt"))
					{
                        p__dbg_symbol_table = true;
                    }
                    else if (OmniaString(argv[i]).trim().equals("--save-code") || OmniaString(argv[i]).trim().equals("-sc"))
					{
                        p__dbg_save_code = true;
                    }
				}
			}
            if (p__dbg_save_code && !p__dbg_symbol_table) p__dbg_save_code = false;
			if (p__input_file_path.trim() == "") return 0xFFFE; //TODO: Error
			if (p__output_file_path.trim() == "")
                p__output_file_path = p__input_file_path.substr(0, p__input_file_path.lastIndexOf(".")).add(".oex");
			TMemoryList __program = assemble(p__input_file_path);
            if (!createExecutableFile(p__output_file_path, __program))
            {
                out.print("Error: Failed to create executable.").newLine();
                return 0xCFCF; //TODO: Add error code
            }
            if (p__dbg_symbol_table)
            {
                p__output_file_dbg_table = p__output_file_path.substr(0, p__output_file_path.lastIndexOf(".")).add(".odb");
                if (!createDebugTableFile(p__output_file_dbg_table))
                {
                    out.print("Error: Failed to create debug table file.").newLine();
                    return 0xAFAF; //TODO: Add error code
                }
            }
			return 0;
		}

        bool Assembler::createExecutableFile(OmniaString __outputFile, TMemoryList __program)
        {
            if (__program.size() == 0) return false;
            std::ofstream writeFile;
            writeFile.open(__outputFile.cpp(), std::ios::out | std::ios::binary);
            writeFile.write((char*)(&__program[0]), __program.size() * sizeof(word));
            writeFile.close();
            getOutputHandler()->print("*** Generated executable file: ").print(__outputFile).print(" (").print((int64)(__program.size() * sizeof(BitEditor))).print(" bytes)").newLine();
            return true;
        }

        bool Assembler::createDebugTableFile(OmniaString __outputFile)
        {
            StringBuilder __sb;
            std::ofstream writeFile;
            writeFile.open(__outputFile.cpp());
            writeFile << "#version(" << Utils::getVerionString().cpp() << ")\n";
            for (auto& __label : PreProcessor::instance().m_symTable.m_labels)
            {
                __sb.add(".label ").add(__label.second).add(" = ").add(Utils::intToHexStr(__label.first));
                writeFile << __sb.get().cpp() << "\n";
                __sb = StringBuilder();
            }
            for (auto& __reserve : PreProcessor::instance().m_symTable.m_reserves)
            {
                __sb.add(".data ").add(__reserve.second).add(" = ").add(Utils::intToHexStr(__reserve.first));
                writeFile << __sb.get().cpp() << "\n";
                __sb = StringBuilder();
            }
            if (PreProcessor::instance().m_symTable.m_source.size() > 0)
            {
                writeFile << ".source = { \n";
                for (auto& __line : PreProcessor::instance().m_symTable.m_source)
                {
                    OmniaString __src_line = __line.second;
                    if (__src_line.startsWith("lda_str,"))
                    {
                        OmniaString::StringTokens __st = __src_line.tokenize(",", true);
                        __st.next();
                        TMemoryList __str_stream;
                        while (__st.hasNext())
                            __str_stream.push_back((word)Utils::strToInt(__st.next()));
                        __src_line = "lda_str,";
                        __src_line = __src_line.add("\"").add(BitEditor::constStreamToString(__str_stream)).add("\"");
                    }
                    writeFile << "\t\t" << Utils::intToHexStr(__line.first).cpp() << ":" << __src_line.cpp() << "\n";
                }
                writeFile << "}";
            }
            writeFile.close();
            getOutputHandler()->print("*** Generated debug-file: ").print(__outputFile).newLine();
            return true;
        }

		TMemoryList Assembler::assemble(OmniaString __source_file_path)
		{
            std::vector<OmniaString> __source = PreProcessor::instance().open(__source_file_path);
            std::vector<OmniaString> source = PreProcessor::instance().m_dataSection;
            source.insert(source.begin(), __source.begin(), __source.end());
            if (p__dbg_save_code)
            {
                std::map<MemAddress, OmniaString> __dbg_src;
                MemAddress __curr_addr = 0;
                OmniaString::StringTokens __st;
                for (auto& __line : source)
                {
                    __line = __line.trim();
                    if (__line.startsWith(":") && __line.endsWith(":")) continue;
                    __dbg_src[__curr_addr] = __line;
                    __st = __line.tokenize(",", true);
                    __curr_addr += __st.count();
                }
                PreProcessor::instance().m_symTable.m_source = __dbg_src;
            }
			std::vector<OmniaString> code = resolveKeyWords(source);
			return assemble(code);
		}

		TMemoryList Assembler::assemble(std::vector<OmniaString>& __source)
		{
			TMemoryList __code;
			OmniaString::StringTokens __sb;
			OmniaString __data = "";
			for (auto& __line : __source)
			{
				__sb = __line.tokenize(",", true);
				while (__sb.hasNext())
				{
					__data = __sb.next();
					if (__data == "") continue;
					if (!Utils::isInt(__data))
					{
						//TODO: Error
						return TMemoryList();
					}
					__code.push_back(Utils::strToInt(__data));
				}
			}
			return __code;
		}
		
		OmniaString Assembler::executeAssemblerFunction(OmniaString __func)
        {
            StringBuilder __generated_code;
            
            if (!__func.contains("(") || !__func.contains(")") || __func.lastIndexOf(")") <= __func.indexOf("("))
            {
                //TODO: Error
                std::cout << "Err 1 - Invalid syntax for assembler command: ";
                std::cout << "$." << __func.cpp() << "\n";
                return OmniaString("$.").add(__func);
            }
            OmniaString __params = __func.substr(__func.indexOf("(") + 1, __func.lastIndexOf(")")).trim();
            if (__func.startsWith("char"))
            {
                if (__params.length() < 3 || !__params.startsWith("\'") || !__params.endsWith("\'"))
                {
                    //TODO: Error
                    std::cout << "Err 2 - Invalid parameter for assembler command: ";
                    std::cout << "$." << __func.cpp() << "\n";
                    return OmniaString("$.").add(__func);
                }
                __params = __params.replaceAll("\\", "");
                __params = __params.substr(1, __params.length() - 1);
                if (__params.length() == 1)
                    __generated_code.add((int32)__params[0]);
                else
                {
                    //TODO: Error
                    std::cout << "Err 3 - Invalid parameter for assembler command: ";
                    std::cout << "$." << __func.cpp() << "\n";
                    return OmniaString("$.").add(__func);
                }
            }
            else
            {
                //TODO: Error
                std::cout << "Err 4 - Unknown assembler command: ";
                std::cout << "$." << __func.cpp() << "\n";
                return OmniaString("$.").add(__func);
            }
            return __generated_code.get();
        }

		std::vector<OmniaString> Assembler::resolveKeyWords(std::vector<OmniaString> lines)
		{
            PreProcessor& PP = PreProcessor::instance();
            std::vector<OmniaString> code;
			OmniaString __data = "";
			OmniaString::StringTokens __st;
			StringBuilder __new_line;
			MemAddress __addr = 0;
            for (auto& l : lines) //Resolve labels
            {
                l = l.trim();
				if (l.startsWith(":") && l.endsWith(":"))
				{
					l = l.substr(1, l.length() - 1).trim();
					if (l.contains(" ") || l.contains("\t"))
					{
						//TODO: Error
						return std::vector<OmniaString>();
					}
					m_labels[l.cpp()] = __addr;
                    if (p__dbg_symbol_table)
                        PP.m_symTable.m_labels[__addr] = l;
				}
				else
				{
					__st = l.tokenize(",", true);
					__addr += __st.count();
					code.push_back(l);
				}
			}
			lines = code;
			code.clear();
            for (auto& l : lines)
            {
				__new_line = StringBuilder();
                l = l.trim();
				__st = l.tokenize(",", true);
				while (__st.hasNext())
				{
					word __tmp = 0;
					__data = __st.next();
					if (Utils::isInt(__data))
					{
						__tmp = (word)Utils::strToInt(__data);
						__new_line.add(Utils::intToHexStr(__tmp)).add(",");
					}
					else if (isKeyword(__data, __tmp))
					{
						__new_line.add(Utils::intToHexStr(__tmp)).add(",");
					}
					else if (__data.startsWith("\"") && __data.endsWith("\""))
					{
						__data = __data.substr(1, __data.length() - 1);
						TMemoryList __str_stream = BitEditor::stringToConstSream(__data);
						for (auto& __mc : __str_stream)
							__new_line.add(Utils::intToHexStr(__mc.val())).add(",");
					}
					else if (PP.hasReserved(__data.cpp()))
					{
						__new_line.add(Utils::intToHexStr(PP.m_reserves[__data.cpp()])).add(",");
					}
                    else if (__data.startsWith("%") && __data.indexOf("[") > 1 && __data.endsWith("]"))
					{
                        OmniaString __res_name = __data.substr(0, __data.indexOf("[")).trim();
                        OmniaString __off_str = __data.substr(__data.indexOf("[") + 1, __data.length() - 1);
                        if (!Utils::isInt(__off_str))
                        {
						    //TODO: Error
                            std::cout << "Non-constant array index: ";
                            std::cout << __off_str.cpp() << "\n";
                            std::cout << "\n\t" << l.cpp() << "\n";
                            return std::vector<OmniaString>();
                        }
						__new_line.add(Utils::intToHexStr(PP.m_reserves[__res_name.cpp()] + Utils::strToInt(__off_str))).add(",");
					}
					else if (isLabel(__data.cpp(), __addr))
					{
						__new_line.add(Utils::intToHexStr(__addr)).add(",");
					}
					else if (__data.startsWith("$."))
                    {
                        OmniaString __as_cmd = __data.substr(2).trim();
                        __new_line.add(executeAssemblerFunction(__as_cmd)).add(",");
                    }
                    else
					{
						//TODO: Error
						std::cout << "Unknown symbol: ";
						std::cout << __data.cpp() << "\n";
						std::cout << "\n\t" << l.cpp() << "\n";
						return std::vector<OmniaString>();
					}
				}
				code.push_back(__new_line.get());
			}
			return code;
		}
	}
}