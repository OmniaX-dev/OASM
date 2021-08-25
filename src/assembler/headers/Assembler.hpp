#ifndef __ASSEMBLER_HPP__
#define __ASSEMBLER_HPP__

#include "Common.hpp"
#include "IOManager.hpp"

namespace Omnia
{
	namespace oasm
	{
		class Assembler : public IOReciever, public ErrorReciever
		{
			public:
				inline static Assembler& instance(void) { return *Assembler::s_instance; }
				int32 run(int argc, char** argv);

			private:
				static Assembler* s_instance;
		};

		inline Assembler* Assembler::s_instance = new Assembler();
	}
}

#endif