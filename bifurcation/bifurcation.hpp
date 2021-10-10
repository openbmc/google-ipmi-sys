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
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace google
{
namespace ipmi
{

using Cache = std::unordered_map<std::string, std::vector<uint8_t>>;

struct PCIe
{
    enum class Type
    {
        None,
        Slot,
        Device,
    };

    Type type = Type::None;
    uint8_t lanes = 0;
    uint8_t channels = 0;
};

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

class BifurcationDynamic : public BifurcationInterface
{
  public:
    static std::reference_wrapper<BifurcationInterface> createBifurcation()
    {
        static BifurcationDynamic bifurcationDynamic;

        return std::ref(bifurcationDynamic);
    }

    BifurcationDynamic(bool setup = true,
                       const std::string& i2cPath = "/sys/bus/i2c/devices/") :
        setup(setup),
        i2cPath(i2cPath){};

    std::optional<std::vector<uint8_t>>
        getBifurcation(uint8_t index) noexcept override;

    std::optional<bool> needsUpdate(uint8_t bus);

    std::unordered_map<uint8_t, PCIe> pcieResources;

  private:
    BifurcationDynamic(const BifurcationDynamic&) = delete;
    BifurcationDynamic& operator=(const BifurcationDynamic&) = delete;
    BifurcationDynamic(BifurcationDynamic&&) = delete;
    BifurcationDynamic& operator=(BifurcationDynamic&&) = delete;

    bool setup;
    std::string i2cPath;
    void findPCIeDevices();

    /**
     * Walk the I2C tree to get the highest level of bifurcation at the given
     * i2c bus.
     *
     * @param[in] basePath  - Base path to look for i2c device
     * @return the bifurcation details. The number of lane taken by each device.
     */
    std::vector<uint8_t> walkI2CTreeBifurcation(std::string_view basePath,
                                                uint8_t bus, Cache& cache);

    /**
     * Walk all of the channel of a given i2c path
     *
     * @param[in] basePath  - Base path to look for i2c device
     * @param[in] path      - Current I2C path
     * @param[in] cache     - Cache to prevent loops
     * @return the bifurcation details. The number of lane taken by each device.
     */
    std::vector<uint8_t> walkChannel(std::string_view basePath,
                                     std::string_view path, Cache& cache);

    /**
     * Walk all of the Muxes of a given I2C path
     *
     * @param[in] basePath  - Base path to look for i2c device
     * @param[in] path      - Current I2C path
     * @param[in] channelCount  - Number of channels to look at
     * @param[in] cache     - Cache to prevent loops
     * @return the bifurcation details. The number of lane taken by each device.
     */
    std::vector<std::vector<uint8_t>> walkMux(std::string_view basePath,
                                              std::string_view path,
                                              uint8_t channelCount,
                                              Cache& cache);
};

} // namespace ipmi
} // namespace google
