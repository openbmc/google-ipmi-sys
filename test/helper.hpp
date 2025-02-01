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

#include "handler_mock.hpp"

#include <ipmid/api-types.hpp>

#include <span>
#include <utility>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

// Validate the return code and the data for the IPMI reply.
// Returns the subcommand and the optional information.
std::pair<std::uint8_t, std::vector<std::uint8_t>> ValidateReply(
    ::ipmi::RspType<std::uint8_t, std::vector<uint8_t>> reply,
    bool hasData = true);

} // namespace ipmi
} // namespace google
