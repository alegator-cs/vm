#ifndef PCB_H
#define PCB_H

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

class pcb
{
    public:
        enum state {
            NEW,
            READY,
            RUNNING,
            WAIT,
            WAITLOCK,
            WAITEVENT,
            ZOMBIE
        } status;

        uint32_t processID = 0;
        uint32_t priority = 0;
        uint32_t registers[14] = {0};
        uint32_t codeSize = 0;
        uint32_t stackSize = 0;
        // no private heaps
        // uint8_t *heap;
        // uint32_t heapSize;
        std::vector<uint8_t *> pages = {};
        uint32_t pagesSize = 0;
        uint32_t flags = 0;
        
        pcb();
        pcb(int32_t pid, int32_t prio);
        void setProcessID(int32_t pid);
        void setPriority(int32_t prio);
        void setState(state s); 
        void setRegs(uint32_t regs[14]);
        void dumpPcb();
};
#endif