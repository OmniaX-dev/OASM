#include <iostream>

#ifdef __COMPILE_AS__  //Assembler frontend
	#include "Assembler.hpp"
#elif defined(__COMPILE_VM__)  //Interpreter frontend
	#include "Interpreter.hpp"
#elif defined(__COMPILE_DBG__)  //Debugger frontend
	#include "Debugger.hpp"
#endif

#define __exit_verbose(__base_instance, __err_code) \
	__base_instance.getOutputHandler()->newLine().newLine().newLine().print(" [system]:/ Exit_Error ").print(Omnia::common::Utils::intToHexStr(__err_code)).newLine(); \
	return __err_code;

int main(int argc, char** argv)
{
#ifdef __COMPILE_AS__  //Assembler frontend

	Omnia::oasm::Assembler::instance().getOutputHandler()->newLine().newLine().print("[system]:/ oasm_as: version ")
	.print((long int)Omnia::eVersion::Major).print(".").print((long int)Omnia::eVersion::Minor)
	.print(".").print((long int)Omnia::eVersion::Build).newLine().newLine();

	Omnia::common::ErrorCode __err = Omnia::oasm::Assembler::instance().run(argc, argv);

	__exit_verbose(Omnia::oasm::Assembler::instance(), __err)

#elif defined(__COMPILE_VM__)  //Interpreter frontend

	Omnia::oasm::Interpreter::instance().getOutputHandler()->newLine().newLine().print("[system]:/ oasm_vm: version ")
	.print((long int)Omnia::eVersion::Major).print(".").print((long int)Omnia::eVersion::Minor)
	.print(".").print((long int)Omnia::eVersion::Build).newLine().newLine();

	Omnia::common::ErrorCode __err = Omnia::oasm::Interpreter::instance().run(argc, argv);

	__exit_verbose(Omnia::oasm::Interpreter::instance(), __err)

#elif defined(__COMPILE_DBG__)  //Debugger frontend

	Omnia::oasm::Debugger::instance().getOutputHandler()->newLine().newLine().print("[system]:/ oasm_vm: version ")
	.print((long int)Omnia::eVersion::Major).print(".").print((long int)Omnia::eVersion::Minor)
	.print(".").print((long int)Omnia::eVersion::Build).newLine().newLine();

	Omnia::common::ErrorCode __err = Omnia::oasm::Debugger::instance().run(argc, argv);

	__exit_verbose(Omnia::oasm::Debugger::instance(), __err)

#endif
#include "Defines.hpp"
	return D__EXIT_ERR__NO_FRONTEND_SPECIFIED;
}
