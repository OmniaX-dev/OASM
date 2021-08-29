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
                error(ePreProcessorErrors::FailedToOpenFile, OmniaString("Failed to open source file: ").add(fileName), true);
                return std::vector<OmniaString>();
            }
            if (lines.size() == 0)
            {
                error(ePreProcessorErrors::EmptyFile, OmniaString("Empty source file provided: ").add(fileName), true);
                return std::vector<OmniaString>();
            }
			_line = "";
			m_reserveCount = 0;
			if (!m_includeGuards.empty()) m_includeGuards.clear();
			if (!m_aliases.empty()) m_aliases.clear();
			if (!m_reserves.empty()) m_reserves.clear();
            if (!m_dataSection.empty()) m_dataSection.clear();
            currentFile = fileName;
            return process(lines, options);
        }

        std::vector<OmniaString> PreProcessor::process(std::vector<OmniaString> lines, PreProcessorOptions options)
        {
            m_options = options;
            std::vector<OmniaString> finalCode = resolveIncludes(lines, currentFile);
            finalCode = removeComments(finalCode);
            finalCode = resolveAliases(finalCode);
            finalCode = resolveMacros(finalCode);
			finalCode = resolveCommandDirective(finalCode);
			finalCode = resolveDataDirective(finalCode);
			for (auto& _line : finalCode)
			{
				_line = _line.replaceAll(" ", "");
				_line = _line.replaceAll("\t", "");
				_line = _line.replaceAll("\n", "");
			}
            m_dataSection.push_back("0x1200,    0x0018,    0x003F,      __main__");
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
							m_dataSection.push_back(StringBuilder("req, ").add((word)eFlags::req_all).add(", ").add(value).get());
						else if (param.equals("stack") && Utils::isInt(value))
							m_dataSection.push_back(StringBuilder("req, ").add((word)eFlags::req_stack).add(", ").add(value).get());
						else if (param.equals("heap") && Utils::isInt(value))
							m_dataSection.push_back(StringBuilder("req, ").add((word)eFlags::req_heap).add(", ").add(value).get());
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
			for (auto& l : lines)
            {
                _line = l;
                lineNumber++;
                l = l.trim();
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
						param = __st.next().toLowerCase();
						m_dataSection.push_back(StringBuilder("reserve, 0x001A, 0x002E").get());
						//code.push_back(StringBuilder("mem, 0x0014, ").add(Utils::intToHexStr(m_reserveCount++)).add(", 0x002E").get());
						m_reserves[param.cpp()] = (MemAddress)(m_reserveCount++);
                        m_symTable.m_reserves[(MemAddress)(m_reserveCount - 1)] = param;
					}
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
					StringBuilder __sb("lda_str, ");
					for (auto& __mc : __str_stream)
						__sb.add(Utils::intToHexStr(__mc.val())).add(",");
					__str = __sb.get().substr(0, __sb.get().length() - 1);
					m_dataSection.push_back(__str);
					m_dataSection.push_back(StringBuilder("mem, 0x0014, ").add(Utils::intToHexStr(m_reserves[param.cpp()])).add(", 0x002F").get());
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
						if (++i >= argc && p__output_file_path.trim() != "")
                            p__output_file_dbg_table = p__output_file_path.substr(0, p__output_file_path.lastIndexOf(".")).add(".odb");
                        else if (i < argc)
						    p__output_file_dbg_table = OmniaString(argv[i]);
                        else
                        {
							out.print("Error: Output file must be specified before the --debug-table option, if you intend to not specify a file for it.").newLine();
							return 0xFFF9; //TODO: Add error code
                        }
                        p__dbg_symbol_table = true;
                    }
				}
			}
			if (p__input_file_path.trim() == "") return 0xFFFE; //TODO: Error
			if (p__output_file_path.trim() == "") return 0xFFFD; //TODO: Error
            if (p__dbg_symbol_table && p__output_file_dbg_table.trim() == "") return 0xFFFC; //TODO: Error
			TMemoryList __program = assemble("oasm_test_1.oasm");
            if (!createExecutableFile(p__output_file_path, __program))
            {
                out.print("Error: Failed to create executable.").newLine();
                return 0xCFCF; //TODO: Add error code
            }
            if (p__dbg_symbol_table)
            {
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
            return true;
        }

        bool Assembler::createDebugTableFile(OmniaString __outputFile)
        {
            StringBuilder __sb;
            std::ofstream writeFile;
            writeFile.open(__outputFile.cpp());
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
            writeFile.close();
            return true;
        }

		TMemoryList Assembler::assemble(OmniaString __source_file_path)
		{
            std::vector<OmniaString> __source = PreProcessor::instance().open(__source_file_path);
            std::vector<OmniaString> source = PreProcessor::instance().m_dataSection;
            source.insert(source.end(), __source.begin(), __source.end());
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
					else if (isLabel(__data.cpp(), __addr))
					{
						__new_line.add(Utils::intToHexStr(__addr)).add(",");
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