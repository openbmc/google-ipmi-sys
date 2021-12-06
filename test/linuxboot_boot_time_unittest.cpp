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
#include "linuxboot_boot_time.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

TEST(LinuxbootBootTimeCommandTest, InvalidRequestLength)
{
    std::vector<std::uint8_t> request = {};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              saveLinuxbootBootTime(request, &hMock));
}

TEST(LinuxbootBootTimeCommandTest, InvalidComponentCode)
{
    constexpr uint8_t kInvalidComponentCode = 0xFF;

    std::span<std::uint8_t> request(sizeof(struct SaveLinuxbootBootTimeRequest));
    request[0] = kInvalidComponentCode;
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseParmOutOfRange(),
              saveLinuxbootBootTime(request, &hMock));
}

TEST(LinuxbootBootTimeCommandTest, ValidRequest)
{
    constexpr uint64_t kDuration = 845296639; // ~= 14 minutes

    std::vector<std::uint8_t> request(sizeof(struct SaveLinuxbootBootTimeRequest));
    struct SaveLinuxbootBootTimeRequest requestContents;
    requestContents.component =
        static_cast<uint8_t>(ComponentCode::kLinuxbootKernel);
    requestContents.duration_us = kDuration;
    memcpy(request.data(), &requestContents, sizeof(requestContents));

    HandlerMock hMock;
    EXPECT_CALL(hMock, saveLinuxbootBootTime(requestContents.component, requestContents.duration_us));

    auto reply = saveLinuxbootBootTime(request, &hMock);
    auto result = ValidateReply(reply, false);
    EXPECT_EQ(SysOEMCommands::SysLinuxbootBootTime, result.first);
}

} // namespace ipmi
} // namespace google
