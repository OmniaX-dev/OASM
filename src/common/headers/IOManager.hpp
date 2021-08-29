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

                inline virtual std::vector<OmniaString> flush(void) { return std::vector<OmniaString>(); }
        };

        class InputManager
        {
            public:
                inline virtual ~InputManager(void) = default;
                inline virtual InputManager& read(OmniaString& in) { return *this; } //TODO: make this always blocking
        };

        class StandardConsoleOutput : public OutputManager
        {
            public:
                inline virtual OutputManager& print(OmniaString s) { std::cout << s.cpp(); return *this; }
                inline virtual OutputManager& print(int32 i) { std::cout << (int32)i; return *this; }
                inline virtual OutputManager& print(int64 i) { std::cout << (int64)i; return *this; }
                inline virtual OutputManager& newLine(void) { std::cout << "\n"; return *this; }
                inline virtual OutputManager& tab(void) { std::cout << "    "; return *this; }
        };

        class BufferedOutput : public OutputManager
        {
            public:
                inline virtual OutputManager& print(OmniaString s) { m_buffer.push_back(s); return *this; }
                inline virtual OutputManager& print(int32 i) { m_buffer.push_back(OmniaString().addInt(i)); return *this; }
                inline virtual OutputManager& newLine(void) { m_buffer.push_back(OmniaString("\n")); return *this; }
                inline virtual OutputManager& tab(void) { m_buffer.push_back(OmniaString("    ")); return *this; }
                inline std::vector<OmniaString> flush(void) { std::vector<OmniaString> tmp = m_buffer; m_buffer.clear(); return tmp;}
            
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