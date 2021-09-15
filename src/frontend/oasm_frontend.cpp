#include <iostream>

#ifdef __COMPILE_AS__  //Assembler frontend
	#include "Assembler.hpp"
#elif defined(__COMPILE_VM__)  //Interpreter frontend
	#include "Interpreter.hpp"
#elif defined(__COMPILE_DBG__)  //Debugger frontend
	#include "Debugger.hpp"
#endif

#define __exit_verbose(__base_instance, __err_code) \
	if (__err_code != D__NO_ERROR) \
		__base_instance.getOutputHandler()->newLine().fc_red().print("*** Exit Error: ").print(Omnia::common::Utils::intToHexStr(__err_code)).newLine().newLine(); \
	Omnia::common::Utils::hideCursor(false); \
	__base_instance.getOutputHandler()->tc_reset(); \
	return __err_code;

int main(int argc, char** argv)
{

#ifdef __COMPILE_AS__   //Assembler frontend
	Omnia::common::Utils::init();

	Omnia::common::StringBuilder __ver("oasm_as: version ");
	__ver.add((long int)Omnia::eVersion::Major).add(".").add((long int)Omnia::eVersion::Minor)
	.add(".").add((long int)Omnia::eVersion::Build);

	Omnia::oasm::Assembler::instance().getOutputHandler()->newLine();
	Omnia::common::Utils::message(__ver.get(), *Omnia::oasm::Assembler::instance().getOutputHandler(), Omnia::eMsgType::Version);
	Omnia::oasm::Assembler::instance().getOutputHandler()->newLine();

	Omnia::common::ErrorCode __err = Omnia::oasm::Assembler::instance().run(argc, argv);
	Omnia::oasm::Assembler::instance().getOutputHandler()->tc_reset();
	Omnia::common::Utils::hideCursor(false);
	
	return __err;

#elif defined(__COMPILE_VM__)  //Interpreter frontend
	Omnia::common::Utils::init();

	Omnia::common::StringBuilder __ver("oasm_as: version ");
	__ver.add((long int)Omnia::eVersion::Major).add(".").add((long int)Omnia::eVersion::Minor)
	.add(".").add((long int)Omnia::eVersion::Build);

	Omnia::oasm::Interpreter::instance().getOutputHandler()->newLine();
	Omnia::common::Utils::message(__ver.get(), *Omnia::oasm::Interpreter::instance().getOutputHandler(), Omnia::eMsgType::Version);
	Omnia::oasm::Interpreter::instance().getOutputHandler()->newLine();

	Omnia::common::ErrorCode __err = Omnia::oasm::Interpreter::instance().run(argc, argv);

	__exit_verbose(Omnia::oasm::Interpreter::instance(), __err)

#elif defined(__COMPILE_DBG__)  //Debugger frontend
	Omnia::common::Utils::init();

	Omnia::common::ErrorCode __err = Omnia::oasm::Debugger::instance().run(argc, argv);
	Omnia::oasm::Debugger::instance().getOutputHandler()->tc_reset();
	Omnia::common::Utils::hideCursor(false);
	return __err;

#endif
#include "Defines.hpp"
	return D__EXIT_ERR__NO_FRONTEND_SPECIFIED;
}
