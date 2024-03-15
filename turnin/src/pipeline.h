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
#include "trace.h"

namespace tinyrv {

class PipelineLatch {
public:
  PipelineLatch() {}
  ~PipelineLatch() {}
  
  bool empty() const {
    return queue_.empty();
  }

  pipeline_trace_t* front() {
    return queue_.front();
  }

  pipeline_trace_t* back() {
    return queue_.back();
  }

  void push(pipeline_trace_t* value) {    
    queue_.push(value);
  }

  void pop() {
    queue_.pop();
  }

  void clear() {
    std::queue<pipeline_trace_t*> empty;
    std::swap(queue_, empty );
  }

protected:
  std::queue<pipeline_trace_t*> queue_;
};

///////////////////////////////////////////////////////////////////////////////

class Pipeline {
public:
  Pipeline() {}
  virtual ~Pipeline() {}

  virtual bool issue(pipeline_trace_t* trace) = 0;

  virtual std::vector<pipeline_trace_t*> execute() = 0;

  virtual pipeline_trace_t* writeback() = 0;

  virtual pipeline_trace_t* commit() = 0;

  virtual void dump() = 0;
};

}