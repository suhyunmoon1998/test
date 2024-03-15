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

#include <stdint.h>
#include <bitset>
#include <queue>
#include <unordered_map>
#include <stringutil.h>
#include <simobject.h>
#include <util.h>
#include "uuid_gen.h"
#include "debug.h"
#include "config.h"

namespace tinyrv {

typedef uint8_t  Byte;
typedef uint32_t Word;
typedef int32_t  WordI;

typedef std::bitset<NUM_REGS> RegMask;

typedef std::unordered_map<uint32_t, uint32_t> CSRs;

///////////////////////////////////////////////////////////////////////////////

enum class RegType {
  None,
  Integer,
  Float
};

inline std::ostream &operator<<(std::ostream &os, const RegType& type) {
  switch (type) {
  case RegType::None: break;
  case RegType::Integer: os << "x"; break;  
  case RegType::Float:   os << "f"; break;
  default: assert(false);
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class FUType {
  ALU,
  LSU,
  CSR,
};

inline std::ostream &operator<<(std::ostream &os, const FUType& type) {
  switch (type) {
  case FUType::ALU: os << "ALU"; break;
  case FUType::LSU: os << "LSU"; break;
  case FUType::CSR: os << "CSR"; break;
  default: assert(false);
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class AluOp {
  ARITH,
  BRANCH,
  SYSCALL,
  IMUL,
  IDIV
};

inline std::ostream &operator<<(std::ostream &os, const AluOp& type) {
  switch (type) {
  case AluOp::ARITH:   os << "ARITH"; break;
  case AluOp::BRANCH:  os << "BRANCH"; break;
  case AluOp::SYSCALL: os << "SYSCALL"; break;
  case AluOp::IMUL:    os << "IMUL"; break;
  case AluOp::IDIV:    os << "IDIV"; break;
  default: assert(false);
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class LsuOp {
  LOAD,
  STORE,
  FENCE
};

inline std::ostream &operator<<(std::ostream &os, const LsuOp& type) {
  switch (type) {
  case LsuOp::LOAD:  os << "LOAD"; break;
  case LsuOp::STORE: os << "STORE"; break;
  default: assert(false);
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class AddrType {
  Global,
  IO
};

inline std::ostream &operator<<(std::ostream &os, const AddrType& type) {
  switch (type) {
  case AddrType::Global: os << "Global"; break;
  case AddrType::IO:     os << "IO"; break;
  default: assert(false);
  }
  return os;
}

inline AddrType get_addr_type(uint64_t addr) {
  if (addr >= IO_BASE_ADDR) {
     return AddrType::IO;
  }
  return AddrType::Global;
}

///////////////////////////////////////////////////////////////////////////////

struct mem_addr_size_t {
  uint64_t addr;
  uint32_t size;
};

///////////////////////////////////////////////////////////////////////////////

enum class CSROp {
  CSRRW,
  CSRRS,
  CSRRC
};

inline std::ostream &operator<<(std::ostream &os, const CSROp& type) {
  switch (type) {
  case CSROp::CSRRW: os << "CSRRW"; break;
  case CSROp::CSRRS: os << "CSRRS"; break;
  case CSROp::CSRRC: os << "CSRRC"; break;
  default: assert(false);
  }
  return os;
}

}
