#ifndef VM_H
#define VM_H

#include "mmu.h"
#include "pcb.h"

#include <cstdint>

class vm {
public:
  mmu_t mmu;
  uint32_t flags = 0;
  int32_t regs[14] = {0};
  uint8_t mem[4096] = {0};
  int32_t *ip = &regs[11];
  int32_t *pid = &regs[12]; 
  int32_t *sp = &regs[13];

  using ops_t = std::vector<std::tuple<std::string, int32_t, int32_t, int32_t>>;
  using progs_t = std::vector<ops_t>;
  using procs_t = std::vector<pcb>;

  progs_t progs;
  procs_t procs;
  
  ops_t parse_file(std::string& filepath);

  vm() = delete;
  vm(std::string, mmu_t m);
  vm(std::vector<std::string>& prog_names, mmu_t m);

  using op_t = void (vm::*)(int32_t, int32_t);

  const char *op_lookup[39] = {
    "incr\0",
    "addi\0",
    "addr\0",
    "pushr\0",
    "pushi\0",
    "movi\0",
    "movr\0",
    "movmr\0",
    "movrm\0",
    "movmm\0",
    "printr\0",
    "printm\0",
    "printcr\0", 
    "printcm\0",
    "jmp\0",
    "jmpi\0",
    "jmpa\0",
    "cmpi\0",
    "cmpr\0",
    "jlt\0",
    "jlti\0",
    "jlta\0",
    "jgt\0",
    "jgti\0",
    "jgta\0",
    "je\0",
    "jei\0",
    "jea\0",
    "call\0",
    "callm\0",
    "ret\0",
    "exit\0",
    "popr\0",
    "popm\0",
    "sleep\0",
    "input\0",
    "inputc\0",
    "setPriority\0",
    "setPriorityl\0",
  };
  int32_t get_op_code(char *op);
  bool round_robin_pass();
  
  void incr(int32_t reg, int32_t);
  void addi(int32_t reg, int32_t imm);
  void addr(int32_t rx, int32_t ry);
  void pushr(int32_t reg, int32_t);
  void pushi(int32_t imm, int32_t);
  void movi(int32_t reg, int32_t imm);
  void movr(int32_t rx, int32_t ry);
  void movmr(int32_t offset, int32_t reg);
  void movrm(int32_t reg, int32_t offset);
  void movmm(int32_t mx, int32_t my);
  void printr(int32_t reg, int32_t);
  void printm(int32_t reg, int32_t);
  void printcr(int32_t reg, int32_t);
  void printcm(int32_t reg, int32_t);
  void jmp(int32_t reg, int32_t);
  void jmpi(int32_t imm, int32_t);
  void jmpa(int32_t imm, int32_t);
  void cmpi(int32_t reg, int32_t imm);
  void cmpr(int32_t rx, int32_t ry);
  void jlt(int32_t reg, int32_t);
  void jlti(int32_t imm, int32_t);
  void jlta(int32_t imm, int32_t);
  void jgt(int32_t reg, int32_t);
  void jgti(int32_t reg, int32_t);
  void jgta(int32_t reg, int32_t);
  void je(int32_t reg, int32_t);
  void jei(int32_t imm, int32_t);
  void jea(int32_t imm, int32_t);
  void call(int32_t reg, int32_t);
  void callm(int32_t reg, int32_t);
  void ret(int32_t , int32_t);
  void exit(int32_t, int32_t);
  void popr(int32_t reg, int32_t);
  void popm(int32_t reg, int32_t);
  void sleep(int32_t reg, int32_t);
  void input(int32_t reg, int32_t);
  void inputc(int32_t reg, int32_t);
  void setPriority(int32_t reg, int32_t);
  void setPriorityl(int32_t imm, int32_t);

  op_t op_table[39] = {
    &vm::incr,
    &vm::addi,
    &vm::addr,
    &vm::pushr,
    &vm::pushi,
    &vm::movi,
    &vm::movr,
    &vm::movmr,
    &vm::movrm,
    &vm::movmm,
    &vm::printr, 
    &vm::printm,
    &vm::printcr,
    &vm::printcm,
    &vm::jmp,
    &vm::jmpi,
    &vm::jmpa,
    &vm::cmpi,
    &vm::cmpr,
    &vm::jlt,
    &vm::jlti,
    &vm::jlta,
    &vm::jgt,
    &vm::jgti,
    &vm::jgta,
    &vm::je,
    &vm::jei,
    &vm::jea,
    &vm::call,
    &vm::callm,
    &vm::ret,
    &vm::exit,
    &vm::popr,
    &vm::popm,
    &vm::sleep,
    &vm::input,
    &vm::inputc,
    &vm::setPriority,
    &vm::setPriorityl,
  };
};

#endif
