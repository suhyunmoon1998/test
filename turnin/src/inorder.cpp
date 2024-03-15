// Copyright 2024 blaise
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "inorder.h"
#include "core.h"

using namespace tinyrv;

InorderPipeline::InorderPipeline(Core* core) 
  : core_(core) {
  //--
}

InorderPipeline::~InorderPipeline() {
  //--
}

bool InorderPipeline::issue(pipeline_trace_t* trace) {
  // check RAW and WAW data dependencies
  if (trace->rs1 != 0 && inuse_.test(trace->rs1))
    return false;
  if (trace->rs2 != 0 && inuse_.test(trace->rs2))
    return false;
  if (trace->rd != 0 && inuse_.test(trace->rd))
    return false;
  
  // mark destination register as in use
  if (trace->rd != 0) {
    inuse_.set(trace->rd);
  }

  issue_latch_.push(trace);

  return true;
}

std::vector<pipeline_trace_t*> InorderPipeline::execute() {
  std::vector<pipeline_trace_t*> traces;
  auto& FUs = core_->FUs_;

  if (!issue_latch_.empty()) {
    auto trace = issue_latch_.front();    
    FUs.at((int)trace->fu_type)->Input.send({trace, 0, 0});  
    traces.push_back(trace);
    issue_latch_.pop();
  }
  
  return traces;
}

pipeline_trace_t* InorderPipeline::writeback() {
  pipeline_trace_t* trace = nullptr;
  auto& FUs = core_->FUs_;

  for (auto& fu : FUs) {
    if (fu->Output.empty())
      continue;
    auto& fu_entry = fu->Output.front();
    trace = fu_entry.trace;
    // clear destination register use
    if (trace->rd != 0) {
      inuse_.reset(trace->rd);
    }
    wb_latch_.push(trace);
    fu->Output.pop();
    // we process one FU at the time
    break;
  }

  return trace;
}

pipeline_trace_t* InorderPipeline::commit() {
  pipeline_trace_t* trace = nullptr;

  if (!wb_latch_.empty()) {
    trace = wb_latch_.front();
    wb_latch_.pop();
  }

  return trace;
}

void InorderPipeline::dump() {
  //--
}