// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "bifurcation.hpp"

#include <optional>
#include <vector>

namespace google
{
namespace ipmi
{

std::optional<std::vector<uint8_t>> Bifurcation::getBifurcation(uint8_t index)
{
    auto findBifurication = bifurication.find(index);

    if (findBifurication == bifurication.end())
    {
        return std::nullopt;
    }

    return std::vector<uint8_t>{8, 8};
}

} // namespace ipmi
} // namespace google
