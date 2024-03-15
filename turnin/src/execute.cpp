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
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <bitset>
#include <climits>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <util.h>
#include "instr.h"
#include "core.h"
#include "trace.h"
#include "emulator.h"

using namespace tinyrv;

union reg_data_t {
  Word     u;
  WordI    i;
  uint32_t u32;
  int32_t  i32;
};

void Emulator::execute(const Instr &instr, pipeline_trace_t *trace) {
  auto next_pc = PC_ + 4;

  auto opcode = instr.getOpcode();
  auto func3  = instr.getFunc3();
  auto func7  = instr.getFunc7();  
  auto rd     = instr.getRDest();
  auto rs1    = instr.getRSrc(0);
  auto rs2    = instr.getRSrc(1);
  auto imm    = sext((Word)instr.getImm(), 32);

  // Register File access
  reg_data_t rsdata [2];
  auto num_rsrcs = instr.getNRSrc();
  if (num_rsrcs) {       
    for (uint32_t i = 0; i < num_rsrcs; ++i) {          
      auto type = instr.getRSType(i);
      auto reg = instr.getRSrc(i);        
      switch (type) {
      case RegType::Integer: 
        rsdata[i].u = reg_file_[reg];
        DPH(2, "Src" << std::dec << i << " Reg: " << type << std::dec << reg << "={0x" << std::hex << rsdata[i].i << "}" << std::endl);
        break;
      case RegType::None:
        break;
      default:
        std::abort();
      }      
    }
  }

  reg_data_t rddata;
  bool rd_write = false;
  
  switch (opcode) {  
  case Opcode::LUI: {
    // RV32I: LUI
    trace->fu_type = FUType::ALU;
    trace->alu_op = AluOp::ARITH;
    rddata.i = imm;
    rd_write = true;
    break;
  }  
  case Opcode::AUIPC: {
    // RV32I: AUIPC
    trace->fu_type = FUType::ALU;
    trace->alu_op = AluOp::ARITH;
    rddata.i = imm + PC_;
    rd_write = true;
    break;
  }
  case Opcode::R: {
    trace->fu_type = FUType::ALU;    
    trace->alu_op = AluOp::ARITH;
    trace->rs1 = rs1;
    trace->rs2 = rs2;
    switch (func3) {
    case 0: {
      if (func7) {
        // RV32I: SUB
        rddata.i = rsdata[0].i - rsdata[1].i;
      } else {
        // RV32I: ADD
        rddata.i = rsdata[0].i + rsdata[1].i;
      }
      break;
    }
    case 1: {
      // RV32I: SLL
      Word shamt_mask = (Word(1) << log2up(XLEN)) - 1;
      Word shamt = rsdata[1].i & shamt_mask;
      rddata.i = rsdata[0].i << shamt;
      break;
    }
    case 2: {
      // RV32I: SLT
      rddata.i = rsdata[0].i < rsdata[1].i;
      break;
    }
    case 3: {
      // RV32I: SLTU
      rddata.i = rsdata[0].u < rsdata[1].u;
      break;
    }
    case 4: {
      // RV32I: XOR
      rddata.i = rsdata[0].i ^ rsdata[1].i;
      break;
    }
    case 5: {
      Word shamt_mask = ((Word)1 << log2up(XLEN)) - 1;
      Word shamt = rsdata[1].i & shamt_mask;
      if (func7) {
        // RV32I: SRA
        rddata.i = rsdata[0].i >> shamt;
      } else {
        // RV32I: SRL
        rddata.i = rsdata[0].u >> shamt;
      }
      break;
    }
    case 6: {
      // RV32I: OR
      rddata.i = rsdata[0].i | rsdata[1].i;
      break;
    }
    case 7: {
      // RV32I: AND
      rddata.i = rsdata[0].i & rsdata[1].i;
      break;
    }
    default:
      std::abort();
    }    
    rd_write = true;
    break;
  }
  case Opcode::I: {
    trace->fu_type = FUType::ALU;    
    trace->alu_op = AluOp::ARITH;    
    trace->rs1 = rs1;
    switch (func3) {
    case 0: {
      // RV32I: ADDI
      rddata.i = rsdata[0].i + imm;
      break;
    }
    case 1: {
      // RV32I: SLLI
      rddata.i = rsdata[0].i << imm;
      break;
    }
    case 2: {
      // RV32I: SLTI
      rddata.i = rsdata[0].i < WordI(imm);
      break;
    }
    case 3: {
      // RV32I: SLTIU
      rddata.i = rsdata[0].u < imm;
      break;
    } 
    case 4: {
      // RV32I: XORI
      rddata.i = rsdata[0].i ^ imm;
      break;
    }
    case 5: {
      if (func7) {
        // RV32I: SRAI
        Word result = rsdata[0].i >> imm;
        rddata.i = result;
      } else {
        // RV32I: SRLI
        Word result = rsdata[0].u >> imm;
        rddata.i = result;
      }
      break;
    }
    case 6: {
      // RV32I: ORI
      rddata.i = rsdata[0].i | imm;
      break;
    }
    case 7: {
      // RV32I: ANDI
      rddata.i = rsdata[0].i & imm;
      break;
    }
    }
    rd_write = true;
    break;
  }
  case Opcode::B: {   
    trace->fu_type = FUType::ALU;    
    trace->alu_op = AluOp::BRANCH;    
    trace->rs1 = rs1;
    trace->rs2 = rs2;
    switch (func3) {
    case 0: {
      // RV32I: BEQ
      if (rsdata[0].i == rsdata[1].i) {
        next_pc = PC_ + imm;
      }
      break;
    }
    case 1: {
      // RV32I: BNE
      if (rsdata[0].i != rsdata[1].i) {
        next_pc = PC_ + imm;
      }
      break;
    }
    case 4: {
      // RV32I: BLT
      if (rsdata[0].i < rsdata[1].i) {
        next_pc = PC_ + imm;
      }
      break;
    }
    case 5: {
      // RV32I: BGE
      if (rsdata[0].i >= rsdata[1].i) {
        next_pc = PC_ + imm;
      }
      break;
    }
    case 6: {
      // RV32I: BLTU
      if (rsdata[0].u < rsdata[1].u) {
        next_pc = PC_ + imm;
      }
      break;
    }
    case 7: {
      // RV32I: BGEU
      if (rsdata[0].u >= rsdata[1].u) {
        next_pc = PC_ + imm;
      }
      break;
    }
    default:
      std::abort();
    }
    break;
  }  
  case Opcode::JAL: {
    // RV32I: JAL
    trace->fu_type = FUType::ALU;    
    trace->alu_op = AluOp::BRANCH;
    rddata.i = next_pc;
    next_pc = PC_ + imm;
    rd_write = true;
    break;
  }  
  case Opcode::JALR: {
    // RV32I: JALR
    trace->fu_type = FUType::ALU;    
    trace->alu_op = AluOp::BRANCH;
    trace->rs1 = rs1;
    rddata.i = next_pc;
    next_pc = rsdata[0].i + imm;
    rd_write = true;
    break;
  }
  case Opcode::L: {
    trace->fu_type = FUType::LSU;    
    trace->slu_op = LsuOp::LOAD;
    trace->rs1 = rs1;
    auto trace_data = std::make_shared<LsuTraceData>();
    trace->data = trace_data;
    uint32_t data_bytes = 1 << (func3 & 0x3);
    uint32_t data_width = 8 * data_bytes;
    uint64_t mem_addr = rsdata[0].i + imm;         
    uint64_t read_data = 0;
    this->dcache_read(&read_data, mem_addr, data_bytes);
    trace_data->mem_addrs = {mem_addr, data_bytes};
    switch (func3) {
    case 0: // RV32I: LB
    case 1: // RV32I: LH
      rddata.i = sext((Word)read_data, data_width);
      break;
    case 2: // RV32I: LW
      rddata.i = sext((Word)read_data, data_width);
      break;
    case 4: // RV32I: LBU
    case 5: // RV32I: LHU
      rddata.u32 = read_data;
      break;
    default:
      std::abort();      
    }
    rd_write = true;
    break;
  }
  case Opcode::S: {
    trace->fu_type = FUType::LSU;    
    trace->slu_op = LsuOp::STORE;
    trace->rs1 = rs1;
    trace->rs2 = rs2;    
    auto trace_data = std::make_shared<LsuTraceData>();
    trace->data = trace_data;
    uint32_t data_bytes = 1 << (func3 & 0x3);
    uint64_t mem_addr = rsdata[0].i + imm;
    uint64_t write_data = rsdata[1].u32;
    trace_data->mem_addrs = {mem_addr, data_bytes};
    switch (func3) {
    case 0:
    case 1:
    case 2:
      this->dcache_write(&write_data, mem_addr, data_bytes);  
      break;
    default:
      std::abort();
    }
    break;
  }
  case Opcode::SYS: {
    uint32_t csr_addr = imm;
    uint32_t csr_value;
    if (func3 == 0) {
      trace->fu_type = FUType::ALU;
      trace->alu_op = AluOp::SYSCALL;
      switch (csr_addr) {
      case 0: 
        // RV32I: ECALL
        this->trigger_ecall();
        break;
      case 1: 
        // RV32I: EBREAK
        this->trigger_ebreak();
        break;
      case 0x002: // URET
      case 0x102: // SRET
      case 0x302: // MRET
        break;
      default:
        std::abort();
      }                
    } else {
      trace->fu_type = FUType::CSR;
      csr_value = this->get_csr(csr_addr);
      switch (func3) {
      case 1: {
        // RV32I: CSRRW
        rddata.i = csr_value;
        this->set_csr(csr_addr, rsdata[0].i);      
        trace->rs1 = rs1;
        trace->csr_op = CSROp::CSRRW;
        rd_write = true;
        break;
      }
      case 2: {
        // RV32I: CSRRS
        rddata.i = csr_value;
        if (rsdata[0].i != 0) {
          this->set_csr(csr_addr, csr_value | rsdata[0].i);
        }
        trace->rs1 = rs1;
        trace->csr_op = CSROp::CSRRS;
        rd_write = true;
        break;
      }
      case 3: {
        // RV32I: CSRRC
        rddata.i = csr_value;
        if (rsdata[0].i != 0) {
          this->set_csr(csr_addr, csr_value & ~rsdata[0].i);
        }
        trace->rs1 = rs1;
        trace->csr_op = CSROp::CSRRC;
        rd_write = true;
        break;
      }
      case 5: {
        // RV32I: CSRRWI
        rddata.i = csr_value;
        this->set_csr(csr_addr, rs1);
        trace->csr_op = CSROp::CSRRW;    
        rd_write = true;
        break;
      }
      case 6: {
        // RV32I: CSRRSI;
        rddata.i = csr_value;
        if (rs1 != 0) {
          this->set_csr(csr_addr, csr_value | rs1);
        }
        trace->csr_op = CSROp::CSRRS;
        rd_write = true;
        break;
      }
      case 7: {
        // RV32I: CSRRCI
        rddata.i = csr_value;
        if (rs1 != 0) {
          this->set_csr(csr_addr, csr_value & ~rs1);
        }
        trace->csr_op = CSROp::CSRRC;
        rd_write = true;
        break;
      }
      default:
        break;
      }
    } 
    break;
  }   
  case Opcode::FENCE: {
    // RV32I: FENCE
    trace->fu_type = FUType::LSU;    
    trace->slu_op = LsuOp::FENCE;
    break;
  }
  default:
    std::abort();
  }

  if (rd_write) {
    trace->wb = true;
    auto type = instr.getRDType();    
    switch (type) {
    case RegType::Integer:      
      if (rd != 0) {
        DPH(2, "Dest Reg: " << type << std::dec << rd << "={0x" << std::hex << rddata.i << "}" << std::endl);    
        reg_file_[rd] = rddata.i;
        trace->rd = rd;
      } else {
        // disable write to x0
        trace->wb = false;
      }
      break;
    default:
      std::abort();
      break;
    }
  }

  PC_ += 4;
  if (PC_ != next_pc) {
    DP(3, "*** Next PC=0x" << std::hex << next_pc << std::dec);
    PC_ = next_pc;
  }
}