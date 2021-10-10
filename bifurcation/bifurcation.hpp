// Copyright 2021 Google LLC
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

#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace google
{
namespace ipmi
{

using Cache = std::unordered_map<std::string, std::vector<uint8_t>>;

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

    std::optional<std::vector<uint8_t>> getBifurcation(uint8_t bus) override;

    /**
     * Walk the I2C tree to get the highest level of bifurcation at the given
     * i2c bus.
     *
     * @param[in] basePath  - Base path to look for i2c device
     * @return the bifurcation details. The number of lane taken by each device.
     */
    std::vector<uint8_t> walkI2CTreeBifurcation(std::string_view basePath,
                                                uint8_t bus, Cache& cache);

  private:
    Bifurcation() = default;
    Bifurcation(const Bifurcation&) = delete;
    Bifurcation& operator=(const Bifurcation&) = delete;
    Bifurcation(Bifurcation&&) = delete;
    Bifurcation& operator=(Bifurcation&&) = delete;

    std::vector<uint8_t> walkChannel(std::string_view basePath,
                                     std::string_view path, Cache& cache);

    std::vector<std::vector<uint8_t>>
        walkMux(std::string_view basePath, std::string_view path, Cache& cache);
};

} // namespace ipmi
} // namespace google
