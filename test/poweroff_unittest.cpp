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
#include "host_power_off.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

TEST(PowerOffCommandTest, InvalidRequestLength)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysHostPowerOff};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              hostPowerOff(request, &hMock));
}

TEST(PowerOffCommandTest, ValidRequest)
{
    // Set the dealy to 15 mins
    std::uint32_t delayValue = 0x384;
    struct HostPowerOffRequest requestContents;
    requestContents.delay = delayValue;

    std::vector<std::uint8_t> request(sizeof(requestContents));
    std::memcpy(request.data(), &requestContents, sizeof(requestContents));

    HandlerMock hMock;
    EXPECT_CALL(hMock, hostPowerOffDelay(delayValue));

    auto reply = hostPowerOff(request, &hMock);
    auto result = ValidateReply(reply, false);
    EXPECT_EQ(SysOEMCommands::SysHostPowerOff, result.first);
}

} // namespace ipmi
} // namespace google
