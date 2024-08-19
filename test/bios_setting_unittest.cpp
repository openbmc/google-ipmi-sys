// Copyright 2024 Google LLC
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

#include "bios_setting.hpp"
#include "commands.hpp"
#include "helper.hpp"

#include <stdplus/fd/create.hpp>
#include <stdplus/fd/managed.hpp>
#include <stdplus/fd/ops.hpp>

#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

using testing::_;
using ::testing::ElementsAre;

TEST(ReadBiosSettingTest, NoOrEmptyFileRead)
{
    std::vector<uint8_t> request = {};

    HandlerMock hMock;
    EXPECT_EQ(::ipmi::responseRetBytesUnavailable(),
              readBiosSetting(request, &hMock));

    {
        stdplus::ManagedFd managedFd = stdplus::fd::open(
            biosSettingPath,
            stdplus::fd::OpenFlags(stdplus::fd::OpenAccess::WriteOnly)
                .set(stdplus::fd::OpenFlag::Trunc)
                .set(stdplus::fd::OpenFlag::Create));
    }
    EXPECT_EQ(::ipmi::responseRetBytesUnavailable(),
              readBiosSetting(request, &hMock));
}

TEST(ReadBiosSettingTest, SuccessfulRead)
{
    std::vector<uint8_t> request = {};
    // Ensure 0x0A which is a new line character '\n', is read properly
    std::vector<uint8_t> payload = {0x0A, 0xDE, 0xAD, 0xBE, 0xEF, 0x0A};
    std::vector<uint8_t> expectedReply = {6,    0x0A, 0xDE, 0xAD,
                                          0xBE, 0xEF, 0x0A};
    {
        stdplus::ManagedFd managedFd = stdplus::fd::open(
            biosSettingPath,
            stdplus::fd::OpenFlags(stdplus::fd::OpenAccess::WriteOnly)
                .set(stdplus::fd::OpenFlag::Trunc)
                .set(stdplus::fd::OpenFlag::Create));
        stdplus::fd::writeExact(managedFd, payload);
    }

    HandlerMock hMock;
    auto reply = readBiosSetting(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(SysOEMCommands::SysReadBiosSetting, result.first);
    EXPECT_EQ(expectedReply.size() - 1, data.front());
    EXPECT_EQ(expectedReply, data);
    // Cleanup the settings file
    std::remove(biosSettingPath);
}

TEST(ReadBiosSettingTest, InvalidRequestWrite)
{
    // Empty request
    std::vector<uint8_t> request = {};

    HandlerMock hMock;
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              writeBiosSetting(request, &hMock));

    // Request with payload size 1 but no payload
    request = {0x01};
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              writeBiosSetting(request, &hMock));

    // Request with payload size 1 but actual payload size of 2 bytes
    request = {0x01, 0x02, 0x03};
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              writeBiosSetting(request, &hMock));

    // Request with payload size 2 but actual payload of 1 byte
    request = {0x02, 0x02};
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              writeBiosSetting(request, &hMock));
}

TEST(ReadBiosSettingTest, SuccessfulWrite)
{
    std::vector<uint8_t> request = {0x02, 0xDE, 0xAD};

    HandlerMock hMock;
    auto reply = writeBiosSetting(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(SysOEMCommands::SysWriteBiosSetting, result.first);
    EXPECT_EQ(std::vector<uint8_t>{2}, data);

    // Validate the payload is correct
    reply = readBiosSetting(request, &hMock);
    result = ValidateReply(reply);
    data = result.second;

    EXPECT_EQ(SysOEMCommands::SysReadBiosSetting, result.first);
    EXPECT_EQ(request.size() - 1, data.front());
    EXPECT_EQ(request, data);

    // Verify that we can write a shorter string and it'll replace the original
    // content of the file
    request = {0x01, 0x0A};

    reply = writeBiosSetting(request, &hMock);
    result = ValidateReply(reply);
    data = result.second;

    EXPECT_EQ(SysOEMCommands::SysWriteBiosSetting, result.first);
    EXPECT_EQ(std::vector<uint8_t>{1}, data);

    // Validate the payload is correct
    reply = readBiosSetting(request, &hMock);
    result = ValidateReply(reply);
    data = result.second;

    EXPECT_EQ(SysOEMCommands::SysReadBiosSetting, result.first);
    EXPECT_EQ(request.size() - 1, data.front());
    EXPECT_EQ(request, data);
    // Cleanup the settings file
    std::remove(biosSettingPath);
}

} // namespace ipmi
} // namespace google
