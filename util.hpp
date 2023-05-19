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

#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

namespace google
{
namespace ipmi
{

/**
 * Parse a file and return the json object.
 *
 * @param[in] file - the path to the file to parse.
 * @return the json object if valid.
 * @throw elog<InternalFailure> on failures.
 */
nlohmann::json parseConfig(const std::string& file);

/**
 * Read a dts property file and return the contents.
 *
 * @param[in] file - the path to the file to parse.
 * @return the property value or an empty string on failure.
 */
std::string readPropertyFile(const std::string& fileName);

/**
 * Build a map of the i2c bus numbers to their PCIe slot names.
 *
 * @return list of pairs of i2c bus with their corresponding slot names.
 */
std::vector<std::tuple<std::uint32_t, std::string>> buildPcieMap();

} // namespace ipmi
} // namespace google
