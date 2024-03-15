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

#pragma once

#include <memory>
#include <iostream>
#include <util.h>
#include "types.h"
#include "debug.h"

namespace tinyrv {

class ITraceData {
public:
    using Ptr = std::shared_ptr<ITraceData>;
    ITraceData() {}
    virtual ~ITraceData() {}
};

struct LsuTraceData : public ITraceData {
  using Ptr = std::shared_ptr<LsuTraceData>;
  mem_addr_size_t mem_addrs;
};

struct pipeline_trace_t {
public:
  // instruction idertifier
  const uint64_t uuid;
  
  // program counter
  Word        PC; 

  // destination register
  uint32_t    rd;   

  // first source register
  uint32_t    rs1;
  
  // second source register
  uint32_t    rs2;

  // writeback enable
  bool        wb; 

  // functional unit type
  FUType     fu_type;

  // functional unit operation
  union {    
    LsuOp   slu_op;
    AluOp   alu_op;
    CSROp   csr_op;
    uint32_t fu_op;
  };

  // additional trace data
  ITraceData::Ptr data;

  pipeline_trace_t(uint64_t uuid, Word PC) 
    : uuid(uuid)
    , PC(PC)    
    , rd(0)
    , rs1(0)
    , rs2(0)
    , wb(false)
    , fu_type(FUType::ALU)
    , fu_op(0)
    , data(nullptr)
  {}

  pipeline_trace_t(const pipeline_trace_t& rhs) 
    : uuid(rhs.uuid)
    , PC(rhs.PC)    
    , rd(rhs.rd)    
    , rs1(rhs.rs1)
    , rs2(rhs.rs2)
    , wb(rhs.wb)
    , fu_type(rhs.fu_type)
    , fu_op(rhs.fu_op)
    , data(rhs.data)
  {}
  
  ~pipeline_trace_t() {}
};

inline std::ostream &operator<<(std::ostream &os, const pipeline_trace_t& state) {
  os << "PC=0x" << std::hex << state.PC;
  os << ", wb=" << state.wb;
  if (state.wb) {
     os << ", rd=x" << std::dec << state.rd;
  }
  os << ", ex=" << state.fu_type;
  os << " (#" << std::dec << state.uuid << ")";
  return os;
}

}