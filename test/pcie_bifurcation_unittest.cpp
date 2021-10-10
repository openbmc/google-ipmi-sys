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

#include "commands.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"
#include "pcie_bifurcation.hpp"

#include <vector>

#include <gtest/gtest.h>

using ::testing::Return;

namespace google
{
namespace ipmi
{

using testing::_;
using ::testing::ContainerEq;

TEST(PcieBifurcationCommandTest, InvalidRequestStatic)
{
    std::vector<uint8_t> request = {};

    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              pcieBifurcation(nullptr, request, &hMock, false));
}

TEST(PcieBifurcationCommandTest, ValidRequestStatic)
{
    std::vector<uint8_t> request = {5};
    std::vector<uint8_t> expectedOutput = {4, 8, 1, 2};

    HandlerMock hMock;
    EXPECT_CALL(hMock, pcieBifurcationByIndex(_, 5, false))
        .WillOnce(Return(expectedOutput));

    auto reply = pcieBifurcation(nullptr, request, &hMock, false);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(struct PcieBifurcationReply) + expectedOutput.size(),
              data.size());
    EXPECT_EQ(SysOEMCommands::SysPCIeSlotBifurcation, result.first);
    EXPECT_THAT(std::vector<uint8_t>(data.begin() + 1, data.end()),
                ContainerEq(expectedOutput));
}

TEST(PcieBifurcationCommandTest, ReplyExceddedMaxValueStatic)
{
    std::vector<uint8_t> request = {5};
    std::vector<uint8_t> expectedOutput(64, 1);

    HandlerMock hMock;
    EXPECT_CALL(hMock, pcieBifurcationByIndex(_, 5, false))
        .WillOnce(Return(expectedOutput));
    EXPECT_EQ(::ipmi::responseInvalidCommand(),
              pcieBifurcation(nullptr, request, &hMock, false));
}

// Dynamic
TEST(PcieBifurcationCommandTest, InvalidRequestDynamic)
{
    std::vector<std::uint8_t> request = {};

    HandlerMock hMock;
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              pcieBifurcation(nullptr, request, &hMock, true));
}

TEST(PcieBifurcationCommandTest, ValidRequestDynamic)
{
    std::vector<std::uint8_t> request = {5};
    std::vector<uint8_t> expectedOutput = {4, 8, 1, 2};

    HandlerMock hMock;
    EXPECT_CALL(hMock, pcieBifurcationByIndex(_, 5, true))
        .WillOnce(Return(expectedOutput));

    auto reply = pcieBifurcation(nullptr, request, &hMock, true);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(struct PcieBifurcationReply) + expectedOutput.size(),
              data.size());
    EXPECT_EQ(SysOEMCommands::SysPCIeSlotBifurcation, result.first);
    ASSERT_EQ(data[0], expectedOutput.size());
    EXPECT_EQ(data[1], 4);
    EXPECT_EQ(data[2], 8);
    EXPECT_EQ(data[3], 1);
    EXPECT_EQ(data[4], 2);
}

TEST(PcieBifurcationCommandTest, ReplyExceddedMaxValueDynamic)
{
    std::vector<std::uint8_t> request = {5};
    std::vector<uint8_t> expectedOutput(64, 1);

    HandlerMock hMock;
    EXPECT_CALL(hMock, pcieBifurcationByIndex(_, 5, true))
        .WillOnce(Return(expectedOutput));
    EXPECT_EQ(::ipmi::responseInvalidCommand(),
              pcieBifurcation(nullptr, request, &hMock, true));
}

} // namespace ipmi
} // namespace google
