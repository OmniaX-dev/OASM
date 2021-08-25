#ifndef __VM_INTERFACE_HPP__
#define __VM_INTERFACE_HPP__

#include "IOManager.hpp"
#include "ProcessManager.hpp"

namespace Omnia
{
	namespace oasm
	{
		class VMLoader : public IOReciever
		{
			public:
				static inline VMLoader& instance(void) { return s_instance; }
				_int32 run(std::vector<_string>& args);

			private:
				static VMLoader s_instance;
		};

		inline VMLoader VMLoader::s_instance;
	}
}

#endif