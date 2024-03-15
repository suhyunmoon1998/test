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

namespace tinyrv {

struct pipeline_trace_t;

class GShare {
public:
  GShare();

  ~GShare();

  bool predict(pipeline_trace_t* trace);

private:
  // Constants for BHR and BHT sizes
  static constexpr int BHR_SIZE = 8; // Adjust as needed
  static constexpr int BHT_SIZE = 1024; // Adjust as needed

  // Define BHR and BHT
  uint8_t bhr; // Assuming 8-bit BHR
  int bht[BHT_SIZE]; // Array of BHT entries (initialized in constructor)

  // Function to calculate the index into the BHT based on PC and BHR
  int getBHTIndex(uint32_t pc);
};

} // namespace tinyrv
