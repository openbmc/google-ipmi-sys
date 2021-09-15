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

#include "cable.hpp"
#include "commands.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"

#include <cstdint>
#include <cstring>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::StrEq;

namespace google
{
namespace ipmi
{

TEST(CableCommandTest, RequestTooSmall)
{
    std::vector<std::uint8_t> request = {};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(), cableCheck(request, &hMock));
}

TEST(CableCommandTest, FailsLengthSanityCheck)
{
    // Minimum is three bytes, but a length of zero for the string is invalid.
    std::vector<std::uint8_t> request = {0x00, 'a'};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(), cableCheck(request, &hMock));
}

TEST(CableCommandTest, LengthTooLongForPacket)
{
    // The length of a the string, as specified is longer than string provided.
    std::vector<std::uint8_t> request = {0x02, 'a'};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(), cableCheck(request, &hMock));
}

TEST(CableCommandTest, ValidRequestValidReturn)
{
    std::vector<std::uint8_t> request = {0x01, 'a'};

    HandlerMock hMock;

    EXPECT_CALL(hMock, getRxPackets(StrEq("a"))).WillOnce(Return(0));

    // Check results.
    struct CableReply expectedReply;
    expectedReply.value = 0;

    auto reply = cableCheck(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(struct CableReply), data.size());
    EXPECT_EQ(SysOEMCommands::SysCableCheck, result.first);
    EXPECT_EQ(expectedReply.value, data[0]);
}

} // namespace ipmi
} // namespace google
