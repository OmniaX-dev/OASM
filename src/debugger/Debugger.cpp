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
            out.print("OASM DEBUGGER!!").newLine();
			return 0;
		}
	}
}