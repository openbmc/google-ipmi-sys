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

#include <ipmid/message.hpp>

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
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] bus    - I2C bus of the device
     * @return the bifurcation at the i2c bus
     */
    virtual std::optional<std::vector<uint8_t>>
        getBifurcation(::ipmi::Context::ptr ctx, uint8_t bus) noexcept = 0;

    /**
     * Request the MaxLanes from the PCIeDevice dbus object
     *
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] path   Dbus object path for PCIeDevice
     * @return the bifurcation at the i2c bus
     */
    virtual std::optional<uint8_t> i2cBus(::ipmi::Context::ptr ctx,
                                          const std::string& path) noexcept = 0;

    /**
     * Request the MaxLanes from the PCIeDevice dbus object
     *
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] path   Dbus object path for PCIeDevice
     * @return the bifurcation at the i2c bus
     */
    virtual std::optional<uint64_t>
        pcieDeviceMaxLanes(::ipmi::Context::ptr ctx,
                           const std::string& path) noexcept = 0;

    /**
     * Request the Lanes and SupportedChannels from the PCIeSlot dbus object
     *
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] path   Dbus object path for PCIeSlot
     * @return the bifurcation at the i2c bus
     */
    virtual std::optional<uint64_t>
        pcieSlotLanes(::ipmi::Context::ptr ctx,
                      const std::string& path) noexcept = 0;

    /**
     * Request the Lanes and SupportedChannels from the PCIeSlot dbus object
     *
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] path   Dbus object path for PCIeSlot
     * @return the bifurcation at the i2c bus
     */
    virtual std::vector<std::string>
        physicalAssociations(::ipmi::Context::ptr ctx,
                             const std::string& path) noexcept = 0;
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
        getBifurcation(::ipmi::Context::ptr ctx,
                       uint8_t index) noexcept override;

    std::optional<uint8_t> i2cBus(::ipmi::Context::ptr,
                                  const std::string&) noexcept override
    {
        return std::nullopt;
    };

    std::optional<uint64_t>
        pcieDeviceMaxLanes(::ipmi::Context::ptr,
                           const std::string&) noexcept override
    {
        return std::nullopt;
    };

    std::optional<uint64_t> pcieSlotLanes(::ipmi::Context::ptr,
                                          const std::string&) noexcept override
    {
        return std::nullopt;
    };

    std::vector<std::string>
        physicalAssociations(::ipmi::Context::ptr,
                             const std::string&) noexcept override
    {
        return {};
    };

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

    BifurcationDynamic() = default;

    std::optional<std::vector<uint8_t>>
        getBifurcation(::ipmi::Context::ptr ctx,
                       uint8_t index) noexcept override;

    std::optional<uint8_t> i2cBus(::ipmi::Context::ptr ctx,
                                  const std::string& path) noexcept override;

    std::optional<uint64_t>
        pcieDeviceMaxLanes(::ipmi::Context::ptr ctx,
                           const std::string& path) noexcept override;

    std::optional<uint64_t>
        pcieSlotLanes(::ipmi::Context::ptr ctx,
                      const std::string& path) noexcept override;

    std::vector<std::string>
        physicalAssociations(::ipmi::Context::ptr ctx,
                             const std::string& path) noexcept override;

    std::unordered_map<uint8_t, PCIe> pcieResources;

  private:
    BifurcationDynamic(const BifurcationDynamic&) = delete;
    BifurcationDynamic& operator=(const BifurcationDynamic&) = delete;
    BifurcationDynamic(BifurcationDynamic&&) = delete;
    BifurcationDynamic& operator=(BifurcationDynamic&&) = delete;

    std::vector<uint8_t> parseDevices(::ipmi::Context::ptr ctx,
                                      const std::string& path);
};

} // namespace ipmi
} // namespace google
