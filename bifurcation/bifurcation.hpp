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
#include <nlohmann/json.hpp>

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

class BifurcationInterface
{
  public:
    virtual ~BifurcationInterface() = default;

    /**
     * Get the Bifurcation of the device
     *
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] name    PCIe Device Name
     * @return the bifurcation with the PCIe Device
     */
    virtual std::optional<std::vector<uint8_t>>
        getBifurcation(::ipmi::Context::ptr ctx,
                       std::string_view name) noexcept = 0;

    /**
     * Request dbus call to EntityManager to fetch the bus number of the object.
     *
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] path   Dbus object path for PCIeDevice
     * @return i2c bud of the object
     */
    virtual std::optional<uint8_t> i2cBus(::ipmi::Context::ptr ctx,
                                          const std::string& path) noexcept = 0;

    /**
     * Request the MaxLanes from the PCIeDevice dbus object
     *
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] path   Dbus object path for PCIeDevice
     * @return the MaxLanes of the dbus object
     */
    virtual std::optional<uint64_t>
        pcieDeviceMaxLanes(::ipmi::Context::ptr ctx,
                           const std::string& path) noexcept = 0;

    /**
     * Request the Lanes from the PCIeSlot dbus object
     *
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] path   Dbus object path for PCIeSlot
     * @return the Lanes of the dbus object
     */
    virtual std::optional<uint64_t>
        pcieSlotLanes(::ipmi::Context::ptr ctx,
                      const std::string& path) noexcept = 0;

    /**
     * Physical Association that is conatained_by the target dbus object.
     *
     * @param[in] ctx    IPMI Context Pointer
     * @param[in] path   Dbus object path for PCIeSlot
     * @return list of dbus object contained by target dbus object.
     */
    virtual std::vector<std::string>
        physicalAssociations(::ipmi::Context::ptr ctx,
                             const std::string& path) noexcept = 0;
};

class BifurcationStatic : public BifurcationInterface
{
  public:
    /**
     * Create BifurcationStatic object
     *
     * @return the reference of the bifurcation implmentation
     */
    static std::reference_wrapper<BifurcationInterface> createBifurcation()
    {
        static BifurcationStatic bifurcationStatic;

        return std::ref(bifurcationStatic);
    }

    BifurcationStatic(std::string_view bifurcationFile);

    std::optional<std::vector<uint8_t>>
        getBifurcation(::ipmi::Context::ptr ctx,
                       std::string_view name) noexcept override;

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
    /**
     * Create BifurcationDynamic object
     *
     * @return the reference of the bifurcation implmentation
     */
    static std::reference_wrapper<BifurcationInterface>
        createBifurcation(const nlohmann::json& entityJson)
    {
        static BifurcationDynamic bifurcationDynamic(entityJson);

        return std::ref(bifurcationDynamic);
    }

    BifurcationDynamic(const nlohmann::json& entityJson) :
        entityJson(entityJson){};

    std::optional<std::vector<uint8_t>>
        getBifurcation(::ipmi::Context::ptr ctx,
                       std::string_view name) noexcept override;

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

    BifurcationDynamic(const BifurcationDynamic&) = delete;
    BifurcationDynamic& operator=(const BifurcationDynamic&) = delete;
    BifurcationDynamic(BifurcationDynamic&&) = delete;
    BifurcationDynamic& operator=(BifurcationDynamic&&) = delete;

  private:
    /**
     * Parese through all devices and associations to return the bifurcation
     *
     * @return bifurcation result
     */
    std::vector<uint8_t> parseDevices(::ipmi::Context::ptr ctx,
                                      const std::string& path);
    nlohmann::json entityJson;
};

} // namespace ipmi
} // namespace google
