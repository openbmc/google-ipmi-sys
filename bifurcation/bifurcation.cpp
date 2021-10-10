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
#include <unordered_set>
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
    std::unordered_map<sdbusplus::message::object_path, ObjectType>;

constexpr static const char* kObjectManagerServiceName =
    "org.freedesktop.DBus.ObjectManager";
constexpr static const char* kEntityManagerServiceName =
    "xyz.openbmc_project.EntityManager";
constexpr static const char* kFruDeviceInterfaceName =
    "xyz.openbmc_project.Inventory.Decorator.FruDevice";

std::optional<std::vector<uint8_t>> Bifurcation::getBifurcation(uint8_t bus)
{
    ManagedObjectType entities;
    auto systemBus = sdbusplus::bus::new_default();
    auto getObjects = systemBus.new_method_call(kEntityManagerServiceName, "/",
                                                kObjectManagerServiceName,
                                                "GetManagedObjects");
    auto resp = systemBus.call(getObjects);
    resp.read(entities);
    for (const auto& entry : entities)
    {
        auto findFruDevice = entry.second.find(kFruDeviceInterfaceName);
        if (findFruDevice == entry.second.end())
        {
            continue;
        }

        auto findBus = findFruDevice->second.find("Bus");
        auto findAddress = findFruDevice->second.find("Address");
        auto findBifurcation = findFruDevice->second.find("Bifurcation");
        if (findBus == findFruDevice->second.end() ||
            findAddress == findFruDevice->second.end() ||
            findBifurcation == findFruDevice->second.end())
        {
            continue;
        }

        if (!std::holds_alternative<uint64_t>(findBus->second) ||
            std::get<uint64_t>(findBus->second) != bus)
        {
            continue;
        }

        if (!std::holds_alternative<std::vector<uint64_t>>(
                findBifurcation->second))
        {
            continue;
        }

        auto bifurcation =
            std::get<std::vector<uint64_t>>(findBifurcation->second);
        return std::vector<uint8_t>(bifurcation.begin(), bifurcation.end());
    }

    return std::nullopt;
}

inline std::vector<uint8_t>
    Bifurcation::walkChannel(std::string_view basePath, std::string_view path,
                             std::shared_ptr<std::unordered_set<uint8_t>> cache)
{
    // Use only the first device
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::string_view device = std::string(entry.path().filename());

        uint8_t bus = 0;
        auto result = std::from_chars(device.data() + 4,
                                      device.data() + device.size(), bus);
        if (result.ec == std::errc::invalid_argument)
        {
            continue;
        }

        return walkI2CTreeBifurcation(basePath, bus, cache);
    }
    return {};
}

inline std::vector<std::vector<uint8_t>>
    Bifurcation::walkMux(std::string_view basePath, std::string_view path,
                         std::shared_ptr<std::unordered_set<uint8_t>> cache)
{
    std::vector<std::vector<uint8_t>> output;
    std::vector<std::string> channels;

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::string_view channelPath = std::string(entry.path().filename());
        if (channelPath.starts_with("channel-"))
        {
            channels.push_back(fmt::format("{}/i2c-dev", entry.path().c_str()));
        }
    }

    // Make sure to read from channel-0 to channel-N.
    std::sort(channels.begin(), channels.end());
    for (const auto& channel : channels)
    {
        output.push_back(walkChannel(basePath, channel, cache));
    }

    return output;
}

// Assuming that there are no loops.
// It will prevent loops, but the result wil be wrong if loops exists.
// If there are loops, the kernel is setup wrong.
std::vector<uint8_t> Bifurcation::walkI2CTreeBifurcation(
    std::string_view basePath, uint8_t bus,
    std::shared_ptr<std::unordered_set<uint8_t>> cache)
{
    if (cache->count(bus) > 0)
    {
        return {};
    }

    auto maybeBifurcation = getBifurcation(bus);
    if (!maybeBifurcation)
    {
        return {};
    }

    cache->insert(bus);
    std::vector<std::vector<uint8_t>> output;

    auto path = fmt::format("{}/i2c-{}", basePath, bus);
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::string_view i2cPath = std::string(entry.path().filename());

        if (i2cPath.starts_with(fmt::format("{}-", bus)))
        {
            auto muxPath = fmt::format("{}/{}", path, i2cPath);
            output = walkMux(basePath, muxPath, cache);
            break;
        }
    }

    std::vector<uint8_t> result;
    for (size_t i = 0; i < maybeBifurcation->size(); ++i)
    {
        if (output[i].empty())
        {
            result.emplace_back((*maybeBifurcation)[i]);
        }
        else
        {
            result.insert(result.end(), output[i].begin(), output[i].end());
        }
    }

    return result;
}

} // namespace ipmi
} // namespace google
