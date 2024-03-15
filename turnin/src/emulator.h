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

#include <vector>
#include <memory>
#include <sstream>
#include <mem.h>
#include "types.h"

namespace tinyrv {

class Instr;
class pipeline_trace_t;
class Core;

class Emulator {
public:
  struct PerfStats {
    uint64_t loads;
    uint64_t stores;
    PerfStats() 
      : loads(0)
      , stores(0)
    {}
  };

  Emulator(Core* core);
  ~Emulator();

  void clear();

  void write_dcr(uint32_t addr, uint32_t value);

  void attach_ram(RAM* ram);

  pipeline_trace_t* step();

  bool check_exit(Word* exitcode, bool riscv_test) const;

private:

  std::shared_ptr<Instr> decode(uint32_t code) const;

  pipeline_trace_t* execute(const Instr &instr);

  void execute(const Instr &instr, pipeline_trace_t *trace);

  void icache_read(void* data, uint64_t addr, uint32_t size);

  void dcache_read(void* data, uint64_t addr, uint32_t size);

  void dcache_write(const void* data, uint64_t addr, uint32_t size);

  uint32_t get_csr(uint32_t addr);
  
  void set_csr(uint32_t addr, uint32_t value);

  void trigger_ecall();

  void trigger_ebreak();
  
  void writeToStdOut(const void* data);

  void cout_flush();
  
  Core* core_;

  std::vector<Word> reg_file_;
  MemoryUnit mmu_;
  CSRs csrs_;
  Word PC_;
  
  std::stringstream cout_buf_;
  
  UUIDGenerator uui_gen_;

  bool exited_;

  PerfStats perf_stats_;
};

}