#include "ProcessManager.hpp"
#include <fstream>

namespace Omnia
{
    namespace oasm
    {
        void Process::kill(void)
        {
            m_startAddr = oasm_nullptr;
            m_dataSize = 0;
            m_codeSize = 0;
            m_heapSize = 0;
            m_stackSize = 0;
            m_heapStartAddr = oasm_nullptr;
            m_stackStartAddr = oasm_nullptr;
            m_valid = false;
            m_args.clear();
        }

        bool Process::isValid(void)
        {
            return  m_valid && m_handle != 0 &&
                    m_startAddr != oasm_nullptr &&
                    m_dataSize != 0 &&
                    m_codeSize != 0 &&
                    m_heapSize != 0 &&
                    m_stackSize != 0 &&
                    m_heapStartAddr != oasm_nullptr &&
                    m_stackStartAddr != oasm_nullptr;
        }

        bool Process::freeMemory(std::vector<MemCellState>& memMap)
        {
            //TODO: Free stack
            if (!isValid()) return false;
            for (MemAddress addr = m_startAddr; addr < m_startAddr + m_codeSize + m_dataSize; addr++)
                memMap[addr].release();
            for (MemAddress addr = m_heapStartAddr - 1; addr < m_heapStartAddr + m_heapSize; addr++)
                memMap[addr].release();
            return true;
        }

        MemCellState::MemCellState(eMemCellType type, eMemState state)
        {
            m_state = state;
            m_type = type;
            m_process = nullptr;
            m_used = false;
            m_readOnly = false;
            m_protected = false;
        }

        bool MemCellState::use(Process& proc)
        {
            if (!testFor(proc)) return false;
            m_used = true;
            return true;
        }

        bool MemCellState::testFor(Process& proc)
        {
            if (m_process == nullptr) return false;
            bool res = m_state == eMemState::Allocated;
            res &= proc.getHandle() == m_process->getHandle();
            res &= !isRegister();
            return res;
        }

        bool MemCellState::free(Process& proc)
        {
            if (!testFor(proc)) return false;
            m_used = false;
            return true;
        }

        bool MemCellState::release(void)
        {
            if (isRegister() || isReserved()) return false;
            m_state = eMemState::Free;
            m_process = nullptr;
            m_used = false;
            m_readOnly = false;
            m_protected = false;
            return true;
        }

        bool MemCellState::set(eMemState state, Process* proc)
        {
            if (isRegister()) return false;
            if (isUsed() && state != eMemState::Free) return false;
            if (isUsed())
            {
                m_state = eMemState::Free;
                m_process = nullptr;
                m_used = false;
                return true;
            }
            if (state == eMemState::Allocated && proc == nullptr) return false;
            if (state != eMemState::Allocated)
            {
                m_state = state;
                m_process = nullptr;
                return true;
            }
            m_state = eMemState::Allocated;
            m_process = proc;
            return true;
        }

        void ProcessManager::allocate(_uint32 memSize)
        {
            m_memory.clear();
            m_memMap.clear();
            m_processes.clear();
            m_memory.resize(memSize + AUTO__OASM_HEAP_SIZE + AUTO__OASM_STACK_SIZE, 0);
            m_memMap.resize(memSize + AUTO__OASM_HEAP_SIZE + AUTO__OASM_STACK_SIZE, MemCellState());
            m_memSize = memSize;
            m_memStart = (_uint32)eRegisters::MemStart;
            m_ipu = AUTO__OASM_INST_PER_TICK;
            m_running = false;
            m_runningProc = nullptr;
            for (_uint16 i = 0; i < (_uint16)eRegisters::MemStart; i++)
                m_memMap[i] = MemCellState(eMemCellType::Register, eMemState::Reserved);
            for (_uint16 i = m_memStart; i < memSize; i++)
                m_memMap[i] = MemCellState(eMemCellType::Normal);
            for (_uint16 i = memSize; i < m_memSize + AUTO__OASM_HEAP_SIZE; i++)
                m_memMap[i] = MemCellState(eMemCellType::Heap);
            for (_uint16 i = memSize + AUTO__OASM_HEAP_SIZE; i < m_memSize + AUTO__OASM_HEAP_SIZE + AUTO__OASM_STACK_SIZE; i++)
                m_memMap[i] = MemCellState(eMemCellType::Stack);
            
            m_memMap[(MemAddress)eRegisters::NP].setReadOnly(true);
            m_memMap[(MemAddress)eRegisters::NP].setProtected(false);
            m_memMap[(MemAddress)eRegisters::CF].setReadOnly(true);
            m_memMap[(MemAddress)eRegisters::CF].setProtected(false);
            m_memMap[(MemAddress)eRegisters::IP].setProtected(true);
            m_memMap[(MemAddress)eRegisters::DR].setReadOnly(true);
            m_memMap[(MemAddress)eRegisters::DR].setProtected(false);
            m_memMap[(MemAddress)eRegisters::ES].setReadOnly(true);
            m_memMap[(MemAddress)eRegisters::ES].setProtected(false);
            m_memMap[(MemAddress)eRegisters::FL].setReadOnly(true);
            m_memMap[(MemAddress)eRegisters::FL].setProtected(false);

            set((MemAddress)eRegisters::SP, (_int32)getStackStart(), false);
            m_savedFlg = 0;
        }

        void ProcessManager::printMemoryBlock(_uint32 start, _uint32 end, String text)
        {
            if (end == 0) return;
            if (text.trim() != "")
                m_out->newLine().print(text);
            m_out->print("  -  startAddr=").print(start).print("  endAddr=").print(end - 1);
            m_out->newLine().print("------------------------------------------------------------------").newLine();
            m_out->print(start).print("\t");
            for (_uint32 i = start; i < end; i++)
            {
                m_out->print(Utils::intToHexStr(m_memory[i], false)).print("  ");
                if (((i - start) + 1) % 9 == 0)
                    m_out->newLine().print(i + 1).print("\t");
            }
            m_out->newLine().print("------------------------------------------------------------------");
            m_out->newLine().print(end - start).print(" memory cells").newLine().newLine();
        }

        void ProcessManager::error(eRuntimeErrors err, String msg, MemAddress addr, Process& proc, bool forceQuit)
        {
            
            m_out->print("\nRuntime Error ").print((_uint32)err).print(":").newLine().print(msg).newLine();
            m_out->print("Address: ").print(addr).newLine();
            m_out->print("Process: ").print(proc.getHandle()).newLine();
            if (forceQuit)
            {
                m_out->print("Killing process with force.").newLine();
                killRunningProc();
            }
        }

        MemAddress ProcessManager::requestMemory(_uint16 size, Process& proc)
        {
            MemAddress addr = oasm_nullptr;
            for (_uint32 i = m_memStart; i < m_memSize; i++)
            {
                if (m_memMap[i].isFree())
                {
                    bool found = true;
                    for (_uint32 j = 1; j < size; j++)
                    {
                        if (i + j >= m_memSize) return oasm_nullptr;
                        if (!m_memMap[i + j].isFree())
                        {
                            i = i + j + 1;
                            found = false;
                            break;
                        }
                    }
                    if (!found) continue;
                    for (_uint32 j = 0; j < size; j++)
                    {
                        if (!m_memMap[i + j].set(eMemState::Allocated, &proc))
                        {
                            error(eRuntimeErrors::RequestMemSetFail, "Failed to request memory.", i + j, proc);
                            return oasm_nullptr;
                        }
                    }
                    addr = i;
                    break;
                }
            }
            return addr;
        }

        MemAddress ProcessManager::requestHeap(_uint16 size, Process& proc)
        {
            MemAddress addr = oasm_nullptr;
            size += 1;
            for (_uint32 i = m_memSize; i < m_memSize + AUTO__OASM_HEAP_SIZE; i++)
            {
                if (m_memMap[i].isFree() && m_memMap[i].isHeap())
                {
                    bool found = true;
                    for (_uint32 j = 1; j < size; j++)
                    {
                        if (i + j >= m_memSize + AUTO__OASM_HEAP_SIZE) return oasm_nullptr;
                        if (!m_memMap[i + j].isFree())
                        {
                            i = i + j + 1;
                            found = false;
                            break;
                        }
                    }
                    if (!found) continue;
                    for (_uint32 j = 0; j < size; j++)
                        m_memMap[i + j].set(eMemState::Allocated, &proc);
                    addr = i;
                    break;
                }
            }
            m_memory[addr] = addr + size - 1;
            return addr + 1;
        }

        MemAddress ProcessManager::requestStack(_uint16 size, Process& proc)
        {
            MemAddress addr = oasm_nullptr;
            size += 1;
            for (_uint32 i = getStackStart(); i < getFullMemorySize(); i++)
            {
                if (m_memMap[i].isFree() && m_memMap[i].isStack())
                {
                    bool found = true;
                    for (_uint32 j = 1; j < size; j++)
                    {
                        if (i + j >= getFullMemorySize()) return oasm_nullptr;
                        if (!m_memMap[i + j].isFree())
                        {
                            i = i + j + 1;
                            found = false;
                            break;
                        }
                    }
                    if (!found) continue;
                    for (_uint32 j = 0; j < size; j++)
                        m_memMap[i + j].set(eMemState::Allocated, &proc);
                    addr = i;
                    break;
                }
            }
            m_memory[addr] = addr + size - 1;
            return addr + 1;
        }

        MemAddress ProcessManager::heapAlloc(_uint16 size, Process& proc)
        {
            MemAddress addr = oasm_nullptr;
            size += 1;
            if (size >= proc.m_heapSize) return addr;
            for (_uint32 i = proc.m_heapStartAddr; i < proc.m_heapStartAddr + proc.m_heapSize; i++)
            {
                if (!m_memMap[i].isUsed() && m_memMap[i].testFor(proc) && m_memMap[i].isHeap())
                {
                    bool found = true;
                    for (_uint32 j = 1; j < size; j++)
                    {
                        if (i + j >= m_memSize + AUTO__OASM_HEAP_SIZE) return oasm_nullptr;
                        if (m_memMap[i].isUsed() || !m_memMap[i + j].testFor(proc) || !m_memMap[i].isHeap())
                        {
                            i = i + j + 1;
                            found = false;
                            break;
                        }
                    }
                    if (!found) continue;
                    for (_uint32 j = 0; j < size; j++)
                        m_memMap[i + j].use(proc);
                    addr = i;
                    break;
                }
            }
            m_memory[addr] = addr + size - 1;
            return addr + 1;
        }

        bool ProcessManager::copyToMemory(Process& proc)
        {
            if (proc.m_startAddr + proc.m_codeSize >= m_memSize)
            {
                error(eRuntimeErrors::CopyProcOutOfBounds, "Not enough memory to load process.", 0, proc);
                return false;
            }
            std::vector<_int32> bkp;
            std::vector<MemCellState> bkpm;
            for (MemAddress i = proc.m_startAddr; i < proc.m_startAddr + proc.m_codeSize; i++)
            {
                bkp.push_back(m_memory[i]);
                bkpm.push_back(m_memMap[i]);
            }

            for (MemAddress i = proc.m_startAddr; i < proc.m_startAddr + proc.m_codeSize; i++)
            {
                if (!set(i, proc.code[i - proc.m_startAddr]))
                {
                    error(eRuntimeErrors::RequestMemSetFail, "Failed to load process.", i, proc);
                    _uint16 k = 0;
                    for (MemAddress j = proc.m_startAddr; j < i; j++, k++)
                    {
                        m_memory[j] = bkp[k];
                        m_memMap[j] = bkpm[k];
                    }
                    return false;
                }
                m_memMap[i].set(eMemState::Allocated, &proc);
            }
            return true;
        }

        bool ProcessManager::checkAddress(MemAddress addr)
        {
            if (!m_running || m_runningProc == nullptr) return false;
            Process& proc = *m_runningProc;
            if (!m_memMap[addr].testFor(proc)) return false;
            if (addr < proc.m_startAddr + proc.m_codeSize) return false;
            if (addr >= proc.m_startAddr + proc.m_codeSize + proc.m_dataSize) return false;
            return true;
        }

        void ProcessManager::nextProcess(void)
        {
            if (m_running) return;
            if (m_processes.size() == 0)
            {
                m_runningProc = nullptr;
                m_running = false;
                return;
            }
            runProcess(*m_processes[0]);
            m_processes.erase(m_processes.begin());
        }
        
        bool ProcessManager::runFromFile(String fileName, std::vector<_int32> args)
        {
            std::ifstream rf(fileName.cpp(), std::ios::out | std::ios::binary);
            if(!rf) return false; //TODO: Error
            std::vector<_int32> code;

            _int32 cell = 0;
            while(rf.read((char*)&cell, sizeof(cell)))
                code.push_back(cell);

            if (code.size() == 0) return false; //TODO: Error
            if (code[0] != code.size()) return false; //TODO: Error
            
            static Process proc;
            proc.code = code;
            return runProcess(proc, args);
        }

        bool ProcessManager::runProcess(Process& proc, std::vector<_int32> args)
        {
            if (m_running)
            {
                m_processes.push_back(&proc);
                return false;
            }
            m_runningProc = &proc;
            proc.m_codeSize = proc.code[0];

            if (proc.code[1] != (_int32)eInstructionSet::req_local) return false;
            proc.m_dataSize = proc.code[2];
            proc.m_startAddr = requestMemory(proc.m_codeSize + proc.m_dataSize, proc);
            if (proc.m_startAddr == oasm_nullptr) return false;

            if (proc.code[3] != (_int32)eInstructionSet::req_heap) return false;
            proc.m_heapSize = proc.code[4];
            proc.m_heapStartAddr = requestHeap(proc.m_heapSize, proc);
            if (proc.m_heapStartAddr == oasm_nullptr) return false;

            if (proc.code[5] != (_int32)eInstructionSet::req_stack) return false;
            proc.m_stackSize = proc.code[6];
            proc.m_stackStartAddr = requestStack(proc.m_stackSize, proc);
            if (proc.m_stackStartAddr == oasm_nullptr) return false;

            proc.m_handle = newProcHandle();
            proc.m_args = args;
            proc.m_valid = true;
            m_running = true;
            copyToMemory(proc);
            for (MemAddress addr = proc.m_startAddr; addr < proc.m_startAddr + proc.m_codeSize; addr++)
                m_memMap[addr].setProtected(true);
            _int32 ep_addr = proc.code[7];
            if (ep_addr == oasm_nullptr)
            {
                error(eRuntimeErrors::EntryPointMissing, "Entry point missing.", 0, proc);
                return false;
            }
            if (!m_memMap[offsetCode(proc, ep_addr)].testFor(proc))
            {
                error(eRuntimeErrors::EntryPointInvalid, "Invalid entry point address.", offsetCode(proc, ep_addr), proc);
                return false;
            }
            proc.m_debug.dbginf = proc.code[8];
            set((MemAddress)eRegisters::IP, offsetCode(proc, ep_addr), false);
            return true;
        }

        void ProcessManager::assignmentConversion(Process& proc, _int32 flg, _uint32 v1, _int32 v2, MemAddress& dest, _int32& val, eByte byte, bool skipDest, _int32 _off, _byte offsetType)
        {
            _int16 off1 = Utils::gets(_off, true);
            _int16 off2 = Utils::gets(_off, false);
            _int32 fl = mem((MemAddress)eRegisters::FL, true, eByte::One);
            if (flg == (_int32)eFlags::NormalAssignment)
            {
                if (!skipDest)
                {
                    if (v1 < (MemAddress)eRegisters::MemStart || v1 >= m_memSize)
                        dest = v1;
                    else
                        dest = offset(proc, v1);
                    if (fl == (_int32)eFlags::DereferenceDestination)
                        dest = mem(dest);
                    if (offsetType == (_int32)eFlags::ConstantOffset)
                        dest += off1;
                    else if (offsetType == (_int32)eFlags::VariableOffset)
                        dest += mem(off1);
                }
                val = v2;
                return;
            }
            if (flg == (_int32)eFlags::IndirectAssignment)
            {
                if (!skipDest)
                {
                    if (v1 < (MemAddress)eRegisters::MemStart || v1 >= m_memSize)
                        dest = v1;
                    else
                        dest = offset(proc, v1);
                    if (fl == (_int32)eFlags::DereferenceDestination)
                        dest = mem(dest);
                    if (offsetType == (_int32)eFlags::ConstantOffset)
                        dest += off1;
                    else if (offsetType == (_int32)eFlags::VariableOffset)
                        dest += mem(off1);
                }
                if (v2 < (MemAddress)eRegisters::MemStart || v2 >= m_memSize)
                    val = mem(v2, true, byte);
                else
                    val = mem(offset(proc, v2), true, byte);
                if (offsetType == (_int32)eFlags::ConstantOffset)
                    dest += off2;
                else if (offsetType == (_int32)eFlags::VariableOffset)
                    dest += mem(off2);
                return;
            }
            if (flg == (_int32)eFlags::DereferenceAssignment)
            {
                if (!skipDest)
                {
                    if (v1 < (MemAddress)eRegisters::MemStart || v1 >= m_memSize)
                        dest = v1;
                    else
                        dest = offset(proc, v1);
                    if (fl == (_int32)eFlags::DereferenceDestination)
                        dest = mem(dest);
                    if (offsetType == (_int32)eFlags::ConstantOffset)
                        dest += off1;
                    else if (offsetType == (_int32)eFlags::VariableOffset)
                        dest += mem(off1);
                }
                if (offsetType == (_int32)eFlags::ConstantOffset)
                    dest += off2;
                else if (offsetType == (_int32)eFlags::VariableOffset)
                    dest += mem(off2);
                return;
            }
            if (flg == (_int32)eFlags::ReferenceAssign)
            {
                if (!skipDest)
                {
                    if (v1 < (MemAddress)eRegisters::MemStart || v1 >= m_memSize)
                        dest = v1;
                    else
                        dest = offset(proc, v1);
                    if (fl == (_int32)eFlags::DereferenceDestination)
                        dest = mem(dest);
                    if (offsetType == (_int32)eFlags::ConstantOffset)
                        dest += off1;
                    else if (offsetType == (_int32)eFlags::VariableOffset)
                        dest += mem(off1);
                }
                if (v2 < (MemAddress)eRegisters::MemStart || v2 >= m_memSize)
                    val = mem(mem(v2), true, byte);
                else
                    val = mem(mem(offset(proc, v2)), true, byte);
                if (offsetType == (_int32)eFlags::ConstantOffset)
                    dest += off2;
                else if (offsetType == (_int32)eFlags::VariableOffset)
                    dest += mem(off2);
                return;
            }
            dest = oasm_nullptr;
            val = 0;
            return;
        }

        bool ProcessManager::tick(void)
        {
            nextProcess();
            if (!m_running || m_runningProc == nullptr || !m_runningProc->isValid()) return false;
            Process& proc = *m_runningProc;
            for (_uint16 i = 0; i < m_ipu; i++)
            {
                if (!m_running || m_runningProc == nullptr || !m_runningProc->isValid()) return false;
                _int32 ip = mem((MemAddress)eRegisters::IP);
                _int32 inst = mem(ip++, false);
                _int32 flags = mem(ip++, false);
                m_lastInst.clear();
                m_lastInst.push_back(inst);
                m_lastInst.push_back(flags);
                _byte flg1 = Utils::getb(flags, eByte::One);
                _byte flg2 = Utils::getb(flags, eByte::Two);
                _byte flg3 = Utils::getb(flags, eByte::Three);
                _byte flg4 = Utils::getb(flags, eByte::Four);
                switch ((eInstructionSet)inst)
                {
                    case eInstructionSet::add: //DONE
                    {
                        MemAddress dest = 0;
                        _int32 val = 0;
                        _int32 v1 =  mem(ip++, false);
                        _int32 v2 =  mem(ip++, false);
                        m_lastInst.push_back(v1);
                        m_lastInst.push_back(v2);
                        assignmentConversion(proc, flg1, v1, v2, dest, val, (eByte)flg3);
                        if (dest == oasm_nullptr)
                        {
                            error(eRuntimeErrors::AssignFault, "Assign Failure: <add> destination cannot be null.", ip - 2, proc);
                            return false;
                        }
                        if (flg2 == (_byte)eByte::All)
                        {
                            _int32 srcval = mem(dest);
                            set(dest, srcval + val);
                        }
                        else
                        {
                            _int32 srcval = mem(dest, true, (eByte)flg2);
                            set(dest, srcval + val, true, (eByte)flg2);
                        }
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::mov: //DONE
                    case eInstructionSet::mem: //DONE
                    {
                        MemAddress dest = 0;
                        _int32 val = 0;
                        _int32 _off = mem(ip++, false);
                        _int32 v1 =  mem(ip++, false);
                        _int32 v2 =  mem(ip++, false);
                        m_lastInst.push_back(_off);
                        m_lastInst.push_back(v1);
                        m_lastInst.push_back(v2);
                        assignmentConversion(proc, flg1, v1, v2, dest, val, (eByte)flg3, false, _off, flg4);
                        if (dest == oasm_nullptr)
                        {
                            error(eRuntimeErrors::AssignFault, "Assign Failure: <mem> destination cannot be null.", ip - 2, proc);
                            return false;
                        }
                        if (flg4 == (_byte)eFlags::StringAssign)
                        {
                            set(dest++, val);
                            _int32 j = mem(ip++, false);
                            m_lastInst.push_back(j);
                            while (j != 0)
                            {
                                set(dest++, j);
                                j = mem(ip++, false);
                                m_lastInst.push_back(j);
                            }
                            set((MemAddress)eRegisters::IP, ip, false);
                            continue;
                        }
                        if (flg2 == (_byte)eByte::All)
                            set(dest, val);
                        else
                            set(dest, val, true, (eByte)flg2);
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::cmp: //DONE
                    {
                        _int32 v1 = 0, v2 = 0;
                        if (flg1 == (_int32)eFlags::PointerValueCompare)
                        {
                            MemAddress dest = mem(ip++, false);
                            if (dest >= (MemAddress)eRegisters::MemStart && dest < m_memSize)
                                dest = offset(proc, dest);
                            m_lastInst.push_back(dest);
                            if (flg2 == (_byte)eByte::All) v1 = mem(dest);
                            else v1 = mem(dest, true, (eByte)flg2);
                            if (flg3 == (_byte)eByte::All) v2 = mem(ip++, false);
                            else v2 = mem(ip++, false, (eByte)flg3);
                            m_lastInst.push_back(v2);
                        }
                        else if (flg1 == (_int32)eFlags::PointerPointerCompare)
                        {
                            MemAddress dest = mem(ip++, false);
                            m_lastInst.push_back(dest);
                            if (dest >= (MemAddress)eRegisters::MemStart && dest < m_memSize)
                                dest = offset(proc, dest);
                            MemAddress src = mem(ip++, false);
                            if (src >= (MemAddress)eRegisters::MemStart && src < m_memSize)
                                src = offset(proc, src);
                            m_lastInst.push_back(src);
                            if (flg2 == (_byte)eByte::All) v1 = mem(dest);
                            else v1 = mem(dest, true, (eByte)flg2);
                            if (flg3 == (_byte)eByte::All) v2 = mem(src);
                            else v2 = mem(src, true, (eByte)flg3);
                        }
                        else if (flg1 == (_int32)eFlags::ValuePointerCompare)
                        {
                            if (flg2 == (_byte)eByte::All) v1 = mem(ip++, false);
                            else v1 = mem(ip++, false, (eByte)flg2);
                            m_lastInst.push_back(v1);
                            MemAddress src = mem(ip++, false);
                            if (src >= (MemAddress)eRegisters::MemStart && src < m_memSize)
                                src = offset(proc, src);
                            m_lastInst.push_back(src);
                            if (flg3 == (_byte)eByte::All) v2 = mem(src);
                            else v2 = mem(src, true, (eByte)flg3);
                        }
                        else if (flg1 == (_int32)eFlags::ValueValueCompare)
                        {
                            if (flg2 == (_byte)eByte::All) v1 = mem(ip++, false);
                            else v1 = mem(ip++, false, (eByte)flg2);
                            m_lastInst.push_back(v1);
                            if (flg3 == (_byte)eByte::All) v2 = mem(ip++, false);
                            else v2 = mem(ip++, false, (eByte)flg3);
                            m_lastInst.push_back(v2);
                        }
                        else
                        {
                            error(eRuntimeErrors::CMPFlagUnknown, "Unknown Flag: <cmp> flag set to invalid value.", ip - 1, proc);
                            return false;
                        }
                        if (v1 < v2)
                            set((MemAddress)eRegisters::CF, (_int32)eCompareState::Less, false);
                        else if (v1 > v2)
                            set((MemAddress)eRegisters::CF, (_int32)eCompareState::Greater, false);
                        else if (v1 == v2)
                            set((MemAddress)eRegisters::CF, (_int32)eCompareState::Equals, false);
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::com: //DONE
                    {
                        _int32 com = mem(ip++, false);
                        m_lastInst.push_back(com);
                        if (com == (_int32)eComCodes::PrintIntToConsole)
                        {
                            MemAddress addr = offset(proc, mem(ip++, false));
                            m_lastInst.push_back(addr);
                            _int32 val = 0;
                            if (flg2 == (_byte)eByte::All)
                                val = mem(addr);
                            else
                                val = mem(addr, true, (eByte)flg2);
                            m_out->print(val);
                            set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (com == (_int32)eComCodes::PrintStringToConsole)
                        {
                            MemAddress src = offset(proc, mem(ip++, false));
                            m_lastInst.push_back(src);
                            m_out->print(readString(mem(src), flg1));
                            set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (com == (_int32)eComCodes::ReadStringInput)
                        {
                            MemAddress addr = mem(offset(proc, mem(ip++, false)));
                            m_lastInst.push_back(addr);
                            String in = "";
                            m_in->read(in);
                            std::vector<_int32> in_c = Utils::strToMemCells(in);
                            set(addr++, in.length());
                            for (auto& c : in_c)
                                set(addr++, c);
                            set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (com == (_int32)eComCodes::ReadIntInput)
                        {
                            MemAddress addr = offset(proc, mem(ip++, false));
                            m_lastInst.push_back(addr);
                            String in = "";
                            m_in->read(in);
                            if (!Utils::isInt(in))
                            {
                                //TODO: Error
                                return false;
                            }
                            set(addr, Utils::strToInt(in));
                            set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (com == (_int32)eComCodes::PrintNewLineToConsole)
                        {
                            m_out->newLine();
                            set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (com == (_int32)eComCodes::StringLength)
                        {
                            MemAddress addr = mem(offset(proc, mem(ip++, false)));
                            m_lastInst.push_back(addr);
                            String str = readString(addr, flg1);
                            set((MemAddress)eRegisters::R23, (_int32)str.length());
                            set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (com == (_int32)eComCodes::External)
                        {
                            MemAddress addr = mem(ip++, false);
                            m_lastInst.push_back(addr);
                            String data = "";
                            if (addr != (MemAddress)eRegisters::NP)
                                data = readString(mem(offset(proc, addr)), flg1);
                            _uint16 code = (_uint16)(mem((MemAddress)eRegisters::R23));
                            bool res = ECM::instance().execHandler(code, data, *m_out);
                            set((MemAddress)eRegisters::R22, (res ? 1 : 0));
                            set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (com == (_int32)eComCodes::GetArgsData)
                        {
                            _int32 argc = (_int32)proc.m_args.size();
                            set((MemAddress)eRegisters::R23, argc);
                            if (argc == 0)
                            {
                                set((MemAddress)eRegisters::R22, oasm_nullptr);
                                set((MemAddress)eRegisters::IP, ip, false);
                                continue;
                            }
                            MemAddress hp = heapAlloc(argc, proc);
                            if (hp == oasm_nullptr)
                            {
                                error(eRuntimeErrors::HeapAllocFail, "Alloc Fail: Unable to allocate memory for input arguments.", ip - 3, proc);
                                return false;
                            }
                            set((MemAddress)eRegisters::R22, (_int32)hp);
                            for (_uint16 i = 0; i < argc; i++)
                                set(hp + i, proc.m_args[i], false);
                            set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else
                        {
                            error(eRuntimeErrors::UnknownCommand, "Unknown Command.", ip - 1, proc);
                            return false;
                        }
                        continue;
                    }
                    case eInstructionSet::dec: //DONE
                    {
                        MemAddress dest = mem(ip++, false);
                        m_lastInst.push_back(dest);
                        if (dest >= (MemAddress)eRegisters::MemStart && dest < m_memSize)
                             dest = offset(proc, dest);
                        if (flg2 == (_byte)eByte::All)
                            set(dest, mem(dest) - 1);
                        else
                            set(dest, mem(dest, true, (eByte)flg2) - 1, true, (eByte)flg2);
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::div: //DONE
                    {
                        MemAddress dest = 0;
                        _int32 val = 0;
                        _int32 v1 =  mem(ip++, false);
                        _int32 v2 =  mem(ip++, false);
                        m_lastInst.push_back(v1);
                        m_lastInst.push_back(v2);
                        assignmentConversion(proc, flg1, v1, v2, dest, val, (eByte)flg3);
                        if (dest == oasm_nullptr)
                        {
                            error(eRuntimeErrors::AssignFault, "Assign Failure: <div> destination cannot be null.", ip - 2, proc);
                            return false;
                        }
                        if (val == 0)
                        {
                            error(eRuntimeErrors::DivZero, "Division by zero.", ip - 1, proc);
                            return false;
                        }
                        if (flg2 == (_byte)eByte::All)
                        {
                            _int32 srcval = mem(dest);
                            _int32 r = srcval % val;
                            _int32 q = srcval / val;
                            set((MemAddress)eRegisters::DR, r);
                            set(dest, q);
                        }
                        else
                        {
                            _int32 srcval = mem(dest, true, (eByte)flg2);
                            _int32 r = srcval % val;
                            _int32 q = srcval / val;
                            set(dest, q, true, (eByte)flg2);
                            set((MemAddress)eRegisters::DR, r);
                        }
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::end: //DONE
                    {
                        set((MemAddress)eRegisters::ES, 0, false);
                        set((MemAddress)eRegisters::ES, flg1, false, eByte::One);
                        m_running = false;
                        proc.freeMemory(m_memMap);
                        return false;
                    }
                    case eInstructionSet::inc: //DONE
                    {
                        MemAddress dest = mem(ip++, false);
                        if (dest >= (MemAddress)eRegisters::MemStart && dest < m_memSize)
                             dest = offset(proc, dest);
                        m_lastInst.push_back(dest);
                        if (flg2 == (_byte)eByte::All)
                            set(dest, mem(dest) + 1);
                        else
                            set(dest, mem(dest, true, (eByte)flg2) + 1, true, (eByte)flg2);
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::jmp: //DONE
                    {
                        if (flg1 == (_int32)eFlags::JumpUnconditional)
                        {
                            MemAddress ji = offsetCode(proc, mem(ip++, false));
                            m_lastInst.push_back(ji);
                            set((MemAddress)eRegisters::IP, ji, false);
                        }
                        else if (flg1 == (_int32)eFlags::JumpEquals)
                        {
                            MemAddress ji = offsetCode(proc, mem(ip++, false));
                            m_lastInst.push_back(ji);
                            if (mem((_int32)eRegisters::CF) == (_int32)eCompareState::Equals)
                                set((MemAddress)eRegisters::IP, ji, false);
                            else
                                set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (flg1 == (_int32)eFlags::JumpGreater)
                        {
                            MemAddress ji = offsetCode(proc, mem(ip++, false));
                            m_lastInst.push_back(ji);
                            if (mem((_int32)eRegisters::CF) == (_int32)eCompareState::Greater)
                                set((MemAddress)eRegisters::IP, ji, false);
                            else
                                set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (flg1 == (_int32)eFlags::JumpLess)
                        {
                            MemAddress ji = offsetCode(proc, mem(ip++, false));
                            m_lastInst.push_back(ji);
                            if (mem((_int32)eRegisters::CF) == (_int32)eCompareState::Less)
                                set((MemAddress)eRegisters::IP, ji, false);
                            else
                                set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (flg1 == (_int32)eFlags::JumpGreaterEquals)
                        {
                            MemAddress ji = offsetCode(proc, mem(ip++, false));
                            m_lastInst.push_back(ji);
                            if (mem((_int32)eRegisters::CF) == (_int32)eCompareState::Equals || mem((_int32)eRegisters::CF) == (_int32)eCompareState::Greater)
                                set((MemAddress)eRegisters::IP, ji, false);
                            else
                                set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (flg1 == (_int32)eFlags::JumpLssEquals)
                        {
                            MemAddress ji = offsetCode(proc, mem(ip++, false));
                            m_lastInst.push_back(ji);
                            if (mem((_int32)eRegisters::CF) == (_int32)eCompareState::Equals || mem((_int32)eRegisters::CF) == (_int32)eCompareState::Less)
                                set((MemAddress)eRegisters::IP, ji, false);
                            else
                                set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else if (flg1 == (_int32)eFlags::JumpNotEquals)
                        {
                            MemAddress ji = offsetCode(proc, mem(ip++, false));
                            m_lastInst.push_back(ji);
                            if (mem((_int32)eRegisters::CF) != (_int32)eCompareState::Equals)
                                set((MemAddress)eRegisters::IP, ji, false);
                            else
                                set((MemAddress)eRegisters::IP, ip, false);
                        }
                        else
                        {
                            error(eRuntimeErrors::AssignFault, "Unknown Flag: <jmp> flag set to invalid value.", ip - 1, proc);
                            return false;
                        }
                        continue;
                    }
                    case eInstructionSet::mul: //DONE
                    {
                        MemAddress dest = 0;
                        _int32 val = 0;
                        _int32 v1 =  mem(ip++, false);
                        _int32 v2 =  mem(ip++, false);
                        m_lastInst.push_back(v1);
                        m_lastInst.push_back(v2);
                        assignmentConversion(proc, flg1, v1, v2, dest, val, (eByte)flg3);
                        if (dest == oasm_nullptr)
                        {
                            error(eRuntimeErrors::AssignFault, "Assign Failure: <mul> destination cannot be null.", ip - 2, proc);
                            return false;
                        }
                        if (flg2 == (_byte)eByte::All)
                        {
                            _int32 srcval = mem(dest);
                            set(dest, srcval * val);
                        }
                        else
                        {
                            _int32 srcval = mem(dest, true, (eByte)flg2);
                            set(dest, srcval * val, true, (eByte)flg2);
                        }
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::sub: //DONE
                    {
                        MemAddress dest = 0;
                        _int32 val = 0;
                        _int32 v1 =  mem(ip++, false);
                        _int32 v2 =  mem(ip++, false);
                        m_lastInst.push_back(v1);
                        m_lastInst.push_back(v2);
                        assignmentConversion(proc, flg1, v1, v2, dest, val, (eByte)flg3);
                        if (dest == oasm_nullptr)
                        {
                            error(eRuntimeErrors::AssignFault, "Assign Failure: <sub> destination cannot be null.", ip - 2, proc);
                            return false;
                        }
                        if (flg2 == (_byte)eByte::All)
                        {
                            _int32 srcval = mem(dest);
                            set(dest, srcval - val);
                        }
                        else
                        {
                            _int32 srcval = mem(dest, true, (eByte)flg2);
                            set(dest, srcval - val, true, (eByte)flg2);
                        }
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::cat: //DONE
                    {
                        MemAddress dest = mem(offset(proc, mem(ip++, false)));
                        m_lastInst.push_back(dest);
                        String source = readString(dest);
                        MemAddress tmp = oasm_nullptr;
                        _int32 val = 0;
                        _int32 v = mem(ip++, false);
                        m_lastInst.push_back(v);
                        if (flg2 == (_int32)eFlags::StringIntConcat)
                        {
                            assignmentConversion(proc, flg1, 0, v, tmp, val, (eByte)flg3, true);
                            source = source.add(String::intToStr(val));
                        }
                        else if (flg2 == (_int32)eFlags::StringCharConcat)
                        {
                            assignmentConversion(proc, flg1, 0, v, tmp, val, (eByte)flg3, true);
                            source = source.add((char)val);
                        }
                        else if (flg2 == (_int32)eFlags::StringStringConcat)
                        {
                            MemAddress saddr = mem(offset(proc, v));
                            source = source.add(readString(saddr));
                        }
                        else if (flg2 == (_int32)eFlags::StringConstStrConcat)
                        {
                            _uint16 cs = 0;
                            String cstr = readString(ip - 1, (_byte)eFlags::CompactStringReadMode, false, &cs);
                            source = source.add(cstr);
                            ip += cs + 2;
                        }
                        else
                        {
                            error(eRuntimeErrors::CATFlagUnknown, "Unknown Flag: <cat> flag set to invalid value.", ip - 1, proc);
                            return false;
                        }
                        std::vector<_int32> nstr = Utils::strToMemCells(source);
                        set(dest++, source.length()); //TODO: Add check to see if new string exceeds allocated space
                        for (_uint16 i = 0; i < nstr.size(); i++)
                            if (nstr[i] != 0) set(dest++, nstr[i]);
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::alloc_h: //DONE
                    {
                        MemAddress saddr = flags;
                        if (saddr >= (MemAddress)eRegisters::MemStart && saddr < m_memSize)
                            saddr = offset(proc, saddr);
                        _uint16 size = mem(saddr);
                        MemAddress h = heapAlloc(size, proc);
                        if (h == oasm_nullptr)
                        {
                            error(eRuntimeErrors::HeapAllocFail, "Alloc Failure: unable to allocate a block of memory on the Heap.", saddr, proc);
                            return false;
                        }
                        set((MemAddress)eRegisters::R23, (_int32)h);
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::free_h: //DONE
                    {
                        MemAddress start = flags;
                        if (start >= (MemAddress)eRegisters::MemStart && start < m_memSize)
                            start = mem(offset(proc, start));
                        else //TODO: Error trying to free registers and static memory
                            start = mem(start);
                        MemAddress end = mem(start - 1);
                        for (_uint32 i = start - 1; i <= end; i++)
                            m_memMap[i].free(proc);
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::flg: //DONE
                    {
                        if (flags == (_int32)eFlags::ResetFlags)
                        {
                            m_savedFlg = 0;
                            set((MemAddress)eRegisters::FL, 0, false);
                        }
                        else if (flags != (_int32)eFlags::RestoreFlags)
                        {
                            m_savedFlg = mem((MemAddress)eRegisters::FL, false);
                            set((MemAddress)eRegisters::FL, flags, false);
                        }
                        else
                        {
                            m_savedFlg = 0;
                            set((MemAddress)eRegisters::FL, m_savedFlg, false);
                        }
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::push: //DONE
                    {
                        MemAddress tmp = oasm_nullptr;
                        _int32 val = 0;
                        _int32 v = mem(ip++, false);
                        m_lastInst.push_back(v);
                        assignmentConversion(proc, flg1, 0, v, tmp, val, (eByte)flg2, true);
                        push(val, proc);
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::pop:
                    {
                        _int32 reg = mem(ip++, false);
                        m_lastInst.push_back(reg);
                        if (reg >= (_int32)eRegisters::MemStart)
                        {
                            error(eRuntimeErrors::PopReg, "The <pop> instruction expects a register.", (MemAddress)reg, proc);
                            return false;
                        }
                        bool read = ((_byte)eFlags::StackRead == flg4);
                        set((MemAddress)reg, pop(proc, read), false);
                        set((MemAddress)eRegisters::IP, ip, false);
                        continue;
                    }
                    case eInstructionSet::call:
                    {
                        push(mem((MemAddress)eRegisters::SP, false), proc);
                        push(ip, proc);
                        MemAddress ipsp = mem((MemAddress)eRegisters::SP, false);
                        _int32 nip = offsetCode(proc, mem(ip++, false));
                        _int32 param_count = mem(ip++, false);
                        m_lastInst.push_back(nip);
                        m_lastInst.push_back(param_count);
                        if (param_count > 0)
                        {
                            _int32 aType = 0;
                            MemAddress tmp = oasm_nullptr;
                            _int32 val = 0, v = 0;
                            for (_uint16 i = 0; i < (_uint16)param_count; i++)
                            {
                                aType = mem(ip++, false);
                                v = mem(ip++, false);
                                m_lastInst.push_back(aType);
                                if (v < (MemAddress)eRegisters::MemStart || v >= m_memSize)
                                    m_lastInst.push_back(v);
                                else
                                    m_lastInst.push_back(offset(proc, v));
                                assignmentConversion(proc, aType, 0, v, tmp, val, eByte::All, true);
                                push(val, proc);
                            }
                        }
                        push(param_count, proc);
                        set(ipsp, ip, false);
                        set((MemAddress)eRegisters::IP, nip, false);
                        continue;
                    }
                    case eInstructionSet::ret:
                    {
                        MemAddress tmp = oasm_nullptr;
                        _int32 val = 0;
                        _int32 v = mem(ip++, false);
                        m_lastInst.push_back(v);
                        assignmentConversion(proc, flg1, 0, v, tmp, val, (eByte)flg2, true);
                        set((MemAddress)eRegisters::RV, val, false);
                        _int32 oip = pop(proc);
                        _int32 osp = pop(proc);
                        if (osp != mem((MemAddress)eRegisters::SP))
                        {
                            error(eRuntimeErrors::SPDiscrepancy, "Stack Pointer discrepancy on sub-routine <ret> instruction.", (MemAddress)eRegisters::SP, proc);
                            return false;
                        }
                        set((MemAddress)eRegisters::IP, oip, false);
                        continue;
                    }
                    default:
                    break;
                }
            }
            return true;
        }

        bool ProcessManager::killRunningProc(void)
        {
            if (m_runningProc == nullptr || !m_running || !m_runningProc->m_valid) return false;
            printMemoryBlock(m_runningProc->m_startAddr, m_runningProc->m_startAddr + m_runningProc->m_codeSize + m_runningProc->m_dataSize, "Process");
            m_runningProc->kill();
            set((MemAddress)eRegisters::ES, (_int32)eExitCodes::ForceQuit, false);

            for (MemAddress addr = m_runningProc->m_startAddr; addr < m_runningProc->m_startAddr + m_runningProc->m_codeSize + m_runningProc->m_dataSize; addr++)
                m_memMap[addr].release();
            for (MemAddress addr = m_runningProc->m_heapStartAddr; addr < m_runningProc->m_heapStartAddr + m_runningProc->m_heapSize; addr++)
                m_memMap[addr].release();
            //TODO: Release stack aswell (when implemented)
            return true;
        }

        bool ProcessManager::push(_int32 val, Process& proc)
        {
            _int32 sp = mem((MemAddress)eRegisters::SP, false);
            if (++sp >= proc.m_stackStartAddr + proc.m_stackSize)
            {
                error(eRuntimeErrors::StackOverflow, "Stack overflow", (MemAddress)sp, *m_runningProc);
                return false;
            }
            set(sp, val, false);
            set((MemAddress)eRegisters::SP, sp);
            return true;
        }

        _int32 ProcessManager::pop(Process& proc, bool read)
        {
            _int32 sp = mem((MemAddress)eRegisters::SP, false);
            if (sp < proc.m_stackStartAddr)
            {
                set((MemAddress)eRegisters::FL, (_int32)eFlags::StackEmpty, false);
                return 0;
            }
            _int32 val = mem(sp, false);
            if (!read)
            {
                sp--;
                set((MemAddress)eRegisters::SP, sp);
            }
            return val;
        }

        _int32 ProcessManager::mem(MemAddress addr, bool restricted, eByte byte)
        {
            if (!restricted)
            {
                if (byte == eByte::All) return m_memory[addr];
                else return (_int32)Utils::getb(m_memory[addr], byte);
            }
            if (!m_running || m_runningProc == nullptr) return oasm_nullptr;
            if (addr >= m_memory.size()) return oasm_nullptr;
            if (addr < (MemAddress)eRegisters::MemStart) return m_memory[addr];
            if (m_memMap[addr].isProtected())
            {
                error(eRuntimeErrors::ACCVmemProtected, "Access violation: Attempt to read a protected memory cell.", addr, *m_runningProc);
                return oasm_nullptr;
            }
            if (m_memMap[addr].isStack())
            {
                error(eRuntimeErrors::ACCVmemStack, "Access violation: Cannot read Stack with <mem> function.", addr, *m_runningProc);
                return oasm_nullptr;
            }
            if (!m_memMap[addr].testFor(*m_runningProc))
            {
                error(eRuntimeErrors::ACCVmemPermission, "Access violation: Process does not have read permission.", addr, *m_runningProc);
                return oasm_nullptr;
            }
            if (byte == eByte::All) return m_memory[addr];
            else return (_int32)Utils::getb(m_memory[addr], byte);
        }

        String ProcessManager::readString(MemAddress addr, _byte flg, bool restricted, _uint16* cellCount)
        {
            String str;
            if (flg == (_byte)eFlags::OldStringReadMode)
            {
                addr++;
                char c = (char)mem(addr++, restricted);
                while (c != 0)
                {
                    str = str.add(c);
                    c = (char)mem(addr++, restricted);
                }
            }
            else
            {
                _uint32 len = mem(addr++, restricted);
                MemAddress oldAddr = addr;
                eByte byte;
                _byte b = 4;
                for (_uint32 i = 0; i < len; i++)
                {
                    if (b == (_byte)eByte::One) byte = eByte::One;
                    if (b == (_byte)eByte::Two) byte = eByte::Two;
                    if (b == (_byte)eByte::Three) byte = eByte::Three;
                    if (b == (_byte)eByte::Four) byte = eByte::Four;
                    str = str.add((char)(mem(addr, restricted, byte)));
                    if (b == 1)
                    {
                        b = 5;
                        addr++;
                    }
                    b--;
                }
                if (cellCount != nullptr) *cellCount = (addr - oldAddr);
            }
            return str;
        }

        bool ProcessManager::set(MemAddress addr, _int32 value, bool restricted, eByte byte)
        {
            if (!restricted)
            {
                if (byte == eByte::All)
                    m_memory[addr] = value;
                else
                    m_memory[addr] = Utils::setb(m_memory[addr], byte, (_ubyte)value);
                return true;
            }
            if (!m_running || m_runningProc == nullptr) return false;
            if (addr >= m_memory.size()) return false;
            if (m_memMap[addr].isProtected())
            {
                error(eRuntimeErrors::ACCVsetProtected, "Access violation: Attempt to write to a protected memory cell.", addr, *m_runningProc);
                return oasm_nullptr;
            }
            if (m_memMap[addr].isReadOnly())
            {
                error(eRuntimeErrors::ACCVsetReadOnly, "Access violation: Attempt to write to a read-only memory cell.", addr, *m_runningProc);
                return oasm_nullptr;
            }
            if (addr < (MemAddress)eRegisters::MemStart)
            {
                if (byte == eByte::All)
                    m_memory[addr] = value;
                else
                    m_memory[addr] = Utils::setb(m_memory[addr], byte, (_ubyte)value);
                return true;
            }
            if (m_memMap[addr].isStack())
            {
                error(eRuntimeErrors::ACCVsetStack, "Access violation: Cannot write to Stack with <mem> function.", addr, *m_runningProc);
                return oasm_nullptr;
            }
            if (!m_memMap[addr].testFor(*m_runningProc))
            {
                error(eRuntimeErrors::ACCVsetPermission, "Access violation: Process does not have write permission.", addr, *m_runningProc);
                return false;
            }
            if (byte == eByte::All)
                m_memory[addr] = value;
            else
                m_memory[addr] = Utils::setb(m_memory[addr], byte, (_ubyte)value);
            return true;
        }

        bool ECM::addHandler(_uint16 code, ExtComHandler& ech)
        {
            if (hasHandler(code)) return false;
            m_comHandlers[code] = &ech;
            return true;
        }

        bool ECM::hasHandler(_uint16 code)
        {
            return m_comHandlers.count(code) != 0;
        }

        bool ECM::execHandler(_uint16 code, String data, OutputManager& out)
        {
            if (!hasHandler(code)) return false;
            return m_comHandlers[code]->handleCommand(code, data, out);
        }
    }
}
