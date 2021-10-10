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

#include <cstdint>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(CpldCommandTest, RequestTooSmall)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysCpldVersion};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              cpldVersion(request.data(), reply, &dataLen, &hMock));
}

TEST(CpldCommandTest, ValidRequestReturnsHappy)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysCpldVersion, 0x04};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];
    std::uint8_t expectedMaj = 0x5;
    std::uint8_t expectedMin = 0x3;
    std::uint8_t expectedPt = 0x7;
    std::uint8_t expectedSbPtr = 0x9;

    HandlerMock hMock;
    EXPECT_CALL(hMock, getCpldVersion(0x04))
        .WillOnce(Return(std::make_tuple(expectedMaj, expectedMin, expectedPt,
                                         expectedSbPtr)));

    EXPECT_EQ(IPMI_CC_OK, cpldVersion(request.data(), reply, &dataLen, &hMock));
    EXPECT_EQ(expectedMaj, reply[1]);
    EXPECT_EQ(expectedMin, reply[2]);
    EXPECT_EQ(expectedPt, reply[3]);
    EXPECT_EQ(expectedSbPtr, reply[4]);
}

} // namespace ipmi
} // namespace google
