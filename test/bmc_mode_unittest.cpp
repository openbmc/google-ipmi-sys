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

#include "bmc_mode.hpp"
#include "commands.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"

#include <vector>

#include <gtest/gtest.h>

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(BmcModeCommandTest, ValidRequest)
{
    std::vector<std::uint8_t> request = {};
    uint8_t bmcMode = 0; // Non Bare Metal Mode

    HandlerMock hMock;
    EXPECT_CALL(hMock, getBmcMode()).WillOnce(Return(bmcMode));

    auto reply = getBmcMode(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(1, data.size());
    EXPECT_EQ(SysOEMCommands::SysGetBmcMode, result.first);
    EXPECT_EQ(0, data[0]);

    bmcMode = 1; // Bare Metal Mode

    EXPECT_CALL(hMock, getBmcMode()).WillOnce(Return(bmcMode));

    reply = getBmcMode(request, &hMock);
    result = ValidateReply(reply);
    data = result.second;

    EXPECT_EQ(1, data.size());
    EXPECT_EQ(SysOEMCommands::SysGetBmcMode, result.first);
    EXPECT_EQ(1, data[0]);

    bmcMode = 2; // Bare Metal Cleaning Mode

    EXPECT_CALL(hMock, getBmcMode()).WillOnce(Return(bmcMode));

    reply = getBmcMode(request, &hMock);
    result = ValidateReply(reply);
    data = result.second;

    EXPECT_EQ(1, data.size());
    EXPECT_EQ(SysOEMCommands::SysGetBmcMode, result.first);
    EXPECT_EQ(2, data[0]);
}

} // namespace ipmi
} // namespace google
