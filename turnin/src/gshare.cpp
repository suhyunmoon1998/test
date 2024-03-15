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

#include <iostream>
#include <assert.h>
#include <util.h>
#include "types.h"
#include "core.h"
#include "gshare.h"



using namespace tinyrv;

#define STRONGLY_NOT_TAKEN 2

GShare::GShare() {
  // Initialize BHR to all zeros
  bhr = 0;

  // Initialize BHT with all entries set to STRONGLY_NOT_TAKEN (2)
  for (int i = 0; i < BHT_SIZE; ++i) {
    bht[i] = STRONGLY_NOT_TAKEN;
  }
}

GShare::~GShare() {
  // Nothing specific to clean up here
}

int GShare::getBHTIndex(uint32_t pc) {
  return ((pc >> (32 - BHR_SIZE)) ^ bhr) % BHT_SIZE;
}

bool GShare::predict(pipeline_trace_t* trace) {

  
  // Your prediction logic here
  // Use 'bht' and 'bhr' as needed
  // ...

  return true; // Placeholder return value
}

