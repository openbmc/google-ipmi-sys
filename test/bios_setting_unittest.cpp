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
    {
        stdplus::ManagedFd managedFd = stdplus::fd::open(
            filename, stdplus::fd::OpenFlags(stdplus::fd::OpenAccess::WriteOnly)
                          .set(stdplus::fd::OpenFlag::Trunc)
                          .set(stdplus::fd::OpenFlag::Create));
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

} // namespace ipmi
} // namespace google
