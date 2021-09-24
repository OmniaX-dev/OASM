#include "Assembler.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "Interpreter.hpp"

namespace Omnia
{
	namespace oasm
	{
		std::vector<OmniaString> PreProcessor::open(OmniaString fileName, OmniaString outputName, PreProcessorOptions options)
        {
            std::vector<OmniaString> lines;
            if (!Utils::readFile(fileName, lines))
            {
                printError(D__ASSEMBLER_ERR_FAILED_TO_OPEN_INPUT_FILE, *getOutputHandler(), fileName);
                return std::vector<OmniaString>();
            }
            if (lines.size() == 0)
            {
                printError(D__ASSEMBLER_ERR_EMPTY_IMPUT_FILE, *getOutputHandler(), fileName);
                return std::vector<OmniaString>();
            }
			_line = "";
			m_nextReserve = 0;
            m_nextTopInst = 0;
            m_skip_rest_of_branches = false;
			if (!m_includeGuards.empty()) m_includeGuards.clear();
			if (!m_aliases.empty()) m_aliases.clear();
			if (!m_reserves.empty()) m_reserves.clear();
            if (!m_dataSection.empty()) m_dataSection.clear();
            if (!m_defines.empty()) m_defines.clear();
            if (!m_def_stack.empty()) m_def_stack.clear();
            if (!m_struct_defs.empty()) m_struct_defs.clear();
            if (!__static_lib_defs.empty()) __static_lib_defs.clear();
            currentFile = fileName;
            lineNumber = 0;
            return process(lines, options, outputName);
        }

        std::vector<OmniaString> PreProcessor::process(std::vector<OmniaString> lines, PreProcessorOptions options, OmniaString outputName)
        {
            m_options = options;
            std::vector<OmniaString> finalCode = resolveIncludes(lines, currentFile);
            if (finalCode.size() == 0)
            {
                printError(D__ASSEMBLER_ERR_RESOLVE_INCLUDES_FAILED, *getOutputHandler());
                return std::vector<OmniaString>();
            }
            
            if (options.makeStaticLib && options.genHeader)
            {
                StringBuilder __sb(".include_guard(__");
                OmniaString __ig = outputName;
                if (__ig.contains("/"))
                    __ig = __ig.substr(__ig.lastIndexOf("/") + 1);
                __ig = __ig.replaceAll(".", "_");
                __ig = __ig.replaceAll("-", "_");
                __static_lib_defs.push_back(__sb.add(__ig).add(")").get());
                __static_lib_defs.push_back("");
                __static_lib_defs.push_back("");

                bool __export = false;
                for (auto& line : finalCode)
                {
                    line = line.trim();
                    if (!__export && line.equals("@@_export_start"))
                    {
                        __export = true;
                        continue;
                    }
                    else if (__export && line.equals("@@_export_end"))
                    {
                        __static_lib_defs.push_back("");
                        __static_lib_defs.push_back("");
                        __export = false;
                        continue;
                    }
                    else if (__export)
                    {
                        __static_lib_defs.push_back(line);
                        continue;
                    }
                }
            }
            
            finalCode = removeComments(finalCode);
            if (finalCode.size() == 0)
            {
                printError(D__ASSEMBLER_ERR_REMOVE_COMMENTS_FAILED, *getOutputHandler());
                return std::vector<OmniaString>();
            }
            finalCode = resolveDefines(finalCode);
            if (finalCode.size() == 0)
            {
                printError(D__ASSEMBLER_ERR_RESOLVE_DEFINES_FAILED, *getOutputHandler());
                return std::vector<OmniaString>();
            }
            finalCode = resolveAliases(finalCode);
            if (finalCode.size() == 0)
            {
                printError(D__ASSEMBLER_ERR_RESOLVE_ALIASES_FAILED, *getOutputHandler());
                return std::vector<OmniaString>();
            }
            finalCode = resolveMacros(finalCode);
            if (finalCode.size() == 0)
            {
                printError(D__ASSEMBLER_ERR_RESOLVE_MACROS_FAILED, *getOutputHandler());
                return std::vector<OmniaString>();
            }
			finalCode = resolveCommandDirective(finalCode);
            if (finalCode.size() == 0)
            {
                printError(D__ASSEMBLER_ERR_RESOLVE_CMD_DIRECTIVE_FAILED, *getOutputHandler());
                return std::vector<OmniaString>();
            }
			finalCode = resolveDataDirective(finalCode);
            if (finalCode.size() == 0)
            {
                printError(D__ASSEMBLER_ERR_RESOLVE_DATA_DIRECTIVE_FAILED, *getOutputHandler());
                return std::vector<OmniaString>();
            }
            //m_dataSection.insert(m_dataSection.begin(), StringBuilder("reserve,  Single_Const,  ").add(Utils::intToHexStr(m_reserveCount)).get());
            
            if (!options.makeStaticLib)
            {
                m_dataSection.insert(m_dataSection.begin(), ":__load__:");
                m_dataSection.push_back("call,  Const_Const,  __main__,  0x0000");
                m_dataSection.push_back("end,  Single_Reg,   RV");
                finalCode.insert(finalCode.begin() + m_nextTopInst++, OmniaString("call,  Const_Const,   __load__,  0x0000"));
            }
            else 
            {
                m_dataSection.insert(m_dataSection.begin(), ":__init__:");
                m_dataSection.push_back("ret, Const_Const, 0");
            }
            return finalCode;
        }


		std::vector<OmniaString> PreProcessor::resolveCommandDirective(std::vector<OmniaString> lines)
		{
            std::vector<OmniaString> code;
            int32 __ln = 0;
			for (auto& l : lines)
            {
                _line = l;
                __ln++;
                findIncludedFileSource(lines, __ln);
                l = l.trim();
                if (!l.toLowerCase().startsWith(".command"))
                {
                    code.push_back(l);
                    continue;
                }
                code.push_back(l);
                OmniaString line = l.substr(8).trim();
                if (!line.startsWith("(") || !line.endsWith(")"))
                {
                    printError(D__ASSEMBLER_ERR_MISSING_PARENTH_IN_COMMAND_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
                    return std::vector<OmniaString>();
                }
                line = line.substr(1, line.length() - 1).trim();
				OmniaString::StringTokens __st = line.tokenize(",", true);
				if (__st.count() < 2)
				{
                    printError(D__ASSEMBLER_ERR_FEW_ARGS_IN_COMMAND_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
					return std::vector<OmniaString>();
				}
				OmniaString __cmd = __st.next().toLowerCase();
				OmniaString param = "", value = "";
				while (__st.hasNext())
				{
					param = __st.next().toLowerCase();
					if (!param.contains("="))
					{
                        printError(D__ASSEMBLER_ERR_MISSING_EQUALS_IN_COMMAND_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
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
            bool __struct = false;
            StringBuilder __struct_code;
            OmniaString __struct_name = "";
            int32 __ln = 0;
			for (auto& l : lines)
            {
                _line = l;
                __ln++;
                findIncludedFileSource(lines, __ln);
                l = l.trim();
                if (__struct)
                {
                    OmniaString line = l;
                    if (line.startsWith(".end_struct"))
                    {
                        code.push_back(line);
                        line = line.substr(11).trim();
                        if (!line.startsWith("(") || !line.endsWith(")"))
                        {
                            printError(D__ASSEMBLER_ERR_MISSING_PARENTH_IN_END_STRUCT_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
                            return std::vector<OmniaString>();
                        }
                        line = line.substr(1, line.length() - 1).trim();
                        if (line == __struct_name)
                        {
                            __struct = false;
                            __struct_name = "";
                            m_struct_defs.push_back(__struct_code.get());
                            __struct_code = StringBuilder();
                            continue;
                        }
                    }
                    else
                        code.push_back("@@__om_space__@@");
                    __struct_code.add(l).add("\n");
                    continue;
                }
                if (!l.toLowerCase().startsWith(".data"))
                {
                    code.push_back(l);
                    continue;
                }
                code.push_back(l);
                OmniaString line = l.substr(5).trim();
                if (!line.startsWith("="))
                {
                    printError(D__ASSEMBLER_ERR_MISSING_EQUALS_IN_DATA_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
                    return std::vector<OmniaString>();
                }
                line = line.substr(1).trim();
				OmniaString::StringTokens __st;
				if (line.toLowerCase().startsWith("reserve"))
				{
					line = line.substr(7).trim();
                	if (!line.startsWith("(") || !line.endsWith(")"))
					{
                        printError(D__ASSEMBLER_ERR_MISSING_PARENTH_IN_RESERVE_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
                        return std::vector<OmniaString>();
					}
                	line = line.substr(1, line.length() - 1).trim();
					__st = line.tokenize(",", true);
					OmniaString param = "", value = "";
					while (__st.hasNext())
					{
						param = __st.next();
						m_reserves[param.cpp()] = (MemAddress)(m_nextReserve++);
                        m_reserveCount++;
                        m_symTable.m_reserves[(MemAddress)(m_nextReserve - 1)] = param;
					}
                    m_dataSection.push_back(StringBuilder("reserve,  Single_Const,  ").add(Utils::intToHexStr(__st.count())).get());
				}
                else if (line.toLowerCase().startsWith("array"))
				{
					line = line.substr(5).trim();
                	if (!line.startsWith("(") || !line.endsWith(")"))
					{
                        printError(D__ASSEMBLER_ERR_MISSING_PARENTH_IN_ARRAY_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
                        return std::vector<OmniaString>();
					}
                	line = line.substr(1, line.length() - 1).trim();
					__st = line.tokenize(",", true);
                    if (__st.count() != 2)
                    {
                        printError(D__ASSEMBLER_ERR_WRONG_ARGS_IN_ARRAY_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
                        return std::vector<OmniaString>();
                    }
					OmniaString def = __st.next();
                    OmniaString __tmp = __st.next();
                    if (!Utils::isInt(__tmp))
                    {
                        printError(D__ASSEMBLER_ERR_NON_INT_ARRAY_SIZE, *getOutputHandler(), "", _line, currentFile, lineNumber);
                        return std::vector<OmniaString>();
                    }

                    m_reserves[OmniaString("%").add(def).cpp()] = (MemAddress)m_nextReserve;
                    m_symTable.m_reserves[(MemAddress)m_nextReserve] = OmniaString("%").add(def);
                    m_nextReserve += Utils::strToInt(__tmp);
                    m_reserveCount += Utils::strToInt(__tmp);
				}
				else if (line.toLowerCase().startsWith("load_string"))
				{
					line = line.substr(11).trim();
                	if (!line.startsWith("(") || !line.endsWith(")"))
					{
                        printError(D__ASSEMBLER_ERR_MISSING_PARENTH_IN_LOAD_STRING_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
                        return std::vector<OmniaString>();
					}
                	line = line.substr(1, line.length() - 1).trim();
					__st = line.tokenize(",", true);
					if (__st.count() != 2)
					{
                        printError(D__ASSEMBLER_ERR_WRONG_ARGS_IN_LOAD_STRING_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
                        return std::vector<OmniaString>();
					}
					OmniaString param = __st.next();
					OmniaString __str = __st.next();
					if (!__str.endsWith("\"") || !__str.startsWith("\""))
					{
                        printError(D__ASSEMBLER_ERR_MISSING_DOUBLE_QUOTE_IN_LOAD_STRING_DIR, *getOutputHandler(), "", _line, currentFile, lineNumber);
                        return std::vector<OmniaString>();
					}
					if (!hasReserved(param))
					{
                        printError(D__ASSEMBLER_ERR_SYM_NOT_FOUND_IN_LOAD_STRING_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
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
                    m_nextReserve += __str_stream.size() + 1;
				}
                else if (line.toLowerCase().startsWith("struct"))
				{
                    line = line.substr(6).trim();
                    if (!line.startsWith("(") || !line.endsWith(")"))
                    {
                        printError(D__ASSEMBLER_ERR_MISSING_PARENTH_IN_STRUCT_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
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


        void PreProcessor::findIncludedFileSource(std::vector<OmniaString>& __source, int32 __current_index)
        {
            if (__current_index >= __source.size())
            {
                currentFile = "Generated_Source";
                lineNumber = __current_index;
                return;
            }
            //lineNumber = 0;
            int32 __level = 0;
            OmniaString __line = "";
            int32 __line_count = 0;
            std::vector<std::pair<int32, OmniaString>> __stack;
            for (int32 __i = 0; __i <= __current_index; __i++)
            {
                __line = __source[__i].trim();
                if (__line.startsWith("@@__om_included_file_here"))
                {
                    if (__line.contains("(") && __line.contains(")"))
                    {
                        __level++;
                        currentFile = __line.substr(__line.indexOf("(") + 1, __line.lastIndexOf(")")).trim();
                        __stack.push_back({ __line_count - 2, currentFile }); //Don't really know why -2
                        __line_count = 0;
                    }
                }
                else if (__line.startsWith("@@__om_end_included_file_here"))
                {
                    if (__level > 1)
                    {
                        __level--;
                        __stack.pop_back();
                        __line_count = __stack[__stack.size() - 1].first;
                        currentFile = __stack[__stack.size() - 1].second;
                        continue;
                    }
                }
                if (__stack.size() > 0)
                    __stack[__stack.size() - 1].first++;
            }
            /*int32 __j = 0;
            for (int32 __i = __current_index; __i > 0; __i--)
            {
                __line = __source[__i].trim();
                __j++;
                if (__line.startsWith("@@__om_end_included_file_here"))
                {
                    __level++;
                }
                else if (__line.startsWith("@@__om_included_file_here"))
                {
                    if (__level > 0)
                    {
                        __level--;
                        continue;
                    }
                    if (!__line.contains("(") || !__line.contains(")")) continue;
                    currentFile = __line.substr(__line.indexOf("(") + 1, __line.lastIndexOf(")")).trim();
                    lineNumber = __current_index - __i;
                    return;
                }
            }*/
            lineNumber = __stack[__stack.size() - 1].first;
        }


        std::vector<OmniaString> PreProcessor::resolveAliases(std::vector<OmniaString> lines)
        {
            std::vector<OmniaString> code;
            int32 __ln = 0;
            for (auto& l : lines) //Aliases round
            {
                _line = l;
                __ln++;
                findIncludedFileSource(lines, __ln);
                l = l.trim();
                if (!l.toLowerCase().startsWith(".alias "))
                {
                    code.push_back(l);
                    continue;
                }
                code.push_back(l);
                OmniaString line = l.substr(7).trim();
                if (!line.contains("="))
                {
                    printError(D__ASSEMBLER_ERR_MISSING_EQUALS_IN_ALIAS_DIRECTIVE, *getOutputHandler(), "", _line, currentFile, lineNumber);
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
            int32 __ln = 0;
            for (auto& l : lines) //Macros round
            {
                _line = l;
                __ln++;
                findIncludedFileSource(lines, __ln);
                l = l.trim();
                if (!l.toLowerCase().startsWith(".macro "))
                {
                    code.push_back(l);
                    continue;
                }
                code.push_back(l);
                OmniaString line = l.substr(7).trim();
                Macro mac(line);
                if (!mac.isValid())
                {
                    printError(D__ASSEMBLER_ERR_INVALID_MACRO_SYNTAX, *getOutputHandler(), "", _line, currentFile, lineNumber);
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
                if (line.startsWith("//") && !multiLineComment)
                {
                    code.push_back(line);
                    continue;
                }
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
                if (multiLineComment)
                {
                    code.push_back("@@__om_space__@@");
                    continue;
                }
                if (line.contains("/*")) newRound = true;
                code.push_back(line);
            }
            if (newRound) code = removeComments(code);
            return code;
        }

        std::vector<OmniaString> PreProcessor::resolveIncludes(std::vector<OmniaString> mainFile, OmniaString curFile)
        {
            int tx, ty;
            Utils::get_terminal_size(tx, ty);
            getOutputHandler()->newLine();
            Utils::printTitle("INCLUSION TREE", *getOutputHandler(), tx);
            std::vector<OmniaString> ilines = resolveIncludes_r(mainFile, curFile);
            if (ilines.size() == 0) return ilines;
            ilines.insert(ilines.begin(), OmniaString("@@__om_included_file_here(").add(curFile).add(")"));
            ilines.push_back(OmniaString("@@__om_end_included_file_here"));
            getOutputHandler()->fc_brightGrey().print(Utils::duplicateChar('-', tx)).newLine().newLine();
            std::vector<OmniaString> nlines;
            bool include_skip = false;
            uint32 grdCount = 0;
            for (auto& line : ilines)
            {
                line = line.trim();
                if (line.toLowerCase().startsWith(".include_guard(") && line.endsWith(")"))
                {
                    nlines.push_back(line);
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
                    nlines.push_back(line);
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
                if (include_skip)
                {
                    nlines.push_back("@@__om_space__@@");
                    continue;
                }
                nlines.push_back(line);
            }
            return nlines;
        }
        
        std::vector<OmniaString> PreProcessor::resolveIncludes_r(std::vector<OmniaString> mainFile, OmniaString curFile, int32 level)
        {
            _line = "";
            lineNumber = 0;
            currentFile = curFile;
            if (!curFile.contains("/"))
                m_currentIncludeDir = ".";
            else
                m_currentIncludeDir = curFile.substr(0, curFile.lastIndexOf("/")).trim();
            std::vector<OmniaString> incl;
            std::vector<OmniaString> tmp;
            getOutputHandler()->fc_brightGrey().print(Utils::duplicateChar(' ', level * 2)).fc_cyan().print("@").fc_magenta().print(curFile).newLine().tc_reset();
            for (auto& l : mainFile) //Includes round
            {
                _line = l;
                lineNumber++;
                if (!l.toLowerCase().trim().startsWith(".include "))
                {
                    tmp.push_back(l);
                    continue;
                }
                tmp.push_back(l);
                OmniaString line = l.substr(9).trim();
                if (!line.startsWith("[") || !line.endsWith("]"))
                {
                    printError(D__ASSEMBLER_ERR_MISSING_SQUARE_BRACKET_IN_INCLUDE_DIR, *getOutputHandler(), "", _line, currentFile, lineNumber);
                    return std::vector<OmniaString>();
                }
                line = line.substr(1, line.length() - 1).trim();
                OmniaString __ln = StringBuilder(m_currentIncludeDir).add("/").add(line).get();
                bool __inc_found = false;
                if (std::filesystem::exists(__ln.cpp()))
                {
                    line = __ln;
                    __inc_found = true;
                }
                else
                {
                    for (auto __inc_dir : m_options.includePaths)
                    {
                        __inc_dir = __inc_dir.trim();
                        if (!__inc_dir.endsWith("/")) __inc_dir.add("/");
                        OmniaString __new_path = StringBuilder(__inc_dir).add(line).get();
                        __new_path = __new_path.replaceAll("//", "/");
                        if (std::filesystem::exists(__new_path.cpp()))
                        {
                            __inc_found = true;
                            line = __new_path;
                            break;
                        }
                    }
                }
                if (!__inc_found || !Utils::readFile(line, incl))
                {
                    int32 tx, ty;
                    Utils::get_terminal_size(tx, ty);
                    printError(D__ASSEMBLER_ERR_UNABLE_TO_READ_FILE_IN_INCLUDE_DIRECTIVE, *getOutputHandler(), line, _line, currentFile, lineNumber, false);
                    return std::vector<OmniaString>();
                }
                if (incl.size() > 0)
                {
                    int32 ln = (signed)lineNumber;
                    OmniaString __tmp_cid = m_currentIncludeDir;
                    std::vector<OmniaString> nincl = resolveIncludes_r(incl, line, level + 1);
                    if (nincl.size() == 0) return nincl;
                    currentFile = curFile;
                    tmp.push_back(OmniaString("@@__om_included_file_here(").add(line).add(")"));
                    tmp.insert(tmp.end(), nincl.begin(), nincl.end());
                    tmp.push_back(OmniaString("@@__om_end_included_file_here"));
                    lineNumber = ln;
                    m_currentIncludeDir = __tmp_cid;
                }
            }
            return tmp;
        }
        
        std::vector<OmniaString> PreProcessor::resolveDefines(std::vector<OmniaString> lines)
        {
            std::vector<OmniaString> code;
            int32 __ln = 0;
            for (auto& l : lines)
            {
                _line = l;
                __ln++;
                findIncludedFileSource(lines, __ln);
                l = l.trim();
                if (l.toLowerCase().startsWith(".def "))
                {
                    code.push_back(l);
                    OmniaString line = l.substr(5).trim();
                    if (line == "")
                    {
                        Utils::message("Warning: No name specified in .def directive.", *getOutputHandler(), eMsgType::Warning);
                        continue;
                    }
                    if (line.contains(" "))
                    {
                        OmniaString __def = line.substr(0, line.indexOf(" ")).trim();
                        line = line.substr(line.indexOf(" ") + 1).trim();
                        if (hasDefine(__def))
                            Utils::message(StringBuilder("Warning: <").add(__def).add("> symbol redefined here.").get(), *getOutputHandler(), eMsgType::Warning);
                        m_defines[__def.cpp()] = { line, true };
                        continue;
                    }
                    else
                    {
                        if (hasDefine(line))
                            Utils::message(StringBuilder("Warning: <").add(line).add("> symbol redefined here.").get(), *getOutputHandler(), eMsgType::Warning);
                        m_defines[line.cpp()] = { "", true };
                        continue;
                    }
                }
                else if (l.toLowerCase().startsWith(".undef "))
                {
                    code.push_back(l);
                    OmniaString line = l.substr(7).trim();
                    if (line == "")
                    {
                        Utils::message(StringBuilder("Warning: No name specified in .undef directive.").get(), *getOutputHandler(), eMsgType::Warning);
                        continue;
                    }
                    if (!hasDefine(line))
                    {
                        Utils::message(StringBuilder("Warning: <").add(line).add("> symbol undefined.").get(), *getOutputHandler(), eMsgType::Warning);
                        continue;
                    }
                    m_defines[line.cpp()].second = false;
                    continue;
                }
                else if (l.toLowerCase().startsWith(".ifdef "))
                {
                    code.push_back(l);
                    OmniaString line = l.substr(7).trim();
                    if (line == "")
                    {
                        Utils::message(StringBuilder("Warning: No define specified in .ifdef directive.").get(), *getOutputHandler(), eMsgType::Warning);
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
                    code.push_back(l);
                    OmniaString line = l.substr(8).trim();
                    if (line == "")
                    {
                        Utils::message(StringBuilder("Warning: No define specified in .ifndef directive.").get(), *getOutputHandler(), eMsgType::Warning);
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
                    code.push_back(l);
                    if (m_def_stack.size() == 0)
                    {
                        Utils::message(StringBuilder("Warning: Dead .else directive. Missing .if* statement").get(), *getOutputHandler(), eMsgType::Warning);
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
                    code.push_back(l);
                    OmniaString line = l.substr(9).trim();
                    if (m_def_stack.size() == 0)
                    {
                        Utils::message(StringBuilder("Warning: Dead .elifdef directive. Missing .if* statement").get(), *getOutputHandler(), eMsgType::Warning);
                        continue;
                    }
                    if (line == "")
                    {
                        Utils::message(StringBuilder("Warning: No define specified in .elifdef directive.").get(), *getOutputHandler(), eMsgType::Warning);
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
                    code.push_back(l);
                    OmniaString line = l.substr(10).trim();
                    if (m_def_stack.size() == 0)
                    {
                        Utils::message(StringBuilder("Warning: Dead .elifndef directive. Missing .if* statement").get(), *getOutputHandler(), eMsgType::Warning);
                        continue;
                    }
                    if (line == "")
                    {
                        Utils::message(StringBuilder("Warning: No define specified in .elifndef directive.").get(), *getOutputHandler(), eMsgType::Warning);
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
                    code.push_back(l);
                    if (m_def_stack.size() == 0)
                    {
                        Utils::message(StringBuilder("Warning: Dead .endif directive. Missing .if* or .else* statement").get(), *getOutputHandler(), eMsgType::Warning);
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
                    else
                        code.push_back("@@__om_space__@@");
                    continue;
                } 
            }
            return code;
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
            p__save_final_code = false;
            p__generate_header = false;
			p__input_file_path = "";
            p__output_file_path = "";
            p__output_file_dbg_table = "";
            m_static_libs.clear();
            m_extern_subroutines.clear();
            m_extern_links.clear();
            m_slib_reserve_addresses.clear();
            m_export_to_static_lib = false;
			if (argc > 1)
			{
				for (int i = 1; i < argc; i++)
				{
					if (OmniaString(argv[i]).trim().equals("--input-file") || OmniaString(argv[i]).trim().equals("-i"))
					{
						if (i + 1 >= argc)
                            return printError(D__ASSEMBLER_ERR_NO_INPUT_FILE, out);
						i++;
						p__input_file_path = OmniaString(argv[i]);
					}
                    else if (OmniaString(argv[i]).trim().equals("--output-file") || OmniaString(argv[i]).trim().equals("-o"))
					{
						if (i + 1 >= argc)
                            return printError(D__ASSEMBLER_ERR_NO_OUTPUT_FILE, out);
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
                    else if (OmniaString(argv[i]).trim().equals("--include-path") || OmniaString(argv[i]).trim().equals("-I"))
					{
						if (i + 1 >= argc)
                        {
                            Utils::message("Warning: No path specified after --include-path option.", out, eMsgType::Warning);
                            continue;
                        }
						i++;
                        m_options.includePaths.push_back(OmniaString(argv[i]));
                    }
                    else if (OmniaString(argv[i]).trim().equals("--lib-include-path") || OmniaString(argv[i]).trim().equals("-L"))
					{
						if (i + 1 >= argc)
                        {
                            Utils::message("Warning: No path specified after --lib-include-path option.", out, eMsgType::Warning);
                            continue;
                        }
						i++;
                        m_options.libIncludePaths.push_back(OmniaString(argv[i]));
                    }
                    else if (OmniaString(argv[i]).trim().equals("--save-final-code") || OmniaString(argv[i]).trim().equals("-SC"))
					{
                        p__save_final_code = true;
                    }
                    else if (OmniaString(argv[i]).trim().equals("--make-static-lib") || OmniaString(argv[i]).trim().equals("-SL"))
					{
                        p__build_static_lib = true;
                        m_options.makeStaticLib = true;
                    }
                    else if (OmniaString(argv[i]).trim().equals("--generate-header") || OmniaString(argv[i]).trim().equals("-gh"))
					{
                        p__generate_header = true;
                        m_options.genHeader = true;
                    }
                    else if (OmniaString(argv[i]).trim().equals("--DEBUG") || OmniaString(argv[i]).trim().equals("-D"))
                    {
                        p__dbg_save_code = true;
                        p__dbg_symbol_table = true;
                        p__save_final_code = true;
                    }
                    else if (OmniaString(argv[i]).trim().startsWith("--link-static") || OmniaString(argv[i]).trim().startsWith("-ls"))
					{
                        bool __lsd = false;
                        if (OmniaString(argv[i]).trim().equals("--link-static") || OmniaString(argv[i]).trim().equals("-ls"))
                            __lsd = false;
                        else if (OmniaString(argv[i]).trim().equals("--link-static-debug") || OmniaString(argv[i]).trim().equals("-lsd"))
                            __lsd = true;
                        else
                        {
                            Utils::message("Warning: Unknown parameter for --link-static-* option.", *getOutputHandler(), eMsgType::Warning);
                            continue;
                        }
						if (i + 1 >= argc)
                        {
                            Utils::message("Warning: No path specified after --link-static option.", out, eMsgType::Warning);
                            continue;
                        }
						i++;
                        OmniaString __lib(argv[i]);
                        if (!__lib.endsWith(".oslib"))
                            __lib = __lib.add(".oslib");
                        TMemoryList __lib_code;
                        OmniaString lib_name(argv[i]);
                        if (lib_name.contains("/"))
                            lib_name = lib_name.substr(lib_name.lastIndexOf("/") + 1).trim();
                        if (lib_name.endsWith(".oslib"))
                            lib_name = lib_name.substr(0, lib_name.length() - 6).trim();
                        if (std::filesystem::exists(__lib.cpp()))
                        {
                            if (!Interpreter::instance().loadFromFile(__lib, __lib_code))
                                return printError(D__ASSEMBLER_ERR_FAILED_TO_LOAD_STATIC_LIB, out, __lib);
                        }
                        else
                        {
                            OmniaString __lp;
                            bool found = false;
                            for (auto __lic : m_options.libIncludePaths)
                            {
                                __lp = __lic.add("/").add(__lib);
                                if (std::filesystem::exists(__lp.cpp()))
                                {
                                    if (!Interpreter::instance().loadFromFile(__lp, __lib_code))
                                        return printError(D__ASSEMBLER_ERR_FAILED_TO_LOAD_STATIC_LIB, out, __lib);
                                    found = true;
                                    break;
                                }
                            }
                            if (!found)
                                return printError(D__ASSEMBLER_ERR_STATIC_LIB_NOT_FOUND , out, __lib);
                            else
                                __lib = __lp;
                        }
                        tStaticLib __slib;
                        MemAddress __lib_init_addr = oasm_nullptr;
                        if (__lib_code.size() > 0)
                        {
                            __lib_init_addr = __lib_code[0].val();
                            STDVEC_REMOVE(__lib_code, 0);

                            word __export_cnt = __lib_code[0].val();
                            STDVEC_REMOVE(__lib_code, 0);
                            
                            if (__export_cnt > 0)
                            {
                                TMemoryList __tmp_str_stream;
                                for (uint16 i = 0; i < __export_cnt; i++)
                                {
                                    tExternSymbol __sym;
                                    __sym.type = (eExternSymType)__lib_code[0].val();
                                    STDVEC_REMOVE(__lib_code, 0);
                                    __sym.address = __lib_code[0].val();
                                    STDVEC_REMOVE(__lib_code, 0);
                                    word __c = 0;
                                    do
                                    {
                                        __c = __lib_code[0].val();
                                        STDVEC_REMOVE(__lib_code, 0);
                                        __tmp_str_stream.push_back(__c);
                                    } while (__c != 0);
                                    __sym.name = BitEditor::constStreamToString(__tmp_str_stream);
                                    __tmp_str_stream.clear();
                                    __slib.externSymbols.push_back(__sym);
                                }
                            }

                            __slib.reserveCount = __lib_code[0].val();
                            STDVEC_REMOVE(__lib_code, 0);
                            word __ref_count = __lib_code[0].val();
                            STDVEC_REMOVE(__lib_code, 0);
                            for (MemAddress __addr = 0; __addr < __ref_count; __addr++)
                            {
                                __slib.reserveRefs.push_back(__lib_code[0]);
                                STDVEC_REMOVE(__lib_code, 0);
                            }
                        }
                        __slib.name = lib_name;
                        __slib.code = __lib_code;
                        __slib.baseAddress = 0;
                        __slib.debugTable = __lsd;
                        __slib.filePath = __lib;
                        m_extra_code.push_back(StringBuilder("$.extern(sub-routine, ").add(__slib.name).add("::__init__, ").add(Utils::intToHexStr(__lib_init_addr)).add(")").get());
                        m_static_libs.push_back(__slib);
                        Utils::message(StringBuilder("Loaded static library: <").add(__lib).add("> as <").add(lib_name).add("> (").add((int64)(__lib_code.size() * sizeof(BitEditor))).add(" bytes)").get(), *getOutputHandler(), eMsgType::Success);
                    }
                }
			}
            if (p__dbg_save_code && !p__dbg_symbol_table) p__dbg_save_code = false;
			if (p__input_file_path.trim() == "")
                return printError(D__ASSEMBLER_ERR_NO_INPUT_FILE, out);
			if (p__output_file_path.trim() == "")
            {
                if (p__build_static_lib)
                    p__output_file_path = p__input_file_path.substr(0, p__input_file_path.lastIndexOf(".")).add(".oslib");
                else
                    p__output_file_path = p__input_file_path.substr(0, p__input_file_path.lastIndexOf(".")).add(".oex");
            }
			TMemoryList __program = assemble(p__input_file_path);
            if (!createExecutableFile(p__output_file_path, __program))
                return printError(D__ASSEMBLER_ERR_FAILED_TO_CREATE_EXEC, out);
            if (p__dbg_symbol_table)
            {
                p__output_file_dbg_table = p__output_file_path.substr(0, p__output_file_path.lastIndexOf(".")).add(".odb");
                if (!createDebugTableFile(p__output_file_dbg_table))
                    return printError(D__ASSEMBLER_ERR_FAILED_TO_CREATE_DBG_FILE, out);
            }
			return 0;
		}

        bool Assembler::createExecutableFile(OmniaString __outputFile, TMemoryList __program)
        {
            if (__program.size() == 0) return false;
            if (p__build_static_lib && m_slib_reserve_addresses.size() > 0)
            {
                __program.insert(__program.begin(), PreProcessor::instance().m_nextReserve);
                __program.insert(__program.begin() + 1, m_slib_reserve_addresses.size());
                __program.insert(__program.begin() + 2, m_slib_reserve_addresses.begin(), m_slib_reserve_addresses.end());
            }
            TMemoryList __temp;
            if (p__build_static_lib)
            {
                __temp.push_back((word)m_exported_symbols.size());
                if (m_exported_symbols.size() > 0)
                {
                    TMemoryList __tmp_str_stream;
                    for (auto& __sym : m_exported_symbols)
                    {
                        __temp.push_back((word)__sym.type);
                        __temp.push_back((word)__sym.address);
                        __tmp_str_stream = BitEditor::stringToConstSream(__sym.name);
                        __temp.insert(__temp.end(), __tmp_str_stream.begin(), __tmp_str_stream.end());
                        if (__temp[__temp.size() - 1] != 0)
                            __temp.push_back(0);
                    }
                }
                __temp.insert(__temp.end(), __program.begin(), __program.end());
                __program = __temp;
                __program.insert(__program.begin(), (word)m_labels["__init__"]);
            }
            std::ofstream writeFile;
            writeFile.open(__outputFile.cpp(), std::ios::out | std::ios::binary);
            writeFile.write((char*)(&__program[0]), __program.size() * sizeof(word));
            writeFile.close();
            Utils::message(StringBuilder("Generated executable file: <").add(__outputFile).add("> (").add((int64)(__program.size() * sizeof(BitEditor))).add(" bytes)").get(), *getOutputHandler(), eMsgType::Success);
            return true;
        }

        bool Assembler::createDebugTableFile(OmniaString __outputFile)
        {
            if (!p__dbg_symbol_table) return true;
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
            if (PreProcessor::instance().m_symTable.m_source.size() > 0 && p__dbg_save_code)
            {
                writeFile << ".source = { \n";
                for (auto& __line : PreProcessor::instance().m_symTable.m_source)
                {
                    OmniaString __src_line = __line.second;
                    if (__src_line.startsWith("lda_str,") && !__src_line.contains("\""))
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
                writeFile << "}\n";
            }
            if (p__dbg_save_code && m_static_libs.size() > 0)
            {
                for (auto& __lib : m_static_libs)
                {

                    if (__lib.debugTable && __lib.filePath.trim().endsWith(".oslib"))
                    {
                        writeFile << ".attach_lib_debug_table " << __lib.filePath.trim().substr(0, __lib.filePath.length() - 5).add("odb").cpp() << " " << Utils::intToHexStr(__lib.baseAddress).cpp() << "\n";
                        continue;
                    }

                    MemAddress __curr_addr = __lib.baseAddress;
                    uint16 __tokens_per_line = 4;
                    uint16 __counter = 0;
                    writeFile << ".static_lib " << __lib.name.cpp() << " = { ";
                    for (auto& __token : __lib.code)
                    {
                        if (__counter++ % __tokens_per_line == 0)
                            writeFile << "\n\t\t" << Utils::intToHexStr(__curr_addr).cpp() << ":  ";
                        writeFile << Utils::intToHexStr(__token.val()).cpp() << ", ";
                        __curr_addr++;
                    }
                    writeFile << "\n}\n";
                }
            }
            writeFile.close();
            Utils::message(StringBuilder("Generated debug-file: <").add(__outputFile).add(">").get(), *getOutputHandler(), eMsgType::Success);
            return true;
        }

        TMemoryList Assembler::linkStaticLibs(TMemoryList code)
        {
            TMemoryList __code = code;
            word __tmp_res_count = PreProcessor::instance().m_nextReserve;
            for (auto& lib : m_static_libs)
            {
                lib.baseAddress = __code.size();
                for (auto& __ref : lib.reserveRefs)
                {
                    lib.code[__ref.val()] += (word)__tmp_res_count;
                }
                __code.insert(__code.end(), lib.code.begin(), lib.code.end());
                __tmp_res_count += lib.reserveCount;
            }
            uint32 __check = 0;
            bool __next = false;
            for (auto& link : m_extern_links)
            {
                if (link.second.startsWith("::"))
                {
                    for (auto& lib : m_static_libs)
                    {
                        for (auto& __sym : lib.externSymbols)
                        {
                            if (link.second == StringBuilder("::").add(__sym.name).get())
                            {
                                if (__sym.type == eExternSymType::SubRoutine)
                                    __code[link.first] = lib.baseAddress + __sym.address;
                                else
                                    continue;
                                __check++;
                                __next = true;
                                break;
                            }
                        }
                        if (__next) break;
                    }
                    if (!__next)
                    {
                        //TODO: Error, unknown symbol link.second
                        std::cout << "Linker Error: Unknown symbol\n";
                        return TMemoryList();
                    }
                    __next = false;
                    continue;
                }
                for (auto& ext : m_extern_subroutines)
                {
                    if (link.second == ext.getScopedName())
                    {
                        for (auto& lib : m_static_libs)
                        {
                            if (lib.name == ext.libName && __code[link.first] == 0)
                            {
                                __code[link.first] = lib.baseAddress + ext.localAddress;
                                __check++;
                                break;
                            }
                        }
                    }
                }
            }
            if (__check != m_extern_links.size())
            {
                printError(D__ASSEMBLER_ERR_DISCREPANT_EXTERN_SUBROUTINES_SIZE, *getOutputHandler());
                return TMemoryList();
            }
            return __code;
        }

		TMemoryList Assembler::assemble(OmniaString __source_file_path)
		{
            std::vector<OmniaString> __source = PreProcessor::instance().open(__source_file_path, p__output_file_path, m_options);
            if (__source.size() == 0) return TMemoryList();
            std::vector<OmniaString> source = PreProcessor::instance().m_dataSection;
            for (auto& lib : m_static_libs)
            {
                source.insert(source.end() - 2, OmniaString("call, Const_Const, ").add(lib.name).add("::__init__, 0"));
            }
            source.insert(source.begin(), __source.begin(), __source.end());
            source.insert(source.end(), m_extra_code.begin(), m_extra_code.end());
            if (p__dbg_save_code)
            {
                std::map<MemAddress, OmniaString> __dbg_src;
                MemAddress __curr_addr = 0;
                OmniaString::StringTokens __st;
                for (auto& __line : source)
                {
                    if (__line.startsWith("@@") || __line.startsWith("//") || __line.startsWith(".") || __line.startsWith("$."))
                        continue;
                    __line = __line.trim();
                    if (__line.startsWith(":") && __line.endsWith(":")) continue;
                    __dbg_src[__curr_addr] = __line;
                    __st = __line.tokenize(",", true);
                    __curr_addr += __st.count();
                }
                PreProcessor::instance().m_symTable.m_source = __dbg_src;
            }
            if (p__save_final_code)
            {
                OmniaString __sfc_file = p__output_file_path.substr(0, p__output_file_path.lastIndexOf(".")).add("__fc.oasm");
                std::ofstream __fc_file(__sfc_file.cpp(), std::ofstream::out | std::ofstream::trunc);
                for (auto& line : source)
                    __fc_file << line.cpp() << "\n";
                __fc_file.close();
                Utils::message(StringBuilder("Generated final-step code: <").add(__sfc_file).add(">").get(), *getOutputHandler(), eMsgType::Success);
            }
			std::vector<OmniaString> code = resolveKeyWords(source);
            TMemoryList __assembled = assemble(code);
			return linkStaticLibs(__assembled);
		}

		TMemoryList Assembler::assemble(std::vector<OmniaString>& __source)
		{
			TMemoryList __code;
			OmniaString::StringTokens __sb;
			OmniaString __data = "";
            MemAddress __addr = 0;
            bool found = false;
			for (auto& __line : __source)
			{
				__sb = __line.tokenize(",", true);
				while (__sb.hasNext())
				{
					__data = __sb.next();
					if (__data == "") continue;
                    if (__data.contains("::"))
                    {
                        if (__data.startsWith("::"))
                        {
                            //__data = __data.substr(2).trim();
                            m_extern_links[__addr] = __data;
                            found = true;
                        }
                        else
                        {
                            for (auto& __ext : m_extern_subroutines)
                            {
                                if (__ext.getScopedName() == __data)
                                {
                                    m_extern_links[__addr] = __data;
                                    found = true;
                                    break;
                                }
                            }
                        }
                        if (!found)
                        {
                            printError(D__ASSEMBLER_ERR_EXTERN_SUBROUTINE_NOT_FOUND, *getOutputHandler(), __data, __line);
                            return TMemoryList();
                        }
                        __code.push_back(0);
                        __addr++;
                        continue;
                    }
					else if (!Utils::isInt(__data))
					{
						printError(D__ASSEMBLER_ERR_INVALID_ASSEMBLER_TOKEN, *getOutputHandler());
						return TMemoryList();
					}
					__code.push_back(Utils::strToInt(__data));
                    __addr++;
				}
			}
			return __code;
		}
		
		OmniaString Assembler::executeAssemblerFunction(OmniaString __func)
        {
            StringBuilder __generated_code;
            
            if (!__func.contains("(") || !__func.contains(")") || __func.lastIndexOf(")") <= __func.indexOf("("))
            {
                printError(D__ASSEMBLER_ERR_INVALID_AS_CMD_SYNTAX, *getOutputHandler(), OmniaString("$.").add(__func));
                return OmniaString("$.").add(__func);
            }
            OmniaString __params = __func.substr(__func.indexOf("(") + 1, __func.lastIndexOf(")")).trim();
            if (__func.startsWith("char"))
            {
                if (__params.length() < 3 || !__params.startsWith("\'") || !__params.endsWith("\'"))
                {
                    printError(D__ASSEMBLER_ERR_INVALID_AS_CMD_PARAM, *getOutputHandler(), OmniaString("$.").add(__func));
                    return OmniaString("$.").add(__func);
                }
                __params = __params.replaceAll("\\", "");
                __params = __params.substr(1, __params.length() - 1);
                if (__params.length() == 1)
                    __generated_code.add((int32)__params[0]);
                else
                {
                    printError(D__ASSEMBLER_ERR_INVALID_AS_CMD_PARAM, *getOutputHandler(), OmniaString("$.").add(__func));
                    return OmniaString("$.").add(__func);
                }
            }
            else if (__func.startsWith("extern"))
            {
                OmniaString::StringTokens __st = __params.tokenize(",", true);
                if (__st.count() != 3)
                {
                    printError(D__ASSEMBLER_ERR_WRONG_ARGS_IN_EXTERN_COMMAND, *getOutputHandler(), OmniaString("$.").add(__func));
                    return OmniaString("$.").add(__func);
                }
                if (__st.next() == "sub-routine")
                {
                    OmniaString __lbl = __st.next();
                    OmniaString __addr = __st.next();
                    if (!__lbl.contains("::"))
                    {
                        printError(D__ASSEMBLER_ERR_EXTERN_SUBROUTINE_NOT_FOUND_IN_AS_CMD, *getOutputHandler(), __lbl, __func);
                        return OmniaString("$.").add(__func);
                    }
                    OmniaString __lib = __lbl.substr(0, __lbl.indexOf("::")).trim();
                    __lbl = __lbl.substr(__lbl.indexOf("::") + 2).trim();
                    if (!isStaticLib(__lib))
                    {
                        printError(D__ASSEMBLER_ERR_UNKNOWN_LIB_IN_EXTERN_AS_CMD, *getOutputHandler(), __lib, __func);
                        return OmniaString("$.").add(__func);
                    }
                    if (!Utils::isInt(__addr))
                    {
                        printError(D__ASSEMBLER_ERR_NON_INtEGER_ADDRESS_IN_EXTERN_AS_CMD, *getOutputHandler(), __addr, __func);
                        return OmniaString("$.").add(__func);
                    }
                    tExternSubroutine __ext;
                    __ext.labelName = __lbl;
                    __ext.libName = __lib;
                    __ext.localAddress = Utils::strToInt(__addr);
                    m_extern_subroutines.push_back(__ext);
                }
            }
            else if (__func.startsWith("export"))
            {
                if (!p__build_static_lib) return "";
                OmniaString::StringTokens __st = __params.tokenize(",", true);
                if (__st.count() < 2)
                {
                    //TODO: Error wrong syntax
                    return OmniaString("$.").add(__func);
                }
                OmniaString __type = __st.next(), __symbol = "";
                while (__st.hasNext())
                {
                    tExternSymbol __sym;
                    __symbol = __st.next();
                    if (__type.toLowerCase() == "sub-routine")
                    {
                        __sym.type = eExternSymType::SubRoutine;
                        __sym.address = 0;
                        __sym.name = __symbol;
                        m_exported_symbols.push_back(__sym);
                    }
                    else if (__type.toLowerCase() == "reserve")
                    {
                        if (!PreProcessor::instance().hasReserved(__symbol))
                        {
                            //TODO: Error unknown reserve
                            std::cout << "Error: unknown reserve\n";
                            return OmniaString("$.").add(__func);
                        }
                        __sym.type = eExternSymType::Reserve;
                        __sym.address = PreProcessor::instance().m_reserves[__symbol.cpp()];;
                        __sym.name = __symbol;
                        m_exported_symbols.push_back(__sym);
                    }
                    else if (__type.toLowerCase() == "array")
                    {
                        if (!PreProcessor::instance().hasReserved(OmniaString("%").add(__symbol)))
                        {
                            //TODO: Error unknown reserve
                            std::cout << "Error: unknown array\n";
                            return OmniaString("$.").add(__func);
                        }
                        __sym.type = eExternSymType::Array;
                        __sym.address = PreProcessor::instance().m_reserves[OmniaString("%").add(__symbol).cpp()];;
                        __sym.name = __symbol;
                        m_exported_symbols.push_back(__sym);
                    }
                    else
                    {
                        //TODO: Error Unknown export type
                        return OmniaString("$.").add(__func);
                    }
                }

                return "";
            }
            else
            {
                printError(D__ASSEMBLER_ERR_UNKNOWN_AS_CMD, *getOutputHandler(), OmniaString("$.").add(__func));
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
            for (auto& l : lines)
            {
                if (l.startsWith("@@") || l.startsWith("//") || l.startsWith("."))
                {
                    continue;
                }
                code.push_back(l);
            }
            lines = code;
            code.clear();
            OmniaString __cmd = "";
            int32 __p_count = 0;
            bool can_finish = false;
            for (auto& l : lines)
            {
                while (l.contains("$."))
                {
                    __cmd = "";
                    can_finish = false;
                    int32 j = l.indexOf("$."), i = j + 2;
                    for ( ; i < lines.size(); i++)
                    {
                        if (l[i] == ' ') continue;
                        if (l[i] == '(') __p_count++;
                        else if (l[i] == ')')
                        {
                            can_finish = true;
                            __p_count--;
                        }
                        __cmd = __cmd.add(l[i]);
                        if (__p_count == 0 && can_finish)  break;
                    }
                    i++;
                    OmniaString __c = l.substr(j, i);
                    OmniaString __result = executeAssemblerFunction(__cmd);
                    if (__cmd.startsWith("$."))
                        return std::vector<OmniaString>();
                    l = l.replaceAll(__c, __result);
                }
            }
            for (auto& l : lines) //Resolve labels
            {
                l = l.trim();
				if (l.startsWith(":") && l.endsWith(":"))
				{
					l = l.substr(1, l.length() - 1).trim();
					if (l.contains(" ") || l.contains("\t"))
					{
                        printError(D__ASSEMBLER_ERR_INVALID_LABEL_NAME, *getOutputHandler(), l);
						return std::vector<OmniaString>();
					}
					m_labels[l.cpp()] = __addr;
                    if (p__dbg_symbol_table)
                        PP.m_symTable.m_labels[__addr] = l;
                    if (p__build_static_lib)
                    {
                        StringBuilder __sb("$.extern(sub-routine, ");
                        OmniaString lib_name = p__output_file_path;
                        if (lib_name.contains("/"))
                            lib_name = lib_name.substr(lib_name.lastIndexOf("/") + 1);
                        if (lib_name.contains(".oslib"))
                            lib_name = lib_name.substr(0, lib_name.lastIndexOf(".oslib")).trim();
                        __sb.add(lib_name).add("::").add(l).add(", ").add(Utils::intToHexStr(__addr)).add(")");
                        PreProcessor::instance().__static_lib_defs.push_back(__sb.get());
                    }
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
            if (p__build_static_lib && p__generate_header)
            { 
                OmniaString h_name = p__output_file_path;
                if (h_name.endsWith(".oslib"))
                    h_name = h_name.replaceAll(".oslib", ".oh");
                PP.__static_lib_defs.push_back("");
                PP.__static_lib_defs.push_back("");
                PP.__static_lib_defs.push_back(".close_include_guard");
                std::ofstream h_file(h_name.cpp(), std::ofstream::out | std::ofstream::trunc);
                for (auto& __l : PP.__static_lib_defs)
                    h_file << __l.cpp() << "\n";
                h_file.close();
                Utils::message(StringBuilder("Generated header file: <").add(h_name).add(">").get(), *getOutputHandler(), eMsgType::Success);
            }

            MemAddress __cur_addr = 0;
            for (auto& l : lines)
            {
				__new_line = StringBuilder();
                l = l.trim();
				__st = l.tokenize(",", true);
				while (__st.hasNext())
				{
                    __cur_addr++;
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
                        __cur_addr--;
                        __cur_addr += __str_stream.size();
					}
					else if (PP.hasReserved(__data.cpp()))
					{
						__new_line.add(Utils::intToHexStr(PP.m_reserves[__data.cpp()])).add(",");
                        if (p__build_static_lib)
                            m_slib_reserve_addresses.push_back(__cur_addr - 1);
					}
                    else if (__data.startsWith("%") && __data.indexOf("[") > 1 && __data.endsWith("]"))
					{
                        OmniaString __res_name = __data.substr(0, __data.indexOf("[")).trim();
                        OmniaString __off_str = __data.substr(__data.indexOf("[") + 1, __data.length() - 1);
                        if (!Utils::isInt(__off_str))
                        {
                            printError(D__ASSEMBLER_ERR_NON_CONST_ARRAY_INDEX, *getOutputHandler(), __off_str, l);
                            return std::vector<OmniaString>();
                        }
						__new_line.add(Utils::intToHexStr(PP.m_reserves[__res_name.cpp()] + Utils::strToInt(__off_str))).add(",");
					}
					else if (isLabel(__data.cpp(), __addr))
					{
						__new_line.add(Utils::intToHexStr(__addr)).add(",");
					}
                    else if (__data.contains("::"))
                    {
                        __new_line.add(__data).add(",");
                    }
                    else
					{
                        printError(D__ASSEMBLER_ERR_UNKNOWN_SYMBOL, *getOutputHandler(), __data, l);
						return std::vector<OmniaString>();
					}
                }
				code.push_back(__new_line.get());
			}

            for (auto& __ext : m_exported_symbols)
            {
                if (__ext.type == eExternSymType::SubRoutine)
                {
                    if (!isLabel(__ext.name, __ext.address))
                    {
                        //TODO: Error, unknown sub-routine name
                        std::cout << "Error: Unknown subroutine\n";
                        return std::vector<OmniaString>();
                    }
                }
            }

			return code;
		}
	}
}