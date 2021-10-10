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
#include <string_view>
#include <unordered_set>
#include <vector>

namespace google
{
namespace ipmi
{

std::optional<uint8_t> Bifurcation::getBifurcation(uint8_t)
{
    // WIP need to call Entity Manager to get the information
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

        uint8_t bus;
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

inline std::vector<uint8_t>
    Bifurcation::walkMux(std::string_view basePath, std::string_view path,
                         std::shared_ptr<std::unordered_set<uint8_t>> cache)
{
    std::vector<uint8_t> output;

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::string_view channelPath = std::string(entry.path().filename());
        if (channelPath.starts_with("channel-"))
        {
            auto devicePath = fmt::format("{}/i2c-dev", entry.path().c_str());
            const auto& bifurcation = walkChannel(basePath, devicePath, cache);
            output.insert(output.end(), bifurcation.begin(), bifurcation.end());
        }
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

    std::vector<uint8_t> output;

    auto path = fmt::format("{}/i2c-{}", basePath, bus);
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::string_view i2cPath = std::string(entry.path().filename());

        if (i2cPath.starts_with(fmt::format("{}-", bus)))
        {
            auto muxPath = fmt::format("{}/{}", path, i2cPath);
            const auto& bifurcation = walkMux(basePath, muxPath, cache);
            output.insert(output.end(), bifurcation.begin(), bifurcation.end());
        }
    }

    if (output.empty())
    {
        return {*maybeBifurcation};
    }

    return output;
}

} // namespace ipmi
} // namespace google
