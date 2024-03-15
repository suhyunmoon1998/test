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

namespace tinyrv {

struct pipeline_trace_t;
class Core;

class InorderPipeline : public Pipeline {
public:
  InorderPipeline(Core* core);

  ~InorderPipeline();

  bool issue(pipeline_trace_t* trace) override;

  std::vector<pipeline_trace_t*> execute() override;

  pipeline_trace_t* writeback() override;

  pipeline_trace_t* commit() override;

  void dump() override;

private:
  Core*         core_;
  PipelineLatch issue_latch_;
  PipelineLatch wb_latch_;
  RegMask       inuse_;
};  

}