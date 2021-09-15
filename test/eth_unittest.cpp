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
#include "eth.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"

#include <cstdint>
#include <cstring>
#include <string_view>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(EthCommandTest, ValidRequestReturnsSuccess)
{
    // This command requests no input, therefore it will just return what it
    // knows.
    std::vector<std::uint8_t> request = {};
    const std::string_view expectedAnswer = "eth0";
    const std::uint8_t expectedChannel = 14;

    HandlerMock hMock;
    EXPECT_CALL(hMock, getEthDetails(""))
        .WillOnce(
            Return(std::make_tuple(expectedChannel, expectedAnswer.data())));

    auto reply = getEthDevice(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(EthDeviceReply) + expectedAnswer.size(), data.size());
    EXPECT_EQ(SysOEMCommands::SysGetEthDevice, result.first);
    EXPECT_EQ(expectedChannel, data[0]);
    EXPECT_EQ(expectedAnswer.size(), data[1]);
    EXPECT_EQ(
        expectedAnswer.data(),
        std::string(data.begin() + sizeof(struct EthDeviceReply), data.end()));
}

TEST(EthCommandTest, ValidPopulatedReturnsSuccess)
{
    std::vector<std::uint8_t> request = {'e'};
    const std::string_view expectedAnswer = "e";
    const std::uint8_t expectedChannel = 11;

    HandlerMock hMock;
    EXPECT_CALL(hMock, getEthDetails("e"))
        .WillOnce(
            Return(std::make_tuple(expectedChannel, expectedAnswer.data())));

    auto reply = getEthDevice(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(EthDeviceReply) + expectedAnswer.size(), data.size());
    EXPECT_EQ(SysOEMCommands::SysGetEthDevice, result.first);
    EXPECT_EQ(expectedChannel, data[0]);
    EXPECT_EQ(expectedAnswer.size(), data[1]);
    EXPECT_EQ(
        expectedAnswer.data(),
        std::string(data.begin() + sizeof(struct EthDeviceReply), data.end()));
}
} // namespace ipmi
} // namespace google
