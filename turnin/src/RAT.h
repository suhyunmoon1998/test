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

class RegisterAliasTable {
public:
  RegisterAliasTable(uint32_t size) : store_(size) {
    for (auto& entry : store_) {
      entry = -1;
    }
  }

  ~RegisterAliasTable() {}

  int get(int index) const {
    return store_[index];
  }

  void set(int index, int value) {
    store_[index] = value;
  }

private:
  std::vector<int> store_;
};

}