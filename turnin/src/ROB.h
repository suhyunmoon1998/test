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
#include <simobject.h>

namespace tinyrv {

class Scoreboard;

class ReorderBuffer : public SimObject<ReorderBuffer> {
public:
  
  SimPort<int> Completed;
  SimPort<pipeline_trace_t*> Committed;

  ReorderBuffer(const SimContext& ctx, Scoreboard* scoreboard, uint32_t size);

  ~ReorderBuffer();

  void reset();

  void tick();

  int allocate(pipeline_trace_t* trace);

  int pop();

  bool is_full() const;

  bool is_empty() const;

  void dump();

private:

  struct rob_entry_t {
    pipeline_trace_t* trace;
    bool completed;
  };
  
  Scoreboard* scoreboard_;
  std::vector<rob_entry_t> store_;
  int head_index_;
  int tail_index_;
  uint32_t count_;  
};

}