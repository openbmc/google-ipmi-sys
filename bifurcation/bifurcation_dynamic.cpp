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

#include <fmt/format.h>

#include <charconv>
#include <filesystem>
#include <memory>
#include <optional>
#include <sdbusplus/bus.hpp>
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
constexpr static const char* kFruDeviceInterfaceName =
    "xyz.openbmc_project.Inventory.Decorator.I2CDevice";
constexpr static const char* kPCIeDeviceInterfaceName =
    "xyz.openbmc_project.Inventory.Item.PCIeDevice";
constexpr static const char* kPCIeSlotInterfaceName =
    "xyz.openbmc_project.Inventory.Item.PCIeSlot";

void BifurcationDynamic::findPCIeDevices()
{
    ManagedObjectType entities;
    try
    {
        auto systemBus = sdbusplus::bus::new_default();
        auto getObjects = systemBus.new_method_call(
            kEntityManagerServiceName, "/", kObjectManagerServiceName,
            "GetManagedObjects");
        systemBus.call(getObjects).read(entities);
    }
    catch (const sdbusplus::exception::SdBusError&)
    {
        return;
    }

    for (const auto& [_, obj] : entities)
    {
        auto findFruDevice = obj.find(kFruDeviceInterfaceName);
        if (findFruDevice == obj.end())
        {
            continue;
        }

        auto findBus = findFruDevice->second.find("Bus");
        if (findBus == findFruDevice->second.end())
        {
            continue;
        }

        PCIe pcie;
        auto findPCIeDevice = obj.find(kPCIeDeviceInterfaceName);
        if (findPCIeDevice != obj.end())
        {
            auto findMaxLanes = findPCIeDevice->second.find("MaxLanes");
            if (findMaxLanes != findPCIeDevice->second.end() &&
                std::holds_alternative<uint64_t>(findMaxLanes->second))
            {
                pcie.type = PCIe::Type::Device;
                pcie.lanes = std::get<uint64_t>(findMaxLanes->second);
            }
        }

        auto findPCIeSlot = obj.find(kPCIeSlotInterfaceName);
        if (findPCIeSlot != obj.end())
        {
            auto findLanes = findPCIeSlot->second.find("Lanes");
            // TODO(wltu): Check to finalize this interface support.
            auto findChannels = findPCIeSlot->second.find("SupportedChannels");
            if (findLanes != findPCIeSlot->second.end() &&
                std::holds_alternative<uint64_t>(findLanes->second) &&
                findChannels != findPCIeSlot->second.end() &&
                std::holds_alternative<uint64_t>(findChannels->second))
            {
                pcie.type = PCIe::Type::Slot;
                pcie.lanes = std::get<uint64_t>(findLanes->second);
                pcie.channels = std::get<uint64_t>(findChannels->second);
            }
        }

        if (!std::holds_alternative<uint64_t>(findBus->second))
        {
            continue;
        }

        uint64_t bus = std::get<uint64_t>(findBus->second);
        pcieResources.emplace(bus, pcie);
    }

    if (!pcieResources.empty())
    {
        setup = false;
    }
}

std::optional<std::vector<uint8_t>>
    BifurcationDynamic::getBifurcation(uint8_t index) noexcept
{
    Cache cache;
    if (setup)
    {
        findPCIeDevices();
    }

    return walkI2CTreeBifurcation(i2cPath, index, cache);
}

std::vector<uint8_t> BifurcationDynamic::walkChannel(std::string_view basePath,
                                                     std::string_view path,
                                                     Cache& cache)
{
    // Use only the first device
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        const std::string& device = entry.path().filename();

        uint8_t bus = 0;
        // i2c-63
        //     ^ (start looking for bus number there)
        auto [ptr, ec] = std::from_chars(device.data() + 4,
                                         device.data() + device.size(), bus);
        if (ec != std::errc() || ptr != device.data() + device.size())
        {
            continue;
        }

        return walkI2CTreeBifurcation(basePath, bus, cache);
    }
    return {};
}

std::vector<std::vector<uint8_t>>
    BifurcationDynamic::walkMux(std::string_view basePath,
                                std::string_view path, uint8_t channelCount,
                                Cache& cache)
{
    std::vector<std::vector<uint8_t>> output;
    std::vector<std::string> channels;

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        const std::string& channelPath = entry.path().filename();
        if (channelPath.starts_with("channel-"))
        {
            channels.push_back(fmt::format("{}/i2c-dev", entry.path().c_str()));
        }
    }

    // Make sure to read from channel-0 to channel-N.
    // This allow the final bifurcation result to be determinsic and not
    // dependent on order of the files returned by directory_iterator.
    std::sort(channels.begin(), channels.end());
    for (uint8_t i = 0;
         i < std::min(channelCount, static_cast<uint8_t>(channels.size())); ++i)
    {
        output.push_back(walkChannel(basePath, channels[i], cache));
    }

    return output;
}

std::vector<uint8_t>
    BifurcationDynamic::walkI2CTreeBifurcation(std::string_view basePath,
                                               uint8_t bus, Cache& cache)
{
    auto path = fmt::format("{}/i2c-{}", basePath, bus);
    auto findBus = cache.find(path);
    if (findBus != cache.end())
    {
        return findBus->second;
    }

    // Set the cache with empty result to prevent loops.
    cache.emplace(path, std::vector<uint8_t>());
    auto maybePCIeDevice = pcieResources.find(bus);
    if (maybePCIeDevice == pcieResources.end())
    {
        return {};
    }

    if (maybePCIeDevice->second.type == PCIe::Type::Device)
    {
        return {maybePCIeDevice->second.lanes};
    }

    std::vector<std::vector<uint8_t>> output;

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        const std::string& i2cPath = entry.path().filename();

        if (i2cPath.starts_with(fmt::format("{}-", bus)))
        {
            auto muxPath = fmt::format("{}/{}", path, i2cPath);
            output = walkMux(basePath, muxPath,
                             maybePCIeDevice->second.channels, cache);
            break;
        }
    }

    std::vector<uint8_t> result;
    uint8_t lanes = maybePCIeDevice->second.lanes;
    uint8_t extraLanes = 0;
    uint8_t emptyCount = 0;
    for (const auto& out : output)
    {
        if (!out.empty())
        {
            for (uint8_t count : out)
            {
                if (count > lanes)
                {
                    return result;
                }
                lanes -= count;
            }
        }
        else
        {
            ++emptyCount;
        }
    }

    if (emptyCount != 0)
    {
        extraLanes = lanes % emptyCount;
        lanes = lanes / emptyCount;
        for (auto& out : output)
        {
            if (out.empty())
            {
                uint8_t currentLane =
                    lanes + static_cast<uint8_t>(extraLanes-- != 0);
                out = std::vector<uint8_t>{currentLane};
            }
        }
    }

    for (const auto& out : output)
    {
        result.insert(result.end(), out.begin(), out.end());
    }

    // Override the cache with real result.
    cache.emplace(path, result);
    return result;
}

} // namespace ipmi
} // namespace google
