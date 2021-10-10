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
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace google
{
namespace ipmi
{

class BifurcationInterface
{
  public:
    virtual ~BifurcationInterface() = default;

    /**
     * Get the Bifurcation of the device at the i2c bus
     *
     * @param[in] bus    - I2C bus of the device
     * @return the bifurcation at the i2c bus
     */
    virtual std::optional<std::vector<uint8_t>> getBifurcation(uint8_t bus) = 0;
};

class Bifurcation : public BifurcationInterface
{
  public:
    static Bifurcation* createBifurcation()
    {
        static Bifurcation bifurcation;

        return &bifurcation;
    }

    Bifurcation();

    std::optional<std::vector<uint8_t>> getBifurcation(uint8_t index) override;

  private:
    std::unordered_map<uint8_t, std::vector<uint8_t>> bifurication;
};

} // namespace ipmi
} // namespace google
