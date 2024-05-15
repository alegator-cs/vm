#include "pcb.h"

pcb::pcb()
{
    processID = 0;
    priority = 0;
    status = NEW;
    registers[14] = {0};
}

pcb::pcb(int32_t pid, int32_t prio)
{
    processID = pid;
    priority = prio;
    status = NEW;
}

void pcb::setProcessID(int32_t pid) { processID = pid; }

void pcb::setPriority(int32_t prio) { priority = prio;}

void pcb::setState(state s){state st = s;}

void pcb::setRegs(uint32_t regs[14]) {
    for (int i = 0; i < 15; i++)
    {
        registers[i] = regs[i];
    }
}

void pcb::dumpPcb()
{
    std::cout << "Process ID: " << processID << std::endl;
    std::cout << "Priority: " << priority << std::endl;

    // Print state as string
    switch (status)
    {
    case RUNNING:
        std::cout << "State: " << "RUNNING" << std::endl;
        break;
    case WAIT:
        std::cout << "State: " << "WAIT" << std::endl;
        break;
    case READY:
        std::cout << "State: " << "READY" << std::endl;
        break;
    case ZOMBIE:
        std::cout << "State: " << "ZOMBIE" << std::endl;
        break;
    default:
        std::cout << "State: " << "NEW" << std::endl;
        break;
    }
   
    // Dump all regs
    for (int i = 0; i < 15; i++)
    {
        std::cout << "reg[" << i << "]: " << registers[i] << std::endl;
    }
}