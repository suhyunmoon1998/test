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

#include <simobject.h>

namespace tinyrv {

class Core;

class FunctionalUnit : public SimObject<FunctionalUnit> {
public:
  struct entry_t {
    pipeline_trace_t* trace;
    int rob_index;
    int rs_index;
  };

  SimPort<entry_t> Input;
  SimPort<entry_t> Output;

  FunctionalUnit(const SimContext& ctx, uint32_t latency);

  ~FunctionalUnit();

  void reset();

  void tick();

private:

  uint32_t latency_;
};

}