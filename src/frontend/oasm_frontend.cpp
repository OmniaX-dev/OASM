#include <iostream>

#ifdef __COMPILE_AS__  //Assembler frontend
	#include "Assembler.hpp"
#elif defined(__COMPILE_VM__)  //Interpreter frontend
	#include "Interpreter.hpp"
#endif

#define __exit_verbose(__base_instance) \
	__base_instance.getOutputHandler()->newLine().print("Exit_Error ").print(__base_instance.getLastErrorCode()).newLine(); \
	return __base_instance.getLastErrorCode();

int main(int argc, char** argv)
{
#ifdef __COMPILE_AS__  //Assembler frontend
	Omnia::oasm::Assembler::instance().getOutputHandler()->newLine().newLine().print("oasm_as: version ")
	.print((long int)Omnia::eVersion::Major).print(".").print((long int)Omnia::eVersion::Minor)
	.print(".").print((long int)Omnia::eVersion::Build).newLine().newLine();
	Omnia::oasm::Assembler::instance().run(argc, argv);
	__exit_verbose(Omnia::oasm::Assembler::instance())
#elif defined(__COMPILE_VM__)  //Interpreter frontend
	Omnia::oasm::Interpreter::instance().getOutputHandler()->newLine().newLine().print("oasm_vm: version ")
	.print((long int)Omnia::eVersion::Major).print(".").print((long int)Omnia::eVersion::Minor)
	.print(".").print((long int)Omnia::eVersion::Build).newLine().newLine();
	Omnia::oasm::Interpreter::instance().run(argc, argv);
	__exit_verbose(Omnia::oasm::Interpreter::instance())
#endif
#include "Defines.hpp"
	return D__EXIT_ERR__NO_FRONTEND_SPECIFIED;
}
