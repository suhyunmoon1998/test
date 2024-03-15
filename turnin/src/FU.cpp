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

#include <iostream>
#include <assert.h>
#include <util.h>
#include "types.h"
#include "core.h"
#include "debug.h"

using namespace tinyrv;

FunctionalUnit::FunctionalUnit(const SimContext& ctx, uint32_t latency)
  : SimObject<FunctionalUnit>(ctx, "FunctionalUnit")
  , Input(this)
  , Output(this)
  , latency_(latency) {
  //--
}

FunctionalUnit::~FunctionalUnit() {
  //--
}

void FunctionalUnit::reset() {
  //--
}

void FunctionalUnit::tick() {
  if (Input.empty())
    return;
  auto trace = Input.front();
  Output.send(trace, latency_);
  Input.pop();
}