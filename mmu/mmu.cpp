#include "mmu.h"

std::string filename = "C:/Users/nihum/github-classroom/Spring-2024-CSCI480/assignment-01-nico-and-oleg/bin/data.bin";

page::page(uint32_t address)
{
    pageNum = (address >> 8) & 0xFFFFFF; // Drops offset
    offset = address & 0xFF;
}

// Converts passed string to int - FOR READING FROM FILE
int mmu_t::toInt(std::string bin)
{
    int spot = 0;
    int num = 0;

    for(int i = (bin.size()-1); i >= 0; i--)
    {
            if(bin[i]== '1')
            {
                num += pow(2, spot);
            }
            spot++;
    }
    return num;
}

// Helper function - converts passed int to 32 bit binary string - FOR WRITING TO FILE
std::string mmu_t::toBinary(int n) 
{
    std::string s; 

    while(n != 0)
    { 
        s = (n % 2 == 0 ? "0" : "1") + s;
        n /= 2;
    }

    while(s.size() < 32)
    {
        s = '0' + s;
    }

   // int converted = std::stoi(s);
    return s;
}


// maps virtual memory -> "physical" memory.  vMem = virtual address, pMem = physical address
void mmu_t::mapMemory(uint32_t _page, uint32_t _offset)
{
    pTable.insert({_page, _offset});
}

// function to break and translate "physical" (long term memory) memory into virtual page num + offset
uint32_t mmu_t::getAddress(uint32_t page, uint32_t offset)
{
    std::cerr << "Page#: " << page << " Offset: " << offset << " Value: " << (page * pageSize) + offset << std::endl;
    return (page * pageSize) + offset;
}

void mmu_t::readFile(std::string file)
{
    int _data;
    std::ifstream readFile(file.data());
    readFile >> _data;

    page mem(_data);

    /*
    if(pMem >= data.size())
    {
        // __throw_out_of_range("address out of range");
    }
    */

   // std::cout << "\n" << data[pMem];
}

// writes passed data to file
void mmu_t::write_file(uint32_t _data)
{
    std::ofstream writeFile;
    std::string temp = toBinary(_data);

    // opens or creates bin file will append
    writeFile.open(filename, std::ios::out | std::ios::app | std::ios::binary);
    writeFile << temp << std::endl;  

    int d = toInt(temp);

    page m(d);
    uint32_t pageNum = m.getPage();
    uint8_t offset = m.getOffset();
    mapMemory(pageNum, offset);
    
    writeFile.close(); 
}
