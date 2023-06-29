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

#include "commands.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"
#include "linux_boot_done.hpp"

#include <vector>

#include <gtest/gtest.h>

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(LinuxBootDoneCommandTest, ValidResponseTest)
{
    std::vector<std::uint8_t> request = {};

    HandlerMock hMock;
    EXPECT_CALL(hMock, linuxBootDone()).Times(1);

    auto reply = linuxBootDone(request, &hMock);
    auto result = ValidateReply(reply, false);
    auto& data = result.second;

    EXPECT_EQ(0, data.size());
    EXPECT_EQ(SysOEMCommands::SysLinuxBootDone, result.first);
}

} // namespace ipmi
} // namespace google
