// Copyright 2024 Blaise Tine
// 
// Licensed under the Apache License;
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <util.h>
#include "debug.h"
#include "types.h"
#include "emulator.h"
#include "trace.h"
#include "instr.h"
#include "core.h"

using namespace tinyrv;

Emulator::Emulator(Core* core) 
  : core_(core)
  , reg_file_(NUM_REGS) {
    this->clear();
}

Emulator::~Emulator() {
  this->cout_flush();
}

void Emulator::clear() {
  PC_ = STARTUP_ADDR;
  csrs_.clear();
  cout_buf_.clear();
  uui_gen_.reset();
  perf_stats_ = PerfStats();  
  exited_ = false;
}

void Emulator::attach_ram(RAM* ram) {
  mmu_.attach(*ram, 0, 0xFFFFFFFF);
}

pipeline_trace_t* Emulator::step() {
#ifndef NDEBUG
  uint32_t uuid = uui_gen_.get_uuid(PC_);
#else
  uint64_t uuid = 0;
#endif
  
  DPH(1, "Fetch: PC=0x" << std::hex << PC_ << " (#" << std::dec << uuid << ")" << std::endl);

  // fetch
  uint32_t instr_code = 0;
  this->icache_read(&instr_code, PC_, sizeof(uint32_t));

  // decode
  auto instr = this->decode(instr_code);
  if (!instr) {
    std::cout << std::hex << "Error: invalid instruction 0x" << instr_code << ", at PC=0x" << PC_ << " (#" << std::dec << uuid << ")" << std::endl;
    std::abort();
  }  

  DP(1, "Instr 0x" << std::hex << instr_code << ": " << *instr);

  // create a new instruction trace
  auto trace = new pipeline_trace_t(uuid, PC_);
    
  // execute
  this->execute(*instr, trace);

  DP(5, "Register File:");
  for (uint32_t i = 0; i < NUM_REGS; ++i) {
    DPN(5, "  x" << std::setfill('0') << std::setw(2) << std::dec << i << "=");
    DPN(5, "0x" << std::setfill('0') << std::setw(XLEN/4) << std::hex << reg_file_.at(i) << std::setfill(' ') << std::endl);
  }  

  return trace;
}

void Emulator::trigger_ecall() {
  exited_ = true;
}

void Emulator::trigger_ebreak() {
  exited_ = true;
}

bool Emulator::check_exit(Word* exitcode, bool riscv_test) const {
  if (exited_) {
    Word ec = reg_file_.at(3);
    if (riscv_test) {
      *exitcode = (1 - ec);
    } else {
      *exitcode = ec;
    }
    return true;
  }
  return false;
}

void Emulator::icache_read(void *data, uint64_t addr, uint32_t size) {
  mmu_.read(data, addr, size, 0);
}

void Emulator::dcache_read(void *data, uint64_t addr, uint32_t size) {  
  auto type = get_addr_type(addr);
  __unused (type);
  mmu_.read(data, addr, size, 0);
  DPH(2, "Mem Read: addr=0x" << std::hex << addr << ", data=0x" << ByteStream(data, size) << " (size=" << size << ", type=" << type << ")" << std::endl);
}

void Emulator::dcache_write(const void* data, uint64_t addr, uint32_t size) {  
  auto type = get_addr_type(addr);
  __unused (type);
  if (addr >= uint64_t(IO_COUT_ADDR)
   && addr < (uint64_t(IO_COUT_ADDR) + IO_COUT_SIZE)) {
     this->writeToStdOut(data);
  } else {
    mmu_.write(data, addr, size, 0);
  }
  DPH(2, "Mem Write: addr=0x" << std::hex << addr << ", data=0x" << ByteStream(data, size) << " (size=" << size << ", type=" << type << ")" << std::endl);  
}

void Emulator::writeToStdOut(const void* data) {
  char c = *(char*)data;
  cout_buf_ << c;
  if (c == '\n') {
    std::cout << cout_buf_.str() << std::flush;
    cout_buf_.str("");
  }
}

void Emulator::cout_flush() {
  auto str = cout_buf_.str();
  if (!str.empty()) {
    std::cout << str << std::endl;
  }
}

uint32_t Emulator::get_csr(uint32_t addr) {
  switch (addr) {
  case VX_CSR_MHARTID:
  case VX_CSR_SATP:
  case VX_CSR_PMPCFG0:
  case VX_CSR_PMPADDR0:
  case VX_CSR_MSTATUS:
  case VX_CSR_MISA:
  case VX_CSR_MEDELEG:
  case VX_CSR_MIDELEG:
  case VX_CSR_MIE:
  case VX_CSR_MTVEC:
  case VX_CSR_MEPC:
  case VX_CSR_MNSTATUS:
    return 0;    
  case VX_CSR_MCYCLE: // NumCycles
    return core_->perf_stats_.cycles & 0xffffffff;
  case VX_CSR_MCYCLE_H: // NumCycles
    return (uint32_t)(core_->perf_stats_.cycles >> 32);
  case VX_CSR_MINSTRET: // NumInsts
    return core_->perf_stats_.instrs & 0xffffffff;
  case VX_CSR_MINSTRET_H: // NumInsts
    return (uint32_t)(core_->perf_stats_.instrs >> 32);
  default:
    std::cout << std::hex << "Error: invalid CSR read addr=0x" << addr << std::endl;
    std::abort();
    return 0;
  }  
}

void Emulator::set_csr(uint32_t addr, uint32_t value) {
  switch (addr) {
  case VX_CSR_SATP:
  case VX_CSR_MSTATUS:
  case VX_CSR_MEDELEG:
  case VX_CSR_MIDELEG:
  case VX_CSR_MIE:
  case VX_CSR_MTVEC:
  case VX_CSR_MEPC:
  case VX_CSR_PMPCFG0:
  case VX_CSR_PMPADDR0:
  case VX_CSR_MNSTATUS:
    break;
  default:
    {
      std::cout << std::hex << "Error: invalid CSR write addr=0x" << addr << ", value=0x" << value << std::endl;
      std::abort();
    }
  }
}