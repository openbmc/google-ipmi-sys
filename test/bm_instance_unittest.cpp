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

#include "bm_instance.hpp"
#include "commands.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

using testing::_;
using ::testing::Return;
using ::testing::StrEq;

TEST(GetBMInstancePropertyTest, InvalidRequestSize)
{
    std::vector<uint8_t> request = {};

    HandlerMock hMock;
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              getBMInstanceProperty(request, &hMock));
}

TEST(GetBMInstancePropertyTest, InvalidCommand)
{
    std::vector<uint8_t> request = {1};
    std::string expectedOutput(64, 'a');

    HandlerMock hMock;
    EXPECT_CALL(hMock, getBMInstanceProperty(1))
        .WillOnce(Return(expectedOutput));
    EXPECT_EQ(::ipmi::responseInvalidCommand(),
              getBMInstanceProperty(request, &hMock));
}

TEST(GetBMInstancePropertyTest, ValidRequest)
{
    std::vector<uint8_t> request = {2};
    std::string expectedOutput = "asdf";

    HandlerMock hMock;
    EXPECT_CALL(hMock, getBMInstanceProperty(2))
        .WillOnce(Return(expectedOutput));

    auto reply = getBMInstanceProperty(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(struct BMInstancePropertyReply) + expectedOutput.size(),
              data.size());
    EXPECT_EQ(SysOEMCommands::SysGetBMInstanceProperty, result.first);
    EXPECT_THAT(std::string(data.begin() + 1, data.end()),
                StrEq(expectedOutput));
}

} // namespace ipmi
} // namespace google
