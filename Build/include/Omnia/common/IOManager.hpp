#ifndef __OUTPUT_HPP__
#define __OUTPUT_HPP__

#include "Utils.hpp"

namespace Omnia
{
    namespace common
    {
        class OutputManager
        {
            public:
                inline virtual ~OutputManager(void) = default;
                inline virtual OutputManager& print(OmniaString s) { return *this; }
                inline virtual OutputManager& print(int32 i) { return *this; }
                inline virtual OutputManager& print(int64 i) { return *this; }
                inline virtual OutputManager& newLine(void) { return *this; }
                inline virtual OutputManager& tab(void) { return *this; }
                inline virtual OutputManager& clear(void) { return *this; }
                
                inline virtual OutputManager& fcolor(uint8 __r, uint8 __g, uint8 __b) { return *this; }
                inline virtual OutputManager& bcolor(uint8 __r, uint8 __g, uint8 __b) { return *this; }

                inline virtual OutputManager& fc_grey(void) { return *this; }
                inline virtual OutputManager& fc_red(void) { return *this; }
                inline virtual OutputManager& fc_green(void) { return *this; }
                inline virtual OutputManager& fc_yellow(void) { return *this; }
                inline virtual OutputManager& fc_blue(void) { return *this; }
                inline virtual OutputManager& fc_magenta(void) { return *this; }
                inline virtual OutputManager& fc_cyan(void) { return *this; }
                inline virtual OutputManager& fc_white(void) { return *this; }
                inline virtual OutputManager& fc_brightGrey(void) { return *this; }
                inline virtual OutputManager& fc_brightRed(void) { return *this; }
                inline virtual OutputManager& fc_brightGreen(void) { return *this; }
                inline virtual OutputManager& fc_brightYellow(void) { return *this; }
                inline virtual OutputManager& fc_brightBlue(void) { return *this; }
                inline virtual OutputManager& fc_brightMagenta(void) { return *this; }
                inline virtual OutputManager& fc_brightCyan(void) { return *this; }
                inline virtual OutputManager& fc_brightWhite(void) { return *this; }

                inline virtual OutputManager& bc_grey(void) { return *this; }
                inline virtual OutputManager& bc_red(void) { return *this; }
                inline virtual OutputManager& bc_green(void) { return *this; }
                inline virtual OutputManager& bc_yellow(void) { return *this; }
                inline virtual OutputManager& bc_blue(void) { return *this; }
                inline virtual OutputManager& bc_magenta(void) { return *this; }
                inline virtual OutputManager& bc_cyan(void) { return *this; }
                inline virtual OutputManager& bc_white(void) { return *this; }
                inline virtual OutputManager& bc_brightGrey(void) { return *this; }
                inline virtual OutputManager& bc_brightRed(void) { return *this; }
                inline virtual OutputManager& bc_brightGreen(void) { return *this; }
                inline virtual OutputManager& bc_brightYellow(void) { return *this; }
                inline virtual OutputManager& bc_brightBlue(void) { return *this; }
                inline virtual OutputManager& bc_brightMagenta(void) { return *this; }
                inline virtual OutputManager& bc_brightCyan(void) { return *this; }
                inline virtual OutputManager& bc_brightWhite(void) { return *this; }

                inline virtual OutputManager& attr_italic(void) { return *this; }
                inline virtual OutputManager& attr_underline(void) { return *this; }
                inline virtual OutputManager& attr_blink(void) { return *this; }
                inline virtual OutputManager& attr_reverse(void) { return *this; }
                inline virtual OutputManager& attr_hide(void) { return *this; }
                inline virtual OutputManager& attr_cross(void) { return *this; }
                
                inline virtual OutputManager& tc_reset(void) { return *this; }

                inline virtual std::vector<OmniaString> flush(void) { return std::vector<OmniaString>(); }
        };

        class InputManager
        {
            public:
                inline virtual ~InputManager(void) = default;
                inline virtual InputManager& read(OmniaString& in) { return *this; } //TODO: make this always blocking
                inline virtual InputManager& waitForKeyPress(void) { return *this; } //TODO: Implement
        };

        class StandardConsoleOutput : public OutputManager
        {
            public:
                inline virtual OutputManager& print(OmniaString s) { std::cout << s.cpp(); return *this; }
                inline virtual OutputManager& print(int32 i) { std::cout << (int32)i; return *this; }
                inline virtual OutputManager& print(int64 i) { std::cout << (int64)i; return *this; }
                inline virtual OutputManager& newLine(void) { std::cout << "\n"; return *this; }
                inline virtual OutputManager& tab(void) { std::cout << "\t"; return *this; }
                inline virtual OutputManager& clear(void) { std::cout << "\x1B[2J\x1B[H"; return *this; } //TODO: Implement for windows aswell

                //inline virtual OutputManager& fcolor(uint8_t __r, uint8_t __g, uint8_t __b) { std::cout << termcolor::color<(const uint8_t)__r, (const uint8_t)__g, (const uint8_t)__b>; return *this; }
                //inline virtual OutputManager& bcolor(uint8_t __r, uint8_t __g, uint8_t __b) { std::cout << termcolor::on_color<(const uint8_t)__r, (const uint8_t)__g, (const uint8_t)__b>; return *this; }

                inline virtual OutputManager& fc_grey(void) { std::cout << termcolor::grey; return *this; }
                inline virtual OutputManager& fc_red(void) { std::cout << termcolor::red; return *this; }
                inline virtual OutputManager& fc_green(void) { std::cout << termcolor::green; return *this; }
                inline virtual OutputManager& fc_yellow(void) { std::cout << termcolor::yellow; return *this; }
                inline virtual OutputManager& fc_blue(void) { std::cout << termcolor::blue; return *this; }
                inline virtual OutputManager& fc_magenta(void) { std::cout << termcolor::magenta; return *this; }
                inline virtual OutputManager& fc_cyan(void) { std::cout << termcolor::cyan; return *this; }
                inline virtual OutputManager& fc_white(void) { std::cout << termcolor::white; return *this; }
                inline virtual OutputManager& fc_brightGrey(void) { std::cout << termcolor::bright_grey; return *this; }
                inline virtual OutputManager& fc_brightRed(void) { std::cout << termcolor::bright_red; return *this; }
                inline virtual OutputManager& fc_brightGreen(void) { std::cout << termcolor::bright_green; return *this; }
                inline virtual OutputManager& fc_brightYellow(void) { std::cout << termcolor::bright_yellow; return *this; }
                inline virtual OutputManager& fc_brightBlue(void) { std::cout << termcolor::bright_blue; return *this; }
                inline virtual OutputManager& fc_brightMagenta(void) { std::cout << termcolor::bright_magenta; return *this; }
                inline virtual OutputManager& fc_brightCyan(void) { std::cout << termcolor::bright_cyan; return *this; }
                inline virtual OutputManager& fc_brightWhite(void) { std::cout << termcolor::bright_white; return *this; }

                inline virtual OutputManager& bc_grey(void) { std::cout << termcolor::on_grey; return *this; }
                inline virtual OutputManager& bc_red(void) { std::cout << termcolor::on_red; return *this; }
                inline virtual OutputManager& bc_green(void) { std::cout << termcolor::on_green; return *this; }
                inline virtual OutputManager& bc_yellow(void) { std::cout << termcolor::on_yellow; return *this; }
                inline virtual OutputManager& bc_blue(void) { std::cout << termcolor::on_blue; return *this; }
                inline virtual OutputManager& bc_magenta(void) { std::cout << termcolor::on_magenta; return *this; }
                inline virtual OutputManager& bc_cyan(void) { std::cout << termcolor::on_cyan; return *this; }
                inline virtual OutputManager& bc_white(void) { std::cout << termcolor::on_white; return *this; }
                inline virtual OutputManager& bc_brightGrey(void) { std::cout << termcolor::on_bright_grey; return *this; }
                inline virtual OutputManager& bc_brightRed(void) { std::cout << termcolor::on_bright_red; return *this; }
                inline virtual OutputManager& bc_brightGreen(void) { std::cout << termcolor::on_bright_green; return *this; }
                inline virtual OutputManager& bc_brightYellow(void) { std::cout << termcolor::on_bright_yellow; return *this; }
                inline virtual OutputManager& bc_brightBlue(void) { std::cout << termcolor::on_bright_blue; return *this; }
                inline virtual OutputManager& bc_brightMagenta(void) { std::cout << termcolor::on_bright_magenta; return *this; }
                inline virtual OutputManager& bc_brightCyan(void) { std::cout << termcolor::on_bright_cyan; return *this; }
                inline virtual OutputManager& bc_brightWhite(void) { std::cout << termcolor::on_bright_white; return *this; }

                inline virtual OutputManager& attr_italic(void) { std::cout << termcolor::italic; return *this; }
                inline virtual OutputManager& attr_underline(void) { std::cout << termcolor::underline; return *this; }
                inline virtual OutputManager& attr_blink(void) { std::cout << termcolor::blink; return *this; }
                inline virtual OutputManager& attr_reverse(void) { std::cout << termcolor::reverse; return *this; }
                inline virtual OutputManager& attr_hide(void) { std::cout << termcolor::concealed; return *this; }
                inline virtual OutputManager& attr_cross(void) { std::cout << termcolor::crossed; return *this; }

                inline virtual OutputManager& tc_reset(void) { std::cout << termcolor::reset; return *this; }
        };

        class BufferedOutput : public OutputManager
        {
            public:
                inline virtual OutputManager& print(OmniaString s) { m_buffer.push_back(s); return *this; }
                inline virtual OutputManager& print(int32 i) { m_buffer.push_back(OmniaString().addInt(i)); return *this; }
                inline virtual OutputManager& newLine(void) { m_buffer.push_back(OmniaString("\n")); return *this; }
                inline virtual OutputManager& tab(void) { m_buffer.push_back(OmniaString("    ")); return *this; }
                inline std::vector<OmniaString> flush(void) { std::vector<OmniaString> tmp = m_buffer; m_buffer.clear(); return tmp;}
                inline virtual OutputManager& clear(void) { m_buffer.clear(); return *this; }
                
            
            private:
                std::vector<OmniaString> m_buffer;
        };

        class StandardConsoleInput : public InputManager
        {
            public:
                inline virtual InputManager& read(OmniaString& io)
                {
                    _string str;
                    std::getline(std::cin, str);
                    io = OmniaString(str);
                    return *this;
                }
        };

        class TextFileOutput : public OutputManager
        {
            public:
                inline TextFileOutput(void) { fileName = ""; }
                inline TextFileOutput(OmniaString fname) { fileName = fname; }
                inline void openOutputFile(void) { m_file.open(fileName.cpp()); }
                inline void closeOutputFile(void) { m_file.close(); }

                inline virtual OutputManager& print(OmniaString s) { m_file << s.cpp(); return *this; }
                inline virtual OutputManager& print(int32 i) { m_file << (int32)i; return *this; }
                inline virtual OutputManager& newLine(void) { m_file << "\n"; return *this; }
                inline virtual OutputManager& tab(void) { m_file << "    "; return *this; }

            public:
                OmniaString fileName;
            private:
                std::ofstream m_file;
        };

        class IOReciever
        {
            public:
                inline void setOutputHandler(OutputManager* out) { if (m_useDefaultOutput) delete m_out; m_out = out; m_useDefaultOutput = false; }
                inline OutputManager* getOutputHandler(void) { return m_out; }
                inline void setInputHandler(InputManager* in) { if (m_useDefaultInput) delete m_in; m_in = in; m_useDefaultInput = false; }
                inline InputManager* getInputHandler(void) { return m_in; }

            protected:
                inline IOReciever(void)
                {
                    m_out = new StandardConsoleOutput(); 
                    m_useDefaultOutput = true;
                    m_in = new StandardConsoleInput(); 
                    m_useDefaultInput = true;
                }

            protected:
                OutputManager* m_out;
                InputManager* m_in;
                bool m_useDefaultInput;
                bool m_useDefaultOutput;
        };
    }
}

#endif