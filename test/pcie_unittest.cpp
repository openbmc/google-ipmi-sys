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

#include "commands.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"
#include "pcie_i2c.hpp"

#include <cstdint>
#include <cstring>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(PcieI2cCommandTest, PcieSlotCountTest)
{
    std::vector<std::uint8_t> request = {};
    size_t expectedSize = 3;

    HandlerMock hMock;
    EXPECT_CALL(hMock, buildI2cPcieMapping());
    EXPECT_CALL(hMock, getI2cPcieMappingSize()).WillOnce(Return(expectedSize));

    auto reply = pcieSlotCount(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(struct PcieSlotCountReply), data.size());
    EXPECT_EQ(SysOEMCommands::SysPcieSlotCount, result.first);
    EXPECT_EQ(expectedSize, data[0]);
}

TEST(PcieI2cCommandTest, PcieSlotEntryRequestTooShort)
{
    std::vector<std::uint8_t> request = {};

    HandlerMock hMock;
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              pcieSlotI2cBusMapping(request, &hMock));
}

TEST(PcieI2cCommandTest, PcieSlotEntryRequestUnsupportedByPlatform)
{
    // If there is no mapping in the device-tree, then the map is of size zero.
    std::vector<std::uint8_t> request = {0};

    HandlerMock hMock;
    EXPECT_CALL(hMock, getI2cPcieMappingSize()).WillOnce(Return(0));
    EXPECT_EQ(::ipmi::responseInvalidReservationId(),
              pcieSlotI2cBusMapping(request, &hMock));
}

TEST(PcieI2cCommandTest, PcieSlotEntryRequestInvalidIndex)
{
    // index of 1 is invalid if length is 1.
    std::vector<std::uint8_t> request = {1};

    HandlerMock hMock;
    EXPECT_CALL(hMock, getI2cPcieMappingSize()).WillOnce(Return(1));
    EXPECT_EQ(::ipmi::responseParmOutOfRange(),
              pcieSlotI2cBusMapping(request, &hMock));
}

TEST(PcieI2cCommandTest, PcieSlotEntryRequestValidIndex)
{
    unsigned int index = 0;
    std::vector<std::uint8_t> request = {static_cast<std::uint8_t>(index)};
    std::string slotName = "abcd";
    std::uint32_t busNum = 5;

    HandlerMock hMock;
    EXPECT_CALL(hMock, getI2cPcieMappingSize()).WillOnce(Return(1));
    EXPECT_CALL(hMock, getI2cEntry(index))
        .WillOnce(Return(std::make_tuple(busNum, slotName)));

    auto reply = pcieSlotI2cBusMapping(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(SysOEMCommands::SysPcieSlotI2cBusMapping, result.first);
    EXPECT_EQ(busNum, data[0]);
    EXPECT_EQ(slotName.length(), data[1]);
    EXPECT_EQ(
        slotName,
        std::string(data.begin() + sizeof(struct PcieSlotI2cBusMappingReply),
                    data.end()));
}

} // namespace ipmi
} // namespace google
