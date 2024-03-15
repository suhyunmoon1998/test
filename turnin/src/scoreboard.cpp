#include <iostream>
#include <assert.h>
#include <util.h>
#include "types.h"
#include "scoreboard.h"
#include "core.h"
#include "debug.h"


using namespace tinyrv;

Scoreboard::Scoreboard(Core* core, uint32_t num_RSs, uint32_t rob_size) 
  : core_(core)  
  , RAT_(NUM_REGS)
  , RS_(num_RSs)
  , RST_(rob_size, -1) {
  // create the ROB
  ROB_ = ReorderBuffer::Create(this, ROB_SIZE);
}

Scoreboard::~Scoreboard() {
  //--
}

bool Scoreboard::issue(pipeline_trace_t* trace) {
  auto& ROB = ROB_;
  auto& RAT = RAT_;
  
  // check for structural hazards return false if found
  if (RS_.is_full()) {
    return false;
  }

  // load renamed operands (rob1_index, rob2_index) from RAT
  int rob1_index = RAT.get(trace->rs1);
  int rob2_index = RAT.get(trace->rs2);

  // for each non-available operands (value == -1), obtain their producing RS indices (rs1_index, rs2_index) from the RST
  int rs1_index = (rob1_index != -1) ? RST_[rob1_index] : -1;
  int rs2_index = (rob2_index != -1) ? RST_[rob2_index] : -1;

  // allocate new ROB entry
  int rob_index = ROB_->allocate(trace);

  // update the RAT if instruction is writing to the register file
  if (trace->wb) {
    RAT.set(trace->rd, rob_index);
  }

  // push trace to RS and obtain index
  int rs_index = RS_.push(trace, rob_index, rs1_index, rs2_index);

  // update the RST with newly allocated RS index
  RST_[rob_index] = rs_index;

  return true;
}
std::vector<pipeline_trace_t*> Scoreboard::execute() {
  std::vector<pipeline_trace_t*> traces;
  auto& FUs = core_->FUs_;

  // search the RS for any valid and not yet running entry
  // that is ready (i.e. both rs1_index and rs2_index are -1)
  // send it to its corresponding FUs
  // mark it as running
  // add its trace to return list
  for (int i = 0; i < (int)RS_.size(); ++i) { 
    auto& rs_entry = RS_[i];
    if (!rs_entry.valid || rs_entry.running)
      continue;

    if (rs_entry.rs1_index == -1 && rs_entry.rs2_index == -1) {
      rs_entry.running = true;
      traces.push_back(rs_entry.trace);
    }
  }

  return traces;
}

pipeline_trace_t* Scoreboard::writeback() {
  pipeline_trace_t* trace = nullptr;
  auto& ROB = ROB_;
  auto& FUs = core_->FUs_;

  // process the first FU to have completed execution by accessing its output
  for (auto& fu : FUs) {
    if (fu->Output.empty())
      continue;

    auto& fu_entry = fu->Output.front();

    // broadcast result to all RS pending for this FU's rs_index
    // invalidate matching rs_index by setting it to -1 to imply that the operand value is now available
    for (uint32_t i = 0; i < RS_.size(); ++i) { 
      auto& rs_entry = RS_[i];
      if (!rs_entry.valid)
        continue;

      if (rs_entry.rs1_index == fu_entry.rs_index) {
        rs_entry.rs1_index = -1;
      }
      if (rs_entry.rs2_index == fu_entry.rs_index) {
        rs_entry.rs2_index = -1;
      }
    }
    
    // clear RST by invalidating current ROB entry to -1
    RST_[fu_entry.rob_index] = -1;
        
    // notify the ROB about completion (using ROB->Completed.send())
    ROB_->Completed.send(fu_entry.rob_index);
    
    // Reset the entry in the reservation station (if needed)
    // RS_[fu_entry.rs_index].reset();
        
    // set the returned trace
    trace = fu_entry.trace;

    // remove FU entry
    fu->Output.pop();

    // we process one FU at the time
    break;
  }

  return trace;
}


pipeline_trace_t* Scoreboard::commit() {
  pipeline_trace_t* trace = nullptr;
  if (!ROB_->Committed.empty()) {
    trace = ROB_->Committed.front();
    ROB_->Committed.pop();
  }
  return trace;
}

void Scoreboard::dump() {
  RS_.dump();
  ROB_->dump();
}
