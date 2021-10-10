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
#include <nlohmann/json.hpp>
#include <stdplus/print.hpp>

#include <charconv>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>
#include <vector>

namespace google
{
namespace ipmi
{

BifurcationStatic::BifurcationStatic() :
    BifurcationStatic(STATIC_BIFURCATION_CONFIG)
{}

BifurcationStatic::BifurcationStatic(std::string_view bifurcationFile) :
    bifurcationFile(bifurcationFile)
{}

std::optional<std::vector<uint8_t>>
    BifurcationStatic::getBifurcation(::ipmi::Context::ptr,
                                      std::string_view name) noexcept
{
    // Example valid data:
    // {
    //     "/PE1": [8,8],
    //     "/PE2": [4, 4, 12]
    // }
    std::ifstream jsonFile(bifurcationFile.c_str());
    if (!jsonFile.is_open())
    {
        stdplus::print(stderr, "Unable to open file {} for bifurcation.\n",
                       bifurcationFile.data());
        return std::nullopt;
    }

    nlohmann::json jsonData;
    try
    {
        jsonData = nlohmann::json::parse(jsonFile, nullptr, false);
    }
    catch (const nlohmann::json::parse_error& ex)
    {
        stdplus::print(
            stderr,
            "Failed to parse the static config. Parse error at byte {}\n",
            ex.byte);
        return std::nullopt;
    }

    std::vector<uint8_t> vec;
    try
    {
        auto value = jsonData[name];
        value.get_to(vec);
    }
    catch (const std::exception& e)
    {
        stdplus::print(stderr,
                       "Failed to convert bifurcation value to vec[uin8_t]\n");
        return std::nullopt;
    }

    return vec;
}

} // namespace ipmi
} // namespace google
