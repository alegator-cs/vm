#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cctype>
#include <cstdio>

#include <vector>
#include <algorithm>
#include <iterator>
#include <vector>
#include <tuple>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <iostream>

#include "vm.h"

static bool isinteger(char c) {
  return std::isdigit(c) || c == '.' || c == '-';
}

static std::string tabs_to_align(const std::string& line) {
  const uint32_t tab_count = 4 - line.size() / 4;
  return std::string(tab_count, '\t');
}

// TODO: auto type tup
// TODO: this function is also in main.cpp, remove a duplicate once decide what goes where
static void debug_print_op(std::tuple<std::string, int, int, int> op) {
  auto&& [line, op_code, arg1, arg2] = op;
  std::cerr << line << tabs_to_align(line) << op_code << "\t\t\t" << arg1 << "\t\t\t" << arg2;
}

// read 4 bytes of little endian memory to an integer
#define mem_at(adr) \
  ( (uint32_t)((adr)[0]) + (uint32_t)((adr)[1]) << 8 + (uint32_t)((adr)[2]) << 16 + (uint32_t)((adr)[3]) << 24 ) \

// write a little endian integer to 4 bytes of memory
#define mem_to(adr, val) \
  (adr)[0] = (uint8_t)(val); \
  (adr)[1] = (uint8_t)((val) >> 8); \
  (adr)[2] = (uint8_t)((val) >> 16); \
  (adr)[3] = (uint8_t)((val) >> 24); \

// put a bit in the nth position
#define bit(n) ((uint32_t)1 << (uint32_t)n)

// get the bit at the nth position
#define get_bit(val, n) (((uint32_t)val >> (uint32_t)n) & 1)

// set the bit at the nth position
#define set_bit(val, n) ((val) = (uint32_t)val | bit(n))

// reset the bit at the nth position
#define reset_bit(val, n) ((val) = (uint32_t)val & ~bit(n))

#define ZERO_BIT (0)
#define SIGN_BIT (1)

// TODO: this function should be cleaned up and made more robust
// TODO: the string trimming belongs in utility functions
// TODO: the use of std::getline breaks if there is any whitespace besides ' '
vm::ops_t vm::parse_file(std::string& filepath) {
  auto ifs = std::ifstream{filepath};
  if (!ifs) return {};

  ops_t ops{};

  // iterate the lines of the file
  std::string line;
  while (std::getline(ifs, line)) {
    // ignore leading whitespace
    // can't use std::isspace directly since `int isspace(char c)` won't coerce to `bool (*)(char c)`
    auto isspace_wrapper = +[](char c){ return (bool)std::isspace(c); };
    auto lhs = std::find_if_not(std::begin(line), std::end(line), isspace_wrapper);
    
    // ignore comments and empty lines
    auto first = *lhs;
    if (first == '#' || first == '\0') continue;

    // these will always be parsed for a value so initializing to "0" is important
    std::string op_code_str = "0";
    std::string arg1_str = "0";
    std::string arg2_str = "0";

    // whether each arg is a register number, an integer constant, or a char constant
    enum class arg_e {
      imm,
      ch,
      reg,
    };
    arg_e arg1_type = arg_e::imm;
    arg_e arg2_type = arg_e::imm;

    // the operands to the instruction and its op code
    int32_t op_code = 0;
    int32_t arg1 = 0;
    int32_t arg2 = 0;

    // get an istringstream of the left trimmed input line
    size_t skipped_spaces_count = std::distance(std::begin(line), lhs);
    size_t len = std::distance(lhs, std::end(line));
    char *cstr = line.data() + skipped_spaces_count;
    auto sv = std::string_view{cstr, len};
    auto ltrim = std::string{sv};
    auto ss = std::istringstream{ltrim};

    // parse the instruction op name
    if (!std::getline(ss, op_code_str, ' ')) continue;

    // parse arg1
    while (ss && !isinteger(ss.peek())) {
      char c = ss.get();
      if (c == '$') {
        arg1_type = arg_e::ch;
      } else if (c == '#') {
        arg1_type = arg_e::imm;
      } else {
        arg1_type = arg_e::reg;
      }
    }
    if (ss) std::getline(ss, arg1_str, ' ');

    // parse arg2
    while (ss && !isinteger(ss.peek())) {
      char c = ss.get();
      if (c == '$') {
        arg2_type = arg_e::ch;
      } else if (c == '#') {
        arg2_type = arg_e::imm;
      } else {
        arg2_type = arg_e::reg;
      }
    }
    if (ss) std::getline(ss, arg2_str, '\n');

    // parse whatever was read (or the default values of "0") as the instruction fields
    op_code = get_op_code(op_code_str.data());
    arg1 = std::stoi(arg1_str);
    arg2 = std::stoi(arg2_str);

    // adjust for argument types
    // TODO: instruction suffixes could and should be handled as preprocessor macros
    // then the macro can expand to a function call with the operands parsed based on type
    if (arg1_type == arg_e::ch) {
      arg1 += '0';
    }
    if (arg2_type == arg_e::ch) {
      arg2 += '0';
    }

    // only print the line parsed if we got this far, so only executable instructions print
    std::cerr << line << tabs_to_align(line) << op_code << "\t\t\t" << arg1 << "\t\t\t" << arg2 << std::endl;

    // store the op
    ops.emplace_back(line, op_code, arg1, arg2);
  }

  return ops;
}

vm::vm(std::string filepath, mmu_t mmu) : mmu{mmu} {
  *sp = (uint32_t)mem;
  
}

vm::vm(std::vector<std::string>& args, mmu_t mmu) : mmu{mmu} {
  *sp = (uint32_t)mem;
  int32_t pid = 0;
  int32_t st = pcb::NEW;
  for (auto&& filepath : args) {
    progs.emplace_back(parse_file(filepath));
    procs.emplace_back(pcb{pid, st});
    pid++;
  }
}

int32_t vm::get_op_code(char *op) {
  int32_t i = 0;
  // TODO: proper sizeof for index max size
  while (i < 39 && std::strcmp(op, vm::op_lookup[i]) != 0) {
    i++;
  }
  return i;
}

bool vm::round_robin_pass() {
  for (auto&& p : procs) {
    bool done = true;
    if (p.status == pcb::READY) {
      done = false;
      int32_t next_pid = (p.processID + 1) % procs.size();
      for (int32_t i = 0; i < 15; i++) {
        regs[i] = p.registers[i];
      }
      *pid = next_pid;
      // TODO: update ip and sp based on the starting address, prog size, and stack size
      // this requires a merged binary file including all the programs organized into
      // process registers stored at the top, then the stack growing towards the heap
      // the stack starts from mem[0] increasing, heap from mem[4095] decreasing
      // there is not yet code to initialize the memory of the parsed files into one
      // executable binary file with these layouts
    }
    if (!done) return true;
  }
  return false;
}

//    Instruction definitions
// ----------------------------------

void vm::incr(int32_t r,  int32_t) {
  regs[r] += 1;
}

void vm::addi(int32_t r, int32_t i) {
  regs[r] += i;
}

void vm::addr(int32_t rx, int32_t ry) {
  regs[rx] += regs[ry];
}

void vm::pushr(int32_t r, int32_t) {
  mem_to(sp, regs[r]);
  sp--;
}

void vm::pushi(int32_t i, int32_t) {
  mem_to(sp, i);
  sp--;
}

void vm::movi(int32_t r, int32_t i) {
  regs[r] = i;
}

void vm::movr(int32_t rx, int32_t ry) {
  regs[rx] = regs[ry];
}

void vm::movmr(int32_t rx, int32_t ry) {
  auto *adr = &mem[regs[rx]];
  regs[rx] = mem_at(adr);
}

void vm::movrm(int32_t rx, int32_t ry) {
  auto *adr = &mem[regs[rx]];
  auto val = regs[ry];
  mem_to(adr, val);
}

void vm::movmm(int32_t rx, int32_t ry) {
  auto *adrx = &mem[regs[rx]];
  auto *adry = &mem[regs[ry]];
  auto val = mem_at(adrx);
  mem_to(adry, val);
}

void vm::printr(int32_t r, int32_t) {
  std::cout << ":" << regs[r] << std::endl;
}

void vm::printm(int32_t r, int32_t) {
  auto *adr = &mem[regs[r]];
  std::cout << ":" << mem_at(adr) << std::endl;
}

void vm::printcr(int32_t r, int32_t) {
  std::cout << ":" << (char)('0' + regs[r]) << std::endl;
}

void vm::printcm(int32_t r, int32_t) {
  auto *adr = &mem[regs[r]];
  std::cout << ":" << (char)('0' + mem_at(adr)) << std::endl;
}

void vm::jmp(int32_t r, int32_t) {
  *ip += regs[r];
}

void vm::jmpi(int32_t i, int32_t) {
  *ip += i;
}

void vm::jmpa(int32_t i, int32_t) {
  *ip = i;
}

void vm::cmpi(int32_t r, int32_t i) {
  if (regs[r] == i) set_bit(flags, ZERO_BIT);
  if (regs[r] > i) reset_bit(flags, SIGN_BIT);
  if (regs[r] < i) set_bit(flags, SIGN_BIT);
  regs[r] -= i;
}

void vm::cmpr(int32_t rx, int32_t ry) {
  if (regs[rx] == regs[ry]) set_bit(flags, ZERO_BIT);
  if (regs[rx] > regs[ry]) reset_bit(flags, SIGN_BIT);
  if (regs[rx] < regs[ry]) set_bit(flags, SIGN_BIT);
  regs[rx] -= regs[ry];
}

void vm::jlt(int32_t r, int32_t) {
  if (get_bit(flags, ZERO_BIT)) {
    *ip += regs[r];
  }
}

void vm::jlti(int32_t i, int32_t) {
  if (get_bit(flags, ZERO_BIT)) {
    *ip += i;
  }
}

void vm::jlta(int32_t i, int32_t) {
  if (get_bit(flags, ZERO_BIT)) {
    *ip = i;
  }
}

void vm::jgt(int32_t r, int32_t) {
  if (!get_bit(flags, ZERO_BIT)) {
    *ip += regs[r];
  }
}

void vm::jgti(int32_t i, int32_t) {
  if (!get_bit(flags, ZERO_BIT)) {
    *ip += i;
  }
}

void vm::jgta(int32_t i, int32_t) {
  if (!get_bit(flags, ZERO_BIT)) {
    *ip = i;
  }
}

void vm::je(int32_t r, int32_t) {  
  if (!get_bit(flags, ZERO_BIT)) {
    *ip += regs[r];
  }
}

void vm::jei(int32_t i, int32_t) {
  if (!get_bit(flags, ZERO_BIT)) {
    *ip += i;
  }
}

void vm::jea(int32_t i, int32_t){
  if (!get_bit(flags, ZERO_BIT)) {
    *ip = i;
  }
}

// TODO: save registers?
void vm::call(int32_t r, int32_t) {
  *sp = *ip;
  *ip += regs[r];
  sp++;
}

// TODO: save registers?
void vm::callm(int32_t r, int32_t) {
  *sp = *ip;
  *ip += mem_at(&mem[regs[r]]);
  sp++;
}

void vm::ret(int32_t, int32_t) {
  *ip = *sp;
  sp--;
}

void vm::exit(int32_t, int32_t) {
  // std::exit(EXIT_SUCCESS);
  procs[*pid].status = pcb::ZOMBIE;
}

void vm::popr(int32_t r, int32_t) {
  regs[r] = *sp;
  sp--;
}

void vm::popm(int32_t r, int32_t) {
  mem_to(&mem[regs[r]], *sp);
  sp--;
}

// TODO: add cycle counting to the vm
void vm::sleep(int32_t r, int32_t) {
  // stub
}

void vm::input(int32_t r, int32_t) {
  std::cin >> regs[r];
}

void vm::inputc(int32_t r, int32_t) {
  std::cin >> regs[r];
  regs[r] += '0';
}

// TODO: add processes to vm
void vm::setPriority(int32_t r, int32_t) {
  // stub
}

// TODO: add processes to vm
void vm::setPriorityl(int32_t i, int32_t) {
  // stub
}