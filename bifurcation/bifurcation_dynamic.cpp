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
#include "config.h"

#include "bifurcation.hpp"

#include <ipmid/message.hpp>
#include <sdbusplus/bus.hpp>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace google
{
namespace ipmi
{

using Value = std::variant<uint64_t, std::vector<uint64_t>>;
using ObjectType =
    std::unordered_map<std::string, std::unordered_map<std::string, Value>>;
using ManagedObjectType =
    std::vector<std::pair<sdbusplus::message::object_path, ObjectType>>;

constexpr static const char* kObjectManagerServiceName =
    "org.freedesktop.DBus.ObjectManager";
constexpr static const char* kEntityManagerServiceName =
    "xyz.openbmc_project.EntityManager";
constexpr static const char* kI2CDeviceInterfaceName =
    "xyz.openbmc_project.Inventory.Decorator.I2CDevice";
constexpr static const char* kPCIeDeviceInterfaceName =
    "xyz.openbmc_project.Inventory.Item.PCIeDevice";
constexpr static const char* kPCIeSlotInterfaceName =
    "xyz.openbmc_project.Inventory.Item.PCIeSlot";

std::optional<uint8_t>
    BifurcationDynamic::i2cBus(::ipmi::Context::ptr ctx,
                               const std::string& path) noexcept
{
    boost::system::error_code ec;
    std::variant<uint64_t> bus =
        ctx->bus->yield_method_call<std::variant<uint64_t>>(
            ctx->yield, ec, kEntityManagerServiceName, path,
            "org.freedesktop.DBus.Properties", "Get", kI2CDeviceInterfaceName,
            "Bus");

    if (ec)
    {
        return std::nullopt;
    }

    return static_cast<uint8_t>(std::get<uint64_t>(bus));
}

std::optional<uint64_t>
    BifurcationDynamic::pcieDeviceMaxLanes(::ipmi::Context::ptr ctx,
                                           const std::string& path) noexcept
{
    boost::system::error_code ec;
    std::variant<uint64_t> deviceLanes =
        ctx->bus->yield_method_call<std::variant<uint64_t>>(
            ctx->yield, ec, kEntityManagerServiceName, path,
            "org.freedesktop.DBus.Properties", "Get", kPCIeDeviceInterfaceName,
            "MaxLanes");
    if (ec)
    {
        return std::nullopt;
    }
    return std::get<uint64_t>(deviceLanes);
}

std::optional<uint64_t>
    BifurcationDynamic::pcieSlotLanes(::ipmi::Context::ptr ctx,
                                      const std::string& path) noexcept
{
    boost::system::error_code ec;
    std::variant<uint64_t> deviceLanes =
        ctx->bus->yield_method_call<std::variant<uint64_t>>(
            ctx->yield, ec, kEntityManagerServiceName, path,
            "org.freedesktop.DBus.Properties", "Get", kPCIeSlotInterfaceName,
            "Lanes");
    if (ec)
    {
        return std::nullopt;
    }
    return std::get<uint64_t>(deviceLanes);
}

std::vector<std::string>
    BifurcationDynamic::physicalAssociations(::ipmi::Context::ptr ctx,
                                             const std::string& path) noexcept
{
    const std::array<std::string_view, 2> interfaces = {
        kPCIeDeviceInterfaceName, kPCIeSlotInterfaceName};
    boost::system::error_code ec;
    std::vector<std::string> associations =
        ctx->bus->yield_method_call<std::vector<std::string>>(
            ctx->yield, ec, "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetAssociatedSubTreePaths",
            sdbusplus::message::object_path(path) / "containing",
            sdbusplus::message::object_path("/xyz/openbmc_project/inventory"),
            0, interfaces);
    if (ec)
    {
        return std::vector<std::string>();
    }
    return associations;
}

std::vector<uint8_t> BifurcationDynamic::parseDevices(::ipmi::Context::ptr ctx,
                                                      const std::string& path)
{
    std::optional<uint64_t> deviceLanes = pcieDeviceMaxLanes(ctx, path);
    if (deviceLanes != std::nullopt)
    {
        return std::vector<uint8_t>{static_cast<uint8_t>(deviceLanes.value())};
    }

    std::optional<uint64_t> slotProperties = pcieSlotLanes(ctx, path);
    if (slotProperties == std::nullopt)
    {
        return std::vector<uint8_t>();
    }

    auto maxLanes = slotProperties.value();
    std::vector<std::string> associations = physicalAssociations(ctx, path);

    std::vector<uint8_t> bifurication;
    for (const std::string& childPath : associations)
    {
        std::vector<uint8_t> current = parseDevices(ctx, childPath);
        bifurication.insert(bifurication.end(), current.begin(), current.end());
    }

    uint64_t totalLanes = std::accumulate(bifurication.begin(),
                                          bifurication.end(), 0);

    if (totalLanes > maxLanes)
    {
        return std::vector<uint8_t>();
    }

    if (totalLanes < maxLanes)
    {
        bifurication.emplace_back(maxLanes - totalLanes);
    }

    return bifurication;
}

std::optional<std::vector<uint8_t>>
    BifurcationDynamic::getBifurcation(::ipmi::Context::ptr ctx,
                                       uint8_t bus) noexcept
{
    std::vector<uint8_t> bifurication;
    for (const std::string& path : physicalAssociations(ctx, MAIN_BOARD))
    {
        if (i2cBus(ctx, path).value_or(0) != bus)
        {
            continue;
        }

        std::vector<uint8_t> currentBifurication = parseDevices(ctx, path);
        if (currentBifurication.size() > bifurication.size())
        {
            bifurication = currentBifurication;
        }
    }

    return bifurication;
}

} // namespace ipmi
} // namespace google
