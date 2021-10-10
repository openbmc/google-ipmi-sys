// Copyright 2023 Google LLC
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

#include "bifurcation.hpp"

#include <ipmid/message.hpp>

#include <optional>
#include <string>
#include <tuple>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

class BifurcationDynamicMock : public BifurcationDynamic
{
  public:
    ~BifurcationDynamicMock() = default;

    MOCK_METHOD(std::optional<uint8_t>, i2cBus,
                (::ipmi::Context::ptr, const std::string&),
                (noexcept, override));
    MOCK_METHOD(std::optional<uint64_t>, pcieDeviceMaxLanes,
                (::ipmi::Context::ptr, const std::string&),
                (noexcept, override));
    MOCK_METHOD((std::optional<uint64_t>), pcieSlotLanes,
                (::ipmi::Context::ptr, const std::string&),
                (noexcept, override));
    MOCK_METHOD(std::vector<std::string>, physicalAssociations,
                (::ipmi::Context::ptr, const std::string&),
                (noexcept, override));
};

} // namespace ipmi
} // namespace google
