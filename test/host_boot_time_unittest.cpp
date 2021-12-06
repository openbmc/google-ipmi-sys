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
#include "host_boot_time.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

TEST(HostBootTimeCommandTest, InvalidRequestLength)
{
    std::vector<std::uint8_t> request = {};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              saveHostBootTime(request, &hMock));
}

TEST(HostBootTimeCommandTest, InvalidStageCode)
{
    constexpr uint8_t kInvalidStageCode = 0xFF;

    std::vector<std::uint8_t> request(struct SaveHostBootTimeRequest);
    request[0] = kInvalidStageCode;
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseParmOutOfRange(),
              saveHostBootTime(request, &hMock));
}

TEST(HostBootTimeCommandTest, ValidRequest)
{
    constexpr uint64_t kDuration = 845296639; // ~= 14 minutes

    std::vector<std::uint8_t> request(sizeof(struct SaveHostBootTimeRequest));
    struct SaveHostBootTimeRequest requestContents;
    requestContents.stage =
        static_cast<uint8_t>(HostBootStageCode::kLinuxbootKernel);
    requestContents.duration_us = kDuration;
    memcpy(request.data(), &requestContents, sizeof(requestContents));

    HandlerMock hMock;
    EXPECT_CALL(hMock, saveHostBootTime(kDuration));

    auto reply = saveHostBootTime(request, &hMock);
    auto result = ValidateReply(reply, false);
    EXPECT_EQ(SysOEMCommands::SysHostBootTime, result.first);
}

} // namespace ipmi
} // namespace google
