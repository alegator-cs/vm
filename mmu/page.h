#ifndef PAGE_H
#define PAGE_H
#include <cstdint>

class page
{
    public:
    // 32-bit words
    uint32_t pageNum; // First 24 bits as page num 
    uint8_t offset; // last 8 bits as offset
    
    page(uint32_t address);

    // Getters
    uint32_t getPage() const{return pageNum;}

    uint8_t getOffset() const{return offset;}
};
#endif