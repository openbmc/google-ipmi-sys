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
#include "entity_name.hpp"
#include "handler_mock.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(EntityNameCommandTest, InvalidCommandLength)
{
    // GetEntityNameRequest is three bytes, let's send 2.
    std::vector<std::uint8_t> request = {SysOEMCommands::SysEntityName, 0x01};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              getEntityName(request.data(), reply, &dataLen, &hMock));
}

TEST(EntityNameCommandTest, ValidRequest)
{
    std::uint8_t entityId = 3;
    std::uint8_t entityInstance = 5;
    std::vector<std::uint8_t> request = {SysOEMCommands::SysEntityName,
                                         entityId, entityInstance};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];
    std::string entityName = "asdf";

    HandlerMock hMock;
    EXPECT_CALL(hMock, getEntityName(entityId, entityInstance))
        .WillOnce(Return(entityName));
    EXPECT_EQ(IPMI_CC_OK,
              getEntityName(request.data(), reply, &dataLen, &hMock));
    EXPECT_EQ(reply[1], entityName.length());
    EXPECT_EQ(0, std::memcmp(&reply[2], entityName.c_str(), reply[1]));
}

} // namespace ipmi
} // namespace google
