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

#include <stdplus/gtest/tmp.hpp>

#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

using testing::_;
using ::testing::ElementsAre;

class BiosSettingTest : public stdplus::gtest::TestWithTmp
{
  public:
    std::string filename = std::format("{}/oem_bios_setting", CaseTmpDir());

    void writeTmpFile(std::vector<uint8_t> payload)
    {
        std::ofstream ofs;
        ofs.open(filename, std::ios::trunc | std::ios::binary);
        ofs.write(reinterpret_cast<char*>(payload.data()), payload.size());
        ofs.close();
    }
};

TEST_F(BiosSettingTest, NoOrEmptyFileRead)
{
    std::vector<uint8_t> request = {};

    HandlerMock hMock;
    EXPECT_EQ(::ipmi::responseRetBytesUnavailable(),
              readBiosSetting(request, &hMock));

    // Create an empty file
    {
        std::ofstream ofs;
        ofs.open(filename, std::ios::trunc | std::ios::binary);
        ofs.close();
    }
    EXPECT_EQ(::ipmi::responseRetBytesUnavailable(),
              readBiosSetting(request, &hMock, filename));
    std::remove(filename.c_str());
}

TEST_F(BiosSettingTest, SuccessfulRead)
{
    std::vector<uint8_t> request = {};
    // Ensure 0x0A which is a new line character '\n', is read properly
    std::vector<uint8_t> payload = {0x0A, 0xDE, 0xAD, 0xBE, 0xEF, 0x0A};
    std::vector<uint8_t> expectedReply = {6,    0x0A, 0xDE, 0xAD,
                                          0xBE, 0xEF, 0x0A};

    writeTmpFile(payload);

    HandlerMock hMock;
    auto reply = readBiosSetting(request, &hMock, filename);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(SysOEMCommands::SysReadBiosSetting, result.first);
    EXPECT_EQ(expectedReply.size() - 1, data.front());
    EXPECT_EQ(expectedReply, data);
    std::remove(filename.c_str());
}

TEST_F(BiosSettingTest, InvalidRequestWrite)
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

TEST_F(BiosSettingTest, SuccessfulWrite)
{
    std::vector<uint8_t> request = {0x02, 0xDE, 0xAD};

    // Write a dummy file to get around permission issues with CI
    // (Not needed in local CI)
    writeTmpFile({});
    HandlerMock hMock;
    auto reply = writeBiosSetting(request, &hMock, filename);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(SysOEMCommands::SysWriteBiosSetting, result.first);
    EXPECT_EQ(std::vector<uint8_t>{2}, data);

    // Validate the payload is correct
    reply = readBiosSetting(request, &hMock, filename);
    result = ValidateReply(reply);
    data = result.second;

    EXPECT_EQ(SysOEMCommands::SysReadBiosSetting, result.first);
    EXPECT_EQ(request.size() - 1, data.front());
    EXPECT_EQ(request, data);

    // Verify that we can write a shorter string and it'll replace the original
    // content of the file
    request = {0x01, 0x0A};

    reply = writeBiosSetting(request, &hMock, filename);
    result = ValidateReply(reply);
    data = result.second;

    EXPECT_EQ(SysOEMCommands::SysWriteBiosSetting, result.first);
    EXPECT_EQ(std::vector<uint8_t>{1}, data);

    // Validate the payload is correct
    reply = readBiosSetting(request, &hMock, filename);
    result = ValidateReply(reply);
    data = result.second;

    EXPECT_EQ(SysOEMCommands::SysReadBiosSetting, result.first);
    EXPECT_EQ(request.size() - 1, data.front());
    EXPECT_EQ(request, data);
    // Cleanup the settings file
    std::remove(filename.c_str());
}

} // namespace ipmi
} // namespace google
