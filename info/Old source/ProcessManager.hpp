#ifndef __EXECUTOR_HPP__
#define __EXECUTOR_HPP__

#include "Types.hpp"
#include "IOManager.hpp"

namespace Omnia
{
    namespace oasm
    {
        class Process;
        class MemCellState
        {
            public:
                MemCellState(eMemCellType type = eMemCellType::Normal, eMemState state = eMemState::Free);
                inline eMemState state(void) { return m_state; }
                inline eMemCellType type(void) { return m_type; }

                inline bool isProtected(void) { return m_protected; }
                inline bool isReadOnly(void) { return m_readOnly; }
                inline void setProtected(bool pro) { m_protected = pro; }
                inline void setReadOnly(bool ro) { m_readOnly = ro; }

                inline bool isReserved(void) const { return m_state == eMemState::Reserved; }
                inline bool isFree(void) const { return m_state == eMemState::Free; }
                inline bool isAllocated(void) const { return m_state == eMemState::Allocated; }

                inline bool isRegister(void) const { return m_type == eMemCellType::Register; }
                inline bool isNormal(void) const { return m_type == eMemCellType::Normal; }
                inline bool isHeap(void) const { return m_type == eMemCellType::Heap; }
                inline bool isStack(void) const { return m_type == eMemCellType::Stack; }

                inline bool isUsed(void) const { return m_used || isReserved(); }

                bool use(Process& proc);
                bool testFor(Process& proc);
                bool free(Process& proc);
                bool release(void);
                bool set(eMemState state, Process* proc = nullptr);

            private:
                eMemCellType m_type;
                eMemState m_state;
                Process* m_process;
                bool m_used;
                bool m_protected;
                bool m_readOnly;
        };

        class DebugInfo
        {
            public:
                inline DebugInfo(void)
                {
                    setAll(false);
                    useDebug = false;
                    dbginf = 0;
                }
                inline void setAll(bool s)
                {
                    stepExecution = s;
                    useDebug = s;
                }
                inline void useAll(void)
                {
                    useDebug = true;
                    setAll(true);
                }

            public:
                _int32 dbginf;
                bool useDebug;

                bool stepExecution;
        };

        class Process
        {
            public:
                inline Process(void) { kill(); }
                void kill(void);
                inline MemAddress getStartAddress(void) { return m_startAddr; }
                inline _uint16 getHandle(void) { return m_handle; }
                bool isValid(void);
                bool freeMemory(std::vector<MemCellState>& memMap);
                inline _uint32 totalMemoryUsed(void) { return m_codeSize + m_dataSize + m_heapSize + m_stackSize; }

            public:
                _uint16 m_handle;
                std::vector<_int32> code;
                MemAddress m_startAddr;
                _uint16 m_codeSize;
                _uint16 m_dataSize;
                _uint16 m_heapSize;
                MemAddress m_heapStartAddr;
                _uint16 m_stackSize;
                MemAddress m_stackStartAddr;
                std::vector<_int32> m_args;
                DebugInfo m_debug;

            private:
                bool m_valid;

                friend class ProcessManager;
        };

        class ExtComHandler
        {
            public:
                inline virtual bool handleCommand(_uint16 code, String data, OutputManager& out) { return false; }
        };

        class ECM
        {
            public:
                static inline ECM& instance(void) { return s_instance; }
                bool addHandler(_uint16 code, ExtComHandler& ech);
                bool hasHandler(_uint16 code);
                bool execHandler(_uint16 code, String data, OutputManager& out);

            private:
                inline ECM(void) {  }

            private:
                std::map<_uint16, ExtComHandler*> m_comHandlers;
                static ECM s_instance;
        };

        inline ECM ECM::s_instance;

        class ProcessManager : public IOReciever
        {
            public:
                inline ~ProcessManager(void) {  }
                static inline ProcessManager& instance(void) { return s_instance; }
                void printMemoryBlock(_uint32 start = 0, _uint32 end = 0, String text = "");
                void allocate(_uint32 memSize);
                inline _uint32 getMemSize(void) { return m_memSize; }
                inline void setInstructionsPerTick(_uint16 ipu) { m_ipu = ipu; }
                inline _uint32 newProcHandle(void) { return ProcessManager::s_nextProcHandle++; }
                MemAddress requestMemory(_uint16 size, Process& proc);
                MemAddress requestHeap(_uint16 size, Process& proc);
                MemAddress requestStack(_uint16 size, Process& proc);
                MemAddress heapAlloc(_uint16 size, Process& proc);
                bool runFromFile(String fileName, std::vector<_int32> args = std::vector<_int32>());
                bool runProcess(Process& proc, std::vector<_int32> args = std::vector<_int32>());
                bool checkAddress(MemAddress addr);
                bool tick(void);
                bool killRunningProc(void);
                const std::vector<_int32>& getMemory(void) { return m_memory; }
                const std::vector<MemCellState>& getMemMap(void) { return m_memMap; }
                inline MemAddress getHeapStart(void) { return getMemSize(); }
                inline MemAddress getStackStart(void) { return getMemSize() + AUTO__OASM_HEAP_SIZE; }
                inline _uint32 getFullMemorySize(void) { return m_memory.size(); }
                inline Process* getRunningProcess(void) { return m_runningProc; }

            private:
                inline ProcessManager(void) { allocate(AUTO__OASM_MEM_ALLOC); }
                _int32 mem(MemAddress addr, bool restricted = true, eByte byte = eByte::All);
                bool set(MemAddress addr, _int32 value, bool restricted = true, eByte byte = eByte::All);
                bool push(_int32 val, Process& proc);
                _int32 pop(Process& proc, bool read = false);
                String readString(MemAddress addr, _byte flg = (_byte)eFlags::CompactStringReadMode, bool restricted = true, _uint16* cellCount = nullptr);
                inline MemAddress offset(Process& proc, MemAddress base) { return proc.m_codeSize + proc.m_startAddr + base - (MemAddress)eRegisters::MemStart; }
                inline MemAddress offsetCode(Process& proc, MemAddress base) { return proc.m_startAddr + base; }
                void nextProcess(void);
                bool copyToMemory(Process& proc);
                void assignmentConversion(Process& proc, _int32 flg, _uint32 v1, _int32 v2, MemAddress& dest, _int32& val, eByte byte = eByte::All, bool skipDest = false, _int32 _off = 0, _byte offsetType = (_int32)eFlags::NoOffset);
                void error(eRuntimeErrors err, String msg, MemAddress addr, Process& proc, bool forceQuit = true);

            private:
                std::vector<_int32> m_memory;
                std::vector<MemCellState> m_memMap;
                std::vector<Process*> m_processes;
                std::vector<_int32> m_lastInst;
                Process* m_runningProc;
                _uint32 m_memSize;
                _uint32 m_memStart;
                _uint16 m_ipu;
                bool m_running;
                _int32 m_savedFlg;

            private:
                static ProcessManager s_instance;
                static _uint32 s_nextProcHandle;

                friend class Debugger;
        };

        inline ProcessManager ProcessManager::s_instance;
        inline _uint32 ProcessManager::s_nextProcHandle = 0x1000;
    }
}

#endif
