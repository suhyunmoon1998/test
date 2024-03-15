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
#include "instr.h"

using namespace tinyrv;

namespace tinyrv {

static const std::unordered_map<Opcode, InstType> sc_instTable = {
  {Opcode::R,     InstType::R},
  {Opcode::L,     InstType::I},
  {Opcode::I,     InstType::I},
  {Opcode::S,     InstType::S},
  {Opcode::B,     InstType::B},
  {Opcode::LUI,   InstType::U},
  {Opcode::AUIPC, InstType::U},
  {Opcode::JAL,   InstType::J},
  {Opcode::JALR,  InstType::I},
  {Opcode::SYS,   InstType::I},
  {Opcode::FENCE, InstType::I},
};

enum Constants {
  width_opcode= 7,
  width_reg   = 5,
  width_func3 = 3,
  width_func7 = 7,
  width_i_imm = 12,
  width_j_imm = 20,

  shift_opcode= 0,
  shift_rd    = width_opcode,
  shift_func3 = shift_rd + width_reg,
  shift_rs1   = shift_func3 + width_func3,
  shift_rs2   = shift_rs1 + width_reg,
  shift_func2 = shift_rs2 + width_reg,
  shift_func7 = shift_rs2 + width_reg,

  mask_opcode = (1 << width_opcode)- 1,  
  mask_reg    = (1 << width_reg)   - 1,
  mask_func3  = (1 << width_func3) - 1,
  mask_func7  = (1 << width_func7) - 1,
  mask_i_imm  = (1 << width_i_imm) - 1,
  mask_j_imm  = (1 << width_j_imm) - 1,
};

static const char* op_string(const Instr &instr) {
  auto opcode = instr.getOpcode();
  auto func3  = instr.getFunc3();
  auto func7  = instr.getFunc7();
  auto imm    = instr.getImm();

  switch (opcode) {
  case Opcode::LUI:   return "LUI";
  case Opcode::AUIPC: return "AUIPC";
  case Opcode::R:
    if (func7 & 0x1) {
      switch (func3) {
      case 0: return "MUL";
      case 1: return "MULH";
      case 2: return "MULHSU";
      case 3: return "MULHU";
      case 4: return "DIV";
      case 5: return "DIVU";
      case 6: return "REM";
      case 7: return "REMU";
      default:
        std::abort();
      }
    } else {
      switch (func3) {
      case 0: return func7 ? "SUB" : "ADD";
      case 1: return "SLL";
      case 2: return "SLT";
      case 3: return "SLTU";
      case 4: return "XOR";
      case 5: return (func7 & 0x20) ? "SRA" : "SRL";
      case 6: return "OR";
      case 7: return "AND";
      default:
        std::abort();
      }
    }
  case Opcode::I:
    switch (func3) {
    case 0: return "ADDI";
    case 1: return "SLLI";
    case 2: return "SLTI";
    case 3: return "SLTIU";
    case 4: return "XORI";
    case 5: return (func7 & 0x20) ? "SRAI" : "SRLI";
    case 6: return "ORI";
    case 7: return "ANDI";
    default:
      std::abort();
    }  
  case Opcode::B:
    switch (func3) {
    case 0: return "BEQ";
    case 1: return "BNE";
    case 4: return "BLT";
    case 5: return "BGE";
    case 6: return "BLTU";
    case 7: return "BGEU";
    default:
      std::abort();
    }
  case Opcode::JAL:   return "JAL";
  case Opcode::JALR:  return "JALR";
  case Opcode::L:
    switch (func3) {
    case 0: return "LB";
    case 1: return "LH";
    case 2: return "LW";
    case 3: return "LD";
    case 4: return "LBU";
    case 5: return "LHU";
    case 6: return "LWU";
    default:
      std::abort();
    }
  case Opcode::S:
    switch (func3) {
    case 0: return "SB";
    case 1: return "SH";
    case 2: return "SW";
    case 3: return "SD";
    default:
      std::abort();
    }
  case Opcode::SYS: 
    switch (func3) {
    case 0:
      switch (imm) {
      case 0x000: return "ECALL";
      case 0x001: return "EBREAK";
      case 0x002: return "URET";
      case 0x102: return "SRET";
      case 0x302: return "MRET";
      default:
        std::abort();      
      }
    case 1: return "CSRRW";
    case 2: return "CSRRS";
    case 3: return "CSRRC";
    case 5: return "CSRRWI";
    case 6: return "CSRRSI";
    case 7: return "CSRRCI";
    default:
      std::abort();
    }
  case Opcode::FENCE: return "FENCE";
  default:
    std::abort();
  }
}

std::ostream &operator<<(std::ostream &os, const Instr &instr) {
  os << op_string(instr);  
  int sep = 0;
  if (instr.getRDType() != RegType::None) {
    if (sep++ != 0) { os << ", "; } else { os << " "; }
    os << instr.getRDType() << std::dec << instr.getRDest();
  }
  for (uint32_t i = 0; i < instr.getNRSrc(); ++i) {    
    if (instr.getRSType(i) == RegType::None)
      continue;
    if (sep++ != 0) { os << ", "; } else { os << " "; }
    if (instr.getRSType(i) != RegType::None) {   
      os << instr.getRSType(i) << std::dec << instr.getRSrc(i);
    } else {
      os << "0x" << std::hex << instr.getRSrc(0);
    }
  }
  if (instr.hasImm()) {
    if (sep++ != 0) { os << ", "; } else { os << " "; }
    os << "0x" << std::hex << instr.getImm();
  }
  return os;
}

}

std::shared_ptr<Instr> Emulator::decode(uint32_t code) const {  
  auto instr = std::make_shared<Instr>();
  auto op = Opcode((code >> shift_opcode) & mask_opcode);
  instr->setOpcode(op);

  auto func3 = (code >> shift_func3) & mask_func3;
  auto func7 = (code >> shift_func7) & mask_func7;

  auto rd  = (code >> shift_rd)  & mask_reg;
  auto rs1 = (code >> shift_rs1) & mask_reg;
  auto rs2 = (code >> shift_rs2) & mask_reg;

  auto op_it = sc_instTable.find(op);
  if (op_it == sc_instTable.end()) {
    std::cout << std::hex << "Error: invalid opcode: 0x" << static_cast<int>(op) << std::endl;
    return nullptr;
  }

  auto iType = op_it->second;
  switch (iType) {
  case InstType::R:
    switch (op) {
    default:
      instr->setDestReg(rd, RegType::Integer);
      instr->addSrcReg(rs1, RegType::Integer);
      instr->addSrcReg(rs2, RegType::Integer);
      break;
    }
    instr->setFunc3(func3);
    instr->setFunc7(func7);
    break;

  case InstType::I: {
    switch (op) {
    case Opcode::I:
      instr->setDestReg(rd, RegType::Integer);      
      instr->addSrcReg(rs1, RegType::Integer);
      instr->setFunc3(func3);
      if (func3 == 0x1 || func3 == 0x5) {
        // Shift instructions
        instr->setImm(rs2);
        instr->setFunc7(func7);
      } else {
        auto imm = code >> shift_rs2;
        instr->setImm(sext(imm, width_i_imm));
      }
      break;    
    case Opcode::L:
    case Opcode::JALR: {
      instr->setDestReg(rd, RegType::Integer);
      instr->addSrcReg(rs1, RegType::Integer);
      instr->setFunc3(func3);
      auto imm = code >> shift_rs2;
      instr->setImm(sext(imm, width_i_imm));
    } break;
    case Opcode::FENCE:
      instr->setFunc3(func3);
      instr->setImm(code >> shift_rs2);
      break;
    case Opcode::SYS:
      if (func3 != 0) {
        // CSR instructions
        instr->setDestReg(rd, RegType::Integer);
        instr->setFunc3(func3);
        if (func3 < 5) { 
          instr->addSrcReg(rs1, RegType::Integer);
        } else {          
          // zimm       
          instr->addSrcReg(rs1, RegType::None);
        }
        instr->setImm(code >> shift_rs2);
      } else {
        // ECALL/EBREACK instructions
        instr->setImm(code >> shift_rs2);
      }     
      break;
    default:
      std::abort();      
      break;
    }
  } break;
  case InstType::S: {    
    instr->addSrcReg(rs1, RegType::Integer);
    instr->addSrcReg(rs2, RegType::Integer);
    instr->setFunc3(func3);
    auto imm = (func7 << width_reg) | rd;
    instr->setImm(sext(imm, width_i_imm));
  } break;

  case InstType::B: {
    instr->addSrcReg(rs1, RegType::Integer);
    instr->addSrcReg(rs2, RegType::Integer);
    instr->setFunc3(func3);
    auto bit_11   = rd & 0x1;
    auto bits_4_1 = rd >> 1;
    auto bit_10_5 = func7 & 0x3f;
    auto bit_12   = func7 >> 6;
    auto imm = (bits_4_1 << 1) | (bit_10_5 << 5) | (bit_11 << 11) | (bit_12 << 12);
    instr->setImm(sext(imm, width_i_imm+1));
  } break;

  case InstType::U: {
    instr->setDestReg(rd, RegType::Integer);
    auto imm = (code >> shift_func3) << shift_func3;
    instr->setImm(imm);
  }  break;

  case InstType::J: {
    instr->setDestReg(rd, RegType::Integer);
    auto unordered  = code >> shift_func3;
    auto bits_19_12 = unordered & 0xff;
    auto bit_11     = (unordered >> 8) & 0x1;
    auto bits_10_1  = (unordered >> 9) & 0x3ff;
    auto bit_20     = (unordered >> 19) & 0x1;
    auto imm = (bits_10_1 << 1) | (bit_11 << 11) | (bits_19_12 << 12) | (bit_20 << 20);
    instr->setImm(sext(imm, width_j_imm+1));
  } break;   

  default:
    std::abort();
  }

  return instr;
}