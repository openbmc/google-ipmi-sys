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
#include "cpld.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"

#include <cstdint>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(CpldCommandTest, RequestTooSmall)
{
    std::vector<std::uint8_t> request = {};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              cpldVersion(request, &hMock));
}

TEST(CpldCommandTest, ValidRequestReturnsHappy)
{
    std::vector<std::uint8_t> request = {0x04};

    std::uint8_t expectedMaj = 0x5;
    std::uint8_t expectedMin = 0x3;
    std::uint8_t expectedPt = 0x7;
    std::uint8_t expectedSbPtr = 0x9;

    HandlerMock hMock;
    EXPECT_CALL(hMock, getCpldVersion(0x04))
        .WillOnce(Return(std::make_tuple(expectedMaj, expectedMin, expectedPt,
                                         expectedSbPtr)));

    // Reply is in the form of
    // std::tuple<ipmi::Cc, std::optional<std::tuple<RetTypes...>>>
    auto reply = cpldVersion(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(struct CpldReply), data.size());
    EXPECT_EQ(SysOEMCommands::SysCpldVersion, result.first);
    EXPECT_EQ(expectedMaj, data[0]);
    EXPECT_EQ(expectedMin, data[1]);
    EXPECT_EQ(expectedPt, data[2]);
    EXPECT_EQ(expectedSbPtr, data[3]);
}

} // namespace ipmi
} // namespace google
