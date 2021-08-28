#include "BitEditor.hpp"
#include <iostream>

namespace Omnia
{
	namespace common
	{
		String BitEditor::binaryStr(bool prefix)
		{
			String binStr = "";
			eBit _bit = eBit::One;
			uint8 i = 0;
			while (_bit != eBit::Low)
			{
				if (i++ % 8 == 0) binStr = binStr.add(" ");
				binStr = binStr.add((bit(_bit) ? "1" : "0"));
				BitEditor::nextBit(_bit); 
			}
			return binStr.add((prefix ? " b0" : "")).reverse();
		}

		word BitEditor::getSliced(eBit start, ubyte count)
		{
			word a = BitData::bitVals[start];
			word b = count + a;
			word mask = __gen_bit_mask(a, b);
			return (m_data & mask);
		}

		BitEditor& BitEditor::setBit(eBit b, bool value)
		{
			word mask = (word)b;
			m_data = (value ? mask | m_data : (mask ^ (word)eBitMasks::Invert) & m_data);
			return *this;
		}

		BitEditor& BitEditor::setBitRange(eBit start, ubyte count, bool value)
		{
			word a = BitData::bitVals[start];
			word b = count + a;
			word mask = __gen_bit_mask(a, b);
			m_data = (value ? mask | m_data : (mask ^ (word)eBitMasks::Invert) & m_data);
			return *this;
		}
		
		BitEditor& BitEditor::setMSB(ubyte value)
		{
			ubyte lsb = (m_data & (word)eBitMasks::LSB);
			m_data = (value << 8) | lsb;
			return *this;
		}
		
		BitEditor& BitEditor::setLSB(ubyte value)
		{
			ubyte msb = (m_data & (word)eBitMasks::MSB) >> 8;
			m_data = (msb << 8) | value;
			return *this;
		}
	
		eBit& BitEditor::nextBit(eBit& bit, bool cycle)
		{
			if (bit == eBit::Low)
			{
				if (cycle)  bit = eBit::One;
				return bit;
			}
			if (bit == eBit::High) return bit;
			bit = BitData::bitLst[BitData::bitVals[bit]];
			return bit;
		}

		TMemoryList BitEditor::stringToConstSream(String __str)
		{
			TMemoryList result;
			BitEditor cell = 0;
			char c = 0;
			bool __msb = true;
			for (word i = 0; i < __str.length(); i++)
			{
				c = __str[i];
				if (!__msb)
				{
					cell.setLSB((ubyte)c);
					result.push_back(cell);
					cell = 0;
				}
				else cell.setMSB((ubyte)c);
				__msb = !__msb;
			}
			result.push_back(cell);
			if (cell != 0 && cell.getLSB() != 0)
				result.push_back(0);
			return result;
		}

		String BitEditor::constStreamToString(TMemoryList __stream)
		{
			StringBuilder __sb;
			BitEditor __tmp_cell;
			char __char;
			for (word i = 0; i < __stream.size(); i++)
			{
				__tmp_cell = __stream[i];
				__char = (char)__tmp_cell.getMSB();
				if (__char == 0) break;
				else __sb.add(__char);
				__char = (char)__tmp_cell.getLSB();
				if (__char == 0) break;
				else __sb.add(__char);
			}

			return __sb.get();

			/*MemAddress __tmp_addr = __stream_addr;
			BitEditor __tmp_stream_cell;
			char __char = 0;
			while (true)
			{
				//TODO: Add Reserved MemCell for string length
				if (!read(__tmp_addr++, __tmp_stream_cell)) return false;
				__char = (char)__tmp_stream_cell.getMSB();
				if (__char == 0) return true;
				out = out.add(__char);
				__char = (char)__tmp_stream_cell.getLSB();
				if (__char == 0) return true;
				out = out.add(__char);
			}
			return "";*/
		}
	}
};