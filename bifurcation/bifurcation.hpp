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
#include <functional>
#include <optional>
#include <string>
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
    virtual std::optional<std::vector<uint8_t>>
        getBifurcation(uint8_t bus) noexcept = 0;
};

class BifurcationStatic : public BifurcationInterface
{
  public:
    static std::reference_wrapper<BifurcationInterface> createBifurcation()
    {
        static BifurcationStatic bifurcationStatic;

        return std::ref(bifurcationStatic);
    }

    BifurcationStatic(std::string_view bifurcationFile);

    std::optional<std::vector<uint8_t>>
        getBifurcation(uint8_t index) noexcept override;

  protected:
    BifurcationStatic();

  private:
    std::string bifurcationFile;
};

} // namespace ipmi
} // namespace google
