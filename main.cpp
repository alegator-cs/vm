#include <cstddef>

// TODO: probably exists more sensible macro to indicate *nix
#ifndef _WIN32
#include <unistd.h>
#endif

#include <cstring>
#include <cctype>
#include <cstdio>

#include <algorithm>
#include <iterator>
#include <vector>
#include <tuple>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "vm.h"

// TODO: integer variables throughout the file should be int32_t except for bit manipulation

std::string tabs_to_align(const std::string& line) {
  const int32_t tab_count = 4 - line.size() / 4;
  return std::string(tab_count, '\t');
}

std::string trim(std::string& str) {
  auto lhs = std::find_if(std::begin(str), std::end(str), [](char c) {
    return !std::isspace(c);
  });
  auto rev_rhs = std::find_if(std::rbegin(str), std::rend(str), [](char c) {
    return !std::isspace(c);
  });
  auto rhs = rev_rhs.base();
  if (lhs != std::end(str) /*&& rhs != std::rend(str)*/) {
    std::ptrdiff_t dist = std::distance(lhs, rhs);
    std::string tr(dist, '\0');
    std::copy(lhs, rhs, tr.begin());
    return tr;
  }
  else return "";
}

void print_usage(char *prog_name) {
  printf("Usage:\t%s <input_file>\n", prog_name);
}

bool isinteger(char c) {
  return std::isdigit(c) || c == '.' || c == '-';
}

// TODO: proper sizeof for dump element count
void debug_dump_regs(vm *m, uint32_t dump[14]) {
  int max = sizeof(m->regs) / sizeof((m->regs)[0]);
  for (int i = 0; i < max; i++) {
    dump[i] = (m->regs)[i];
  }
}

// TODO: proper sizeof for dump element count
void debug_dump_mem(vm *m, uint8_t dump[4096]) {
  int max = sizeof(m->mem);
  for (int i = 0; i < max; i++) {
    dump[i] = (m->mem)[i];
  }
}

// TODO: proper sizeof for dump element count
void debug_print_regs_delta(vm *m, uint32_t dump[14]) {
  int max = sizeof(m->regs) / sizeof((m->regs)[0]);
  for (int i = 0; i < max; i++) {
    if (m->regs[i] != dump[i]) {
      std::cerr << "regs[" << i << "] = " << m->regs[i] << std::endl;
    }
  }
}

// TODO: proper sizeof for dump element count
void debug_print_mem_delta(vm *m, uint8_t dump[4096]) {
  int max = sizeof(m->mem);
  for (int i = 0; i < max; i++) {
    if (m->mem[i] != dump[i]) {
      std::ios init(nullptr);
      init.copyfmt(std::cerr);
      std::cerr << "mem[" << i << "] = " << std::hex << std::setfill('0') << std::setw(2) << (int32_t)((m->mem)[i]) << std::endl;
      std::cerr.copyfmt(init);
    }
  }
}

int main(int argc, char **argv) {
  // TODO: support concurrent filepaths as round robin scheduled processes
  // command line argument for filepath to read the asm from
  // read in this way because vscode debugger is weird
  // just use argv for final executable
  std::cerr << "(vscode debug mode)" << " enter the filenames for the processes to run" << std::endl
            << "newline terminates" << std::endl;
  auto args = std::vector<std::string>{};
  std::string line{};
  while (std::getline(std::cin, line) && line != "") {
    args.emplace_back(line);
  }

  // print heading
  auto post = std::string(40, '=');
  std::cerr << std::endl << post << "parsing input files (output is merged).." << post << std::endl;
  std::cerr << "asm\t\t\top_code\t\t\targ1\t\t\targ2" << std::endl;

  // the constructor will parse the filepaths
  vm m{args, mmu_t{sizeof(m.mem)}};

  /*
  auto check_empty = [](vm::progs_t& progs) {
    bool empty = false;
    for (auto&& p : progs) {
      if (p.empty()) empty = true;
    }
    return empty;
  };
  */

  int32_t idx = 0;
  // executing each process independently in sequence is a step towards multiprocessing
  for (auto&& prog : m.progs) {
    m.procs[idx++].dumpPcb();
    
    // vm execution
    std::cerr << std::endl << post << "executing vm instructions.." << post << std::endl;
    std::cerr << "asm\t\t\top_code\t\t\targ1\t\t\targ2" << std::endl;
    for (auto&& op : prog) {
      // TODO: proper sizeof for dump element count
      // dump the registers and memory so we can check if the next instruction changed anything
      
      uint32_t reg_dump[14];
      uint8_t mem_dump[4096];
      debug_dump_regs(&m, reg_dump);
      debug_dump_mem(&m, mem_dump);

      // TODO: implement the merged binary file for all programs in one executable
      // then the round robin scheduler can work
      // while(round_robin_pass());

      // retrieve the op_code and arguments to next instruction, and execute it
      auto&& [line, op_code, arg1, arg2] = op;
      (m.*((m.op_table)[op_code]))(arg1, arg2);
      
      // print what was executed
      std::cerr << line << tabs_to_align(line) << op_code << "\t\t\t" << arg1 << "\t\t\t" << arg2 << std::endl;

      if (memcmp(m.regs, reg_dump, sizeof(reg_dump)) != 0) {
        // if the registers changed, print the changed register value(s)
        debug_print_regs_delta(&m, reg_dump);
      } else if (memcmp(m.mem, mem_dump, sizeof(mem_dump)) != 0) {
        // if the memory changed, print the changed memory value(s)
        debug_print_mem_delta(&m, mem_dump);
      }
      std::cerr << std::endl;
    }
  
  }
  

  // ----- test function calls

  for(const auto& elem : m.mmu.pTable)
  {
    m.mmu.getAddress(elem.first, elem.second);
  }

  return 0;
}
