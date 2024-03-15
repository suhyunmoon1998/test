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

#include "pipeline.h"
#include "RAT.h"
#include "RS.h"
#include "ROB.h"

namespace tinyrv {

class Core;
struct pipeline_trace_t;

// register status table 
// track the mapping from ROB index and RS index
typedef std::vector<int> RegisterStatusTable;

class Scoreboard : public Pipeline {
public:
  Scoreboard(Core* core, uint32_t num_RSs, uint32_t rob_size);

  ~Scoreboard();

  bool issue(pipeline_trace_t* trace) override;

  std::vector<pipeline_trace_t*> execute() override;

  pipeline_trace_t* writeback() override;

  pipeline_trace_t* commit() override;

  void dump() override;

private:

  Core* core_;
  
  RegisterAliasTable RAT_;  
  ReservationStation RS_;
  RegisterStatusTable RST_;  
  ReorderBuffer::Ptr ROB_;
  
  friend class ReorderBuffer;
};

}