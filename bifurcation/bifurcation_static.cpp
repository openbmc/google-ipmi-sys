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

BifurcationStatic::BifurcationStatic() :
    BifurcationStatic(ENABLE_BIFURCATION_CONFIG, STATIC_BIFURCATION_CONFIG)
{
}

BifurcationStatic::BifurcationStatic(std::string_view enableBifurcation,
                                     std::string_view bifurcationFile) :
    enableBifurcation(enableBifurcation),
    bifurcationFile(bifurcationFile)
{
}

std::optional<std::vector<uint8_t>>
    BifurcationStatic::getBifurcation(uint8_t index)
{
    std::unordered_map<uint8_t, std::vector<uint8_t>> bifurcation;

    if (enableBifurcation.empty())
    {
        fmt::print(stderr,
                   "Invalid command: Bifurcation Enable Config is missing\n");
        return std::nullopt;
    }

    std::string result;
    try
    {
        std::ifstream f(enableBifurcation.data(), std::ios::in);
        size_t size = std::filesystem::file_size(enableBifurcation);
        result = std::string(size, '\0');
        f.read(result.data(), size);
        f.close();
    }
    catch (std::exception const& e)
    {
        fmt::print(stderr, "Failed to open {}\n: {}", enableBifurcation.data(),
                   e.what());
        return std::nullopt;
    }

    if (result != "1\n")
    {
        fmt::print(stderr, "Static Bifurcation not enabled\n");
        return std::nullopt;
    }

    // Example valid data:
    // {
    //     "1": [8,8],
    //     "2": [4, 4, 12]
    // }
    std::ifstream jsonFile(bifurcationFile.data());
    if (!jsonFile.is_open())
    {
        fmt::print(stderr, "Unable to open file {} for bifurcation.\n",
                   bifurcationFile.data());
        return std::nullopt;
    }

    auto jsonData = nlohmann::json::parse(jsonFile, nullptr, false);

    std::string_view key_view = std::to_string(index);
    auto value = jsonData[key_view.data()];
    if (!value.is_array())
    {
        fmt::print(stderr, "failed to fetch bifurcation info from {}\n",
                   key_view.data());
        return std::nullopt;
    }

    std::vector<uint8_t> vec;
    try
    {
        value.get_to(vec);
    }
    catch (std::exception const& e)
    {
        fmt::print(stderr,
                   "Failed to convert bifurcation value to vec[uin8_t]\n");
        return std::nullopt;
    }

    return vec;
}

} // namespace ipmi
} // namespace google
