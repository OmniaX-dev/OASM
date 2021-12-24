#include "Keyboard.hpp"
#include <unistd.h>
    
namespace Omnia
{
	namespace common
	{
		Keyboard::Keyboard(void)
		{
#ifndef _WIN32
			tcgetattr(0, &initial_settings);
			new_settings = initial_settings;
			new_settings.c_lflag &= ~ICANON;
			new_settings.c_lflag &= ~ECHO;
			new_settings.c_lflag &= ~ISIG;
			new_settings.c_cc[VMIN] = 1;
			new_settings.c_cc[VTIME] = 0;
			tcsetattr(0, TCSANOW, &new_settings);
			peek_character = -1;
#endif
			
			m_cmd = "";
			m_output_enabled = true;
			m_cmd_buffer_enabled = true;
		}
			
		Keyboard::~Keyboard(void)
		{
#ifndef _WIN32
			tcsetattr(0, TCSANOW, &initial_settings);
#endif
		}

#ifndef _WIN32			
		int Keyboard::kbhit(void)
		{
			unsigned char ch;
			int nread;
			if (peek_character != -1) return 1;
			new_settings.c_cc[VMIN] = 0;
			tcsetattr(0, TCSANOW, &new_settings);
			nread = read(0, &ch, 1);
			new_settings.c_cc[VMIN] = 1;
			tcsetattr(0, TCSANOW, &new_settings);

			if (nread == 1)
			{
				peek_character = ch;
				return 1;
			}
			return 0;
		}
			
		int Keyboard::getch(void)
		{
			char ch;

			if (peek_character != -1)
			{
				ch = peek_character;
				peek_character = -1;
			}
			else
				read(0, &ch, 1);
			return ch;
		}
#endif
		
		std::string Keyboard::getKeyBuffer(void)
		{
			std::string __str = "";
			for (auto& key : m_key_buff)
			{
				switch (key)
				{
					case eKeys::Backspace:
					case eKeys::Enter:
					case eKeys::Up:
					case eKeys::Down:
					case eKeys::Left:
					case eKeys::Right:
					case eKeys::Escape:
					case eKeys::NoKeyPressed:
						break;
					case eKeys::Tab:
						__str += '\t';
						break;
					default:
						__str += (char)(key);
						break;
				}
			}
			return __str;
		}

		std::string Keyboard::flushKeyBuffer(void)
		{
			std::string __str = getKeyBuffer();
			m_key_buff.clear();
			return __str;
		}
		
		eKeys Keyboard::waitForKeyPress(void) //TODO: Add sleep to this method
		{
			eKeys key = eKeys::NoKeyPressed;
			while (key == eKeys::NoKeyPressed)
				key = getPressedKey();
			return key;
		}

		eKeys Keyboard::getPressedKey(void)
		{
			int k1 = 0, k2 = 0, k3 = 0;
			eKeys key;
			if(kbhit())
			{
				k1 = getch();
				
				if (k1 == 127)
				{
					key = eKeys::Backspace;
					if (isOutputEnabled())
						std::cout << "\b \b" << std::flush;
					if (m_key_buff.size() > 0 && isCommandBufferEnabled())
						m_key_buff.pop_back();
				}
				else if (k1 == 10)
				{
					key = eKeys::Enter;
					if (isOutputEnabled())
						std::cout << "\n" << std::flush;
					if (isCommandBufferEnabled())	
						m_cmd = flushKeyBuffer();
				}
				else if (k1 == 9)
				{
					if (isOutputEnabled())
						std::cout << "\t" << std::flush;	
					if (isCommandBufferEnabled())
						m_key_buff.push_back(key);
					key = eKeys::Tab;
				}
				else if (k1 >= ' ' && k1 <= '~')
				{
					key = (eKeys)(k1);
					if (isCommandBufferEnabled())
						m_key_buff.push_back(key);
					if (isOutputEnabled())
						std::cout << (char)(key) << std::flush;
				}
				else if (k1 == 27)
				{
					if (!kbhit())
					{
						key = eKeys::Escape;
						return key;    	
					}
					k2 = getch();
					if (k2 == 91)
					{
						k3 = getch();
						if (k3 == 65) key = eKeys::Up;
						else if (k3 == 66) key = eKeys::Down;
						else if (k3 == 67) key = eKeys::Right;
						else if (k3 == 68) key = eKeys::Left;
					}
				}
			}
			else key = eKeys::NoKeyPressed;
			return key;
		}
	}
}    

