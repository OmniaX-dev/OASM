#ifndef __KEYBOARD_HPP__
#define __KEYBOARD_HPP__
   
#ifndef _WIN32
	#include <termios.h>
#else
	#include <conio.h>
#endif
#include <vector>
#include <iostream>

namespace Omnia
{
	namespace common
	{
		enum class eKeys
		{
			NoKeyPressed = 0,
			Backspace = 127,
			Enter = 10,
			Tab = 9,
			Up = 0x5B41,
			Down = 0x5B42,
			Left = 0x5B44,
			Right = 0x5B43,
			Escape = 27,
			Space = ' ',
			symExclamation = '!',
			symDoubleQuote = '\"',
			symPound = '#',
			symDollar = '$',
			symPercent = '%',
			symAmpersand = '&',
			symSingeQuote = '\'',
			symLeftParenthesis = '(',
			symRightParenthesis = ')',
			symAsterisk = '*',
			symPlus = '+',
			symComma = ',',
			symMinus = '-',
			symPeriod = '.',
			symSlash = '/',
			Num0 = '0', Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
			symColon = ':',
			symSemicolon = ';',
			symLessThan = '<',
			symEquals = '=',
			symGreaterThan = '>',
			symQuestionMark = '?',
			symAtSign = '@',
			A = 'A',B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,
			symLeftSquareBracket = '[',
			symBackslash = '\\',
			symRightSquareBracket = ']',
			symCaret = '^',
			symUnderscore = '_',
			symGraveAccent = 96,
			a = 'a',b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,
			symLeftCurlyBracket = '{',
			symVerticalBar = '|',
			symRightCurlyBracket = '}',
			symTilde = '~'
		};

		class Keyboard
		{
			public:
				Keyboard(void);
				~Keyboard(void);
				
				eKeys getPressedKey(void);
				eKeys waitForKeyPress(void);
				
				inline std::string getInputString(void) { return m_cmd; }
				
				inline bool isOutputEnabled(void) { return m_output_enabled; }
				inline void enableOutput(bool __oe = true) { m_output_enabled = __oe; }
				inline void disableOutput(void) { enableOutput(false); }
				
				inline bool isCommandBufferEnabled(void) { return m_cmd_buffer_enabled; }
				inline void enableCommandBuffer(bool __cbe = true) { m_cmd_buffer_enabled = __cbe; }
				inline void disableCommandBuffer(void) { enableCommandBuffer(false); }
				
			private:
				std::string getKeyBuffer(void);
				std::string flushKeyBuffer(void);
				
#ifndef _WIN32
				int kbhit(void);
				int getch(void);
#endif
				

			private:
#ifndef _WIN32
				struct termios initial_settings, new_settings;
				int peek_character;
#endif
				std::vector<eKeys> m_key_buff;
				std::string m_cmd;
				bool m_output_enabled;
				bool m_cmd_buffer_enabled;
		};
	}
}
#endif
