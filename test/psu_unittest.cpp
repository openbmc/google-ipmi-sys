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
#include "psu.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(PsuCommandTest, InvalidRequestLength)
{
    std::vector<std::uint8_t> request = {};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              psuHardReset(request, &hMock));
}

TEST(PsuCommandTest, ValidRequest)
{
    std::uint8_t delayValue = 0x45;
    struct PsuResetRequest requestContents;
    requestContents.delay = delayValue;
    std::vector<std::uint8_t> request(sizeof(requestContents));
    std::memcpy(request.data(), &requestContents, sizeof(requestContents));

    HandlerMock hMock;
    EXPECT_CALL(hMock, psuResetDelay(delayValue));

    auto reply = psuHardReset(request, &hMock);
    auto result = ValidateReply(reply, /*hasData=*/false);

    EXPECT_EQ(SysOEMCommands::SysPsuHardReset, result.first);
}

TEST(PsuResetOnShutdownCommandTest, ValidRequest)
{
    std::vector<std::uint8_t> request = {};

    HandlerMock hMock;
    EXPECT_CALL(hMock, psuResetOnShutdown());

    auto reply = psuHardResetOnShutdown(request, &hMock);
    auto result = ValidateReply(reply, /*hasData=*/false);

    EXPECT_EQ(SysOEMCommands::SysPsuHardResetOnShutdown, result.first);
}

} // namespace ipmi
} // namespace google
