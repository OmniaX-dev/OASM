#include "Assembler.hpp"
#include <iostream>

namespace Omnia
{
	namespace oasm
	{
		int64 Assembler::run(int argc, char** argv)
		{
			OutputManager& out = *getOutputHandler();
			for (uint16 i = 0; i < argc; i++)
				out.print(String(argv[i])).print(" ");
			out.print("- ASSEMBLER").newLine();
			return 0;
		}
	}
}