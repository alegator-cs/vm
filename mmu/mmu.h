#ifndef MMU_H
#define MMU_H

#include "page.h"

#include <iomanip>
#include <iostream>
#include <fstream>

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <math.h>
#include <string>

class mmu_t
{ 
    public:
        std::vector<uint32_t> data; // long term memory
        std::unordered_map<uint32_t, uint32_t> pTable; // Page table
        static constexpr uint32_t pageSize = 256;

        mmu_t(size_t size) : data(size){}
        mmu_t() = default; 

        void mapMemory(uint32_t vMem, uint32_t pMem);

        uint32_t getAddress(uint32_t page, uint32_t offset);  

        void readFile(std::string file);

        void write_file(uint32_t _data);

        int toInt(std::string bin);

        std::string toBinary(int n);
};
#endif