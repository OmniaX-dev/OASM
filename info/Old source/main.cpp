#include "VMInterface.hpp"
#include "ProcessManager.hpp"

class TestWeditHandler : public Omnia::oasm::ExtComHandler
{
    public:
        inline virtual bool handleCommand(Omnia::oasm::_uint16 code, Omnia::String data, Omnia::oasm::OutputManager& out)
        {
            out.print(data);
            return true;
        }
};

int main(int argc, char** argv)
{
    TestWeditHandler twh;
    Omnia::oasm::ECM::instance().addHandler((Omnia::oasm::_uint16)Omnia::oasm::eComCodes::ExtCom1, twh);

    Omnia::oasm::OutputManager* m_out = Omnia::oasm::VMLoader::instance().getOutputHandler();
    std::vector<std::string> args;
    if (argc > 1) args.assign(argv + 1, argv + argc);
    else
    {
        m_out->print("No options specified.").newLine();
        return 3;
    }
    return Omnia::oasm::VMLoader::instance().run(args);
}
