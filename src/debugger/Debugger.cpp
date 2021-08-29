#include "Debugger.hpp"
#include <iostream>
#include <fstream>

namespace Omnia
{
	namespace oasm
	{
		int64 Debugger::run(int argc, char** argv)
		{
            OutputManager& out = *getOutputHandler();
            
            bool p__step_exec = false;
			bool p__print_memory = false;
			bool p__assemble = false;
			OmniaString p__input_file_path = "";
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
					else if (OmniaString(argv[i]).trim().equals("--assemble"))
					{
						p__assemble = true;
					}
				}
			}
            out.print("OASM DEBUGGER!!").newLine();
			return 0;
		}
	}
}