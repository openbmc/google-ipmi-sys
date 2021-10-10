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

#include "handler.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>

#include <gmock/gmock.h>

namespace google
{
namespace ipmi
{

class HandlerMock : public HandlerInterface
{

  public:
    ~HandlerMock() = default;

    MOCK_CONST_METHOD1(getEthDetails,
                       std::tuple<std::uint8_t, std::string>(std::string));
    MOCK_CONST_METHOD1(getRxPackets, std::int64_t(const std::string&));
    MOCK_CONST_METHOD1(getCpldVersion,
                       std::tuple<std::uint8_t, std::uint8_t, std::uint8_t,
                                  std::uint8_t>(unsigned int));
    MOCK_CONST_METHOD1(psuResetDelay, void(std::uint32_t));
    MOCK_CONST_METHOD0(psuResetOnShutdown, void());
    MOCK_METHOD0(getFlashSize, uint32_t());
    MOCK_METHOD2(getEntityName, std::string(std::uint8_t, std::uint8_t));
    MOCK_METHOD0(getMachineName, std::string());
    MOCK_METHOD0(buildI2cPcieMapping, void());
    MOCK_CONST_METHOD0(getI2cPcieMappingSize, size_t());
    MOCK_CONST_METHOD1(getI2cEntry,
                       std::tuple<std::uint32_t, std::string>(unsigned int));
    MOCK_CONST_METHOD1(hostPowerOffDelay, void(std::uint32_t));
};

} // namespace ipmi
} // namespace google
