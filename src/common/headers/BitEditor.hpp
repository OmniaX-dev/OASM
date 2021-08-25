#ifndef __BIT_EDITOR_HPP__
#define __BIT_EDITOR_HPP__

#include "Utils.hpp"

namespace Omnia
{
	namespace common
	{
		struct BitData
		{
			inline static std::map<eBit, uint16> bitVals = {
				{eBit::One, 1},
				{eBit::Two, 2},
				{eBit::Three, 3},
				{eBit::Four, 4},
				{eBit::Five, 5},
				{eBit::Six, 6},
				{eBit::Seven, 7},
				{eBit::Eight, 8},
				{eBit::Nine, 9},
				{eBit::Ten, 10},
				{eBit::Eleven, 11},
				{eBit::Twelve, 12},
				{eBit::Thirteen, 13},
				{eBit::Fourteen, 14},
				{eBit::Fifteen, 15},
				{eBit::Sixteen, 16},
				{eBit::Low, 17},
				{eBit::High, 18}
			};
			inline static const eBit bitLst[(uint32)eBit::Count] = {
				eBit::One,			eBit::Two,			eBit::Three,
				eBit::Four,			eBit::Five,			eBit::Six,
				eBit::Seven,		eBit::Eight,		eBit::Nine,
				eBit::Ten,			eBit::Eleven,		eBit::Twelve,
				eBit::Thirteen,		eBit::Fourteen,		eBit::Fifteen,
				eBit::Sixteen,		eBit::Low,			eBit::High
			};
		};
		
		class BitEditor
		{
			public:
				inline BitEditor(void) { m_data = 0; }
				inline BitEditor(word data) { m_data = data; }
				String binaryStr(bool prefix = true);
				
				inline word val(void) const { return m_data; }
				inline bool bit(eBit b) { return m_data & (word)b; }
				inline ubyte getMSB(void) { return (ubyte)((m_data >> 8) & 0x00FF); }
				inline ubyte getLSB(void) { return (ubyte)(m_data & 0x00FF); }

				inline word getMasked(word bitMask) { return m_data & bitMask; }
				word getSliced(eBit start, ubyte count);
				
				//Modifiers
				inline BitEditor& mask(word bitMask) { m_data = getMasked(bitMask); return *this; }
				inline BitEditor& slice(eBit start, ubyte count) { m_data = getSliced(start, count); return *this; }
				BitEditor& setBit(eBit b, bool value = true);
				BitEditor& setBitRange(eBit start, ubyte count, bool value = true);
				BitEditor& setMSB(ubyte value);
				BitEditor& setLSB(ubyte value);

				inline BitEditor operator++(int)
				{
					BitEditor __tmp = *this;
					m_data++;
					return __tmp;
				}
				inline BitEditor operator--(int)
				{
					BitEditor __tmp = *this;
					m_data--;
					return __tmp;
				}
				inline BitEditor& operator++(void)
				{
					m_data++;
					return *this;
				}
				inline BitEditor& operator--(void)
				{
					m_data--;
					return *this;
				}
				inline bool operator==(const BitEditor& op2)
				{
					return m_data == op2.m_data;
				}
				inline bool operator!=(const BitEditor& op2)
				{
					return m_data != op2.m_data;
				}
				inline bool operator==(const word& op2)
				{
					return m_data == op2;
				}
				inline bool operator!=(const word& op2)
				{
					return m_data != op2;
				}
				inline BitEditor operator+(const BitEditor& op2)
				{
					return BitEditor(m_data + op2.val());
				}
				inline BitEditor operator-(const BitEditor& op2)
				{
					return BitEditor(m_data - op2.val());
				}
				inline BitEditor operator+(const word& op2)
				{
					return BitEditor(m_data + op2);
				}
				inline BitEditor operator-(const word& op2)
				{
					return BitEditor(m_data - op2);
				}
				inline BitEditor operator*(const BitEditor& op2)
				{
					return BitEditor(m_data * op2.val());
				}
				inline BitEditor operator/(const BitEditor& op2)
				{
					return BitEditor(m_data / op2.val());
				}
				inline BitEditor operator*(const word& op2)
				{
					return BitEditor(m_data * op2);
				}
				inline BitEditor operator/(const word& op2)
				{
					return BitEditor(m_data / op2);
				}
				inline BitEditor operator%(const BitEditor& op2)
				{
					return BitEditor(m_data % op2.val());
				}
				inline BitEditor operator%(const word& op2)
				{
					return BitEditor(m_data % op2);
				}
				inline bool operator>(const BitEditor& op2)
				{
					return m_data > op2.m_data;
				}
				inline bool operator<(const BitEditor& op2)
				{
					return m_data < op2.m_data;
				}
				inline bool operator>(const word& op2)
				{
					return m_data > op2;
				}
				inline bool operator<(const word& op2)
				{
					return m_data < op2;
				}
				inline bool operator>=(const BitEditor& op2)
				{
					return m_data >= op2.m_data;
				}
				inline bool operator<=(const BitEditor& op2)
				{
					return m_data <= op2.m_data;
				}
				inline bool operator>=(const word& op2)
				{
					return m_data >= op2;
				}
				inline bool operator<=(const word& op2)
				{
					return m_data <= op2;
				}
				inline BitEditor& operator=(const BitEditor& be)
				{
					m_data = be.m_data;
					return *this;
				}
				inline BitEditor& operator=(const word& val)
				{
					m_data = val;
					return *this;
				}
				inline BitEditor& operator+=(const BitEditor& op2)
				{
					m_data += op2.m_data;
					return *this;
				}
				inline BitEditor& operator-=(const BitEditor& op2)
				{
					m_data -= op2.m_data;
					return *this;
				}
				inline BitEditor& operator+=(const word& op2)
				{
					m_data += op2;
					return *this;
				}
				inline BitEditor& operator-=(const word& op2)
				{
					m_data -= op2;
					return *this;
				}
				inline BitEditor& operator*=(const BitEditor& op2)
				{
					m_data *= op2.m_data;
					return *this;
				}
				inline BitEditor& operator/=(const BitEditor& op2)
				{
					m_data /= op2.m_data;
					return *this;
				}
				inline BitEditor& operator*=(const word& op2)
				{
					m_data *= op2;
					return *this;
				}
				inline BitEditor& operator/=(const word& op2)
				{
					m_data /= op2;
					return *this;
				}
				inline BitEditor& operator%=(const BitEditor& op2)
				{
					m_data %= op2.m_data;
					return *this;
				}
				inline BitEditor& operator%=(const word& op2)
				{
					m_data %= op2;
					return *this;
				}

                static TMemoryList stringToConstSream(String __str);
                static String constStreamToString(TMemoryList __stream);
				static eBit& nextBit(eBit& bit, bool cycle = false);
				friend std::ostream& operator<<(std::ostream& out, const BitEditor& be);
				
			private:
				word m_data;
		};

		inline std::ostream& operator<<(std::ostream& out, const BitEditor& be)
		{
			out << (word)be.val();
			return out;
		}
	}
}

#endif