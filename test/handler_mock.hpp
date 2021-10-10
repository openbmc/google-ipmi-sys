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

#pragma once

#include "handler.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
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

    MOCK_METHOD((std::tuple<std::uint8_t, std::string>), getEthDetails,
                (std::string), (const, override));
    MOCK_METHOD(std::int64_t, getRxPackets, (const std::string&),
                (const, override));
    MOCK_METHOD(
        (std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>),
        getCpldVersion, (unsigned int), (const, override));

    MOCK_METHOD(void, psuResetDelay, (std::uint32_t), (const, override));
    MOCK_METHOD(void, psuResetOnShutdown, (), (const, override));
    MOCK_METHOD(std::uint32_t, getFlashSize, (), (override));
    MOCK_METHOD(std::string, getEntityName, (std::uint8_t, std::uint8_t),
                (override));
    MOCK_METHOD(std::string, getMachineName, (), (override));
    MOCK_METHOD(void, buildI2cPcieMapping, (), (override));
    MOCK_METHOD(size_t, getI2cPcieMappingSize, (), (const, override));
    MOCK_METHOD((std::tuple<std::uint32_t, std::string>), getI2cEntry,
                (unsigned int), (const, override));
    MOCK_METHOD(void, hostPowerOffDelay, (std::uint32_t), (const, override));

    MOCK_METHOD(uint32_t, accelOobDeviceCount, (), (const, override));
    MOCK_METHOD(std::string, accelOobDeviceName, (size_t), (const, override));
    MOCK_METHOD(uint64_t, accelOobRead, (std::string_view, uint64_t, uint8_t),
                (const, override));
    MOCK_METHOD(void, accelOobWrite,
                (std::string_view, uint64_t, uint8_t, uint64_t),
                (const, override));
    MOCK_METHOD(std::vector<uint8_t>, pcieBifurcation, (uint8_t, bool),
                (override));
    MOCK_METHOD(uint8_t, getBmcMode, (), (override));
};

} // namespace ipmi
} // namespace google
