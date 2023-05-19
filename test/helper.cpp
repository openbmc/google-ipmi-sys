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

#include "helper.hpp"

#include <ipmid/api-types.hpp>

#include <optional>
#include <span>
#include <utility>

namespace google
{
namespace ipmi
{

std::pair<std::uint8_t, std::vector<std::uint8_t>>
    ValidateReply(::ipmi::RspType<std::uint8_t, std::vector<uint8_t>> reply,
                  bool hasData)
{
    // Reply is in the form of
    // std::tuple<ipmi::Cc, std::optional<std::tuple<RetTypes...>>>
    EXPECT_EQ(::ipmi::ccSuccess, std::get<0>(reply));

    auto actualReply = std::get<1>(reply);
    EXPECT_TRUE(actualReply.has_value());

    auto subcommand = std::get<0>(*actualReply);
    auto data = std::get<1>(*actualReply);
    EXPECT_EQ(hasData, !data.empty());

    return std::make_pair(subcommand, hasData ? data : std::vector<uint8_t>{});
}

} // namespace ipmi
} // namespace google
