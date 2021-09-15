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
#include "flash_size.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(FlashSizeCommandTest, ValidRequest)
{
    std::vector<std::uint8_t> request = {};
    uint32_t flashSize = 5422312; // 0x52BCE8

    HandlerMock hMock;
    EXPECT_CALL(hMock, getFlashSize()).WillOnce(Return(flashSize));

    auto reply = getFlashSize(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(struct GetFlashSizeReply), data.size());
    EXPECT_EQ(SysOEMCommands::SysGetFlashSize, result.first);
    EXPECT_EQ(0, data[3]);
    EXPECT_EQ(0x52, data[2]);
    EXPECT_EQ(0xBC, data[1]);
    EXPECT_EQ(0xE8, data[0]);
}

} // namespace ipmi
} // namespace google
