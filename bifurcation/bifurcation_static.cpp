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

#include <fmt/format.h>

#include <charconv>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>
#include <vector>

namespace google
{
namespace ipmi
{

Bifurcation::Bifurcation()
{
    std::string_view bifuricationFile = STATIC_BIFURICATION_CONFIG;

    // Example valid data:
    // {
    //     "1": [8,8],
    //     "2": [4,4, 12]
    // }
    std::ifstream jsonFile(bifuricationFile.data());
    if (!jsonFile.is_open())
    {
        fmt::print(stderr, "Unable to open file {} for bifurication.\n",
                   bifuricationFile.data());
        return;
    }

    auto jsonData = nlohmann::json::parse(jsonFile, nullptr, false);
    for (auto& [key, value] : jsonData.items())
    {
        if (!value.is_array())
        {
            continue;
        }

        std::string_view key_view = key;
        uint32_t num;
        auto [ptr, ec]{std::from_chars(key_view.begin(), key_view.end(), num)};
        if (ec != std::errc())
        {
            auto message =
                fmt::format("failed to convert string `{}` to uint32_t: {}",
                            key, std::make_error_code(ec).message());
            fmt::print(stderr, "{}\n", message);
            throw std::runtime_error(message);
        }
        if (ptr != key_view.end())
        {
            auto message = fmt::format("converted invalid characters: {}", key);
            fmt::print(stderr, "{}\n", message);
            throw std::runtime_error(message);
        }

        std::vector<uint8_t> vec;
        value.get_to(vec);
        bifurication[num] = vec;
    }
}

std::optional<std::vector<uint8_t>> Bifurcation::getBifurcation(uint8_t index)
{
    auto findBifurication = bifurication.find(index);

    if (findBifurication == bifurication.end())
    {
        return std::nullopt;
    }

    return findBifurication->second;
}

} // namespace ipmi
} // namespace google
