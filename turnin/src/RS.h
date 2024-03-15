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

#include <vector>

namespace tinyrv {

class ReservationStation {
public:

  struct entry_t {
    bool valid;    // valid entry 
    bool running;  // has been assigned an FU 
    int rob_index; // allocated ROB index
    int rs1_index; // RS producing rs1 (-1 indicates rs1 is already available)
    int rs2_index; // RS producing rs2 (-1 indicates rs2 is already available)
    pipeline_trace_t* trace;    
  };

  ReservationStation(uint32_t size) 
    : store_(size)
    , indices_(size)
    , next_index_(0) {
    for (uint32_t i = 0; i < size; ++i) {
      store_[i].valid = false;
      indices_[i] = i;
    }
  }

  ~ReservationStation() {}

  int push(pipeline_trace_t* trace, int rob_index, int rs1_index, int rs2_index) {
    assert(!this->is_full());
    int index = indices_[next_index_++];
    store_[index] = {true, false, rob_index, rs1_index, rs2_index, trace};
    return index;
  }

  void remove(uint32_t index) {
    assert(index < store_.size() && !this->is_empty());
    store_[index].valid = false;
    indices_[--next_index_] = index;    
  }

  entry_t& operator[](uint32_t index) {
    return store_[index];
  }

  const entry_t& operator[](uint32_t index) const {
    return store_[index];
  }

  bool is_full() const {
    return (next_index_ == store_.size());
  }

  bool is_empty() const {
    return (next_index_ == 0);
  }

  uint32_t size() const {
    return store_.size();
  }

  void dump() {
    for (uint32_t i = 0; i < store_.size(); ++i) {
      auto& entry = store_[i];
      if (entry.valid) {
        DT(4, "RS[" << i << "] rob=" << entry.rob_index << ", running=" << entry.running << ", rs1=" << entry.rs1_index << ", rs2=" << entry.rs2_index << ", trace=" << *entry.trace);
      }
    }
  }

private:

  std::vector<entry_t> store_;
  std::vector<uint32_t> indices_;
  uint32_t next_index_;
};

}