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
#include "host_power_off.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

namespace google
{
namespace ipmi
{

TEST(PowerOffCommandTest, InvalidRequestLength)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysHostPowerOff};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              hostPowerOff(request.data(), reply, &dataLen, &hMock));
}

TEST(PowerOffCommandTest, ValidRequest)
{
    // Set the dealy to 15 mins
    std::uint32_t delayValue = 0x384;
    struct HostPowerOffRequest requestContents;
    requestContents.subcommand = SysOEMCommands::SysHostPowerOff;
    requestContents.delay = delayValue;

    std::vector<std::uint8_t> request(sizeof(requestContents));
    std::memcpy(request.data(), &requestContents, sizeof(requestContents));
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_CALL(hMock, hostPowerOffDelay(delayValue));
    EXPECT_EQ(IPMI_CC_OK,
              hostPowerOff(request.data(), reply, &dataLen, &hMock));
}

} // namespace ipmi
} // namespace google
