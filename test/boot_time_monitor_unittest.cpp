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

#include "boot_time_monitor.hpp"
#include "commands.hpp"
#include "errors.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

using ::testing::Return;
using ::testing::Throw;

constexpr uint32_t cpHeadLen = sizeof(struct CheckpointReqHeader);
constexpr uint32_t durHeadLen = sizeof(struct DurationReqHeader);

TEST(SendRebootCheckpointTest, RequestLenTest)
{
    HandlerMock hMock;
    std::vector<uint8_t> request = {};

    // Empty request test
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              sendRebootCheckpoint(request, &hMock));

    constexpr std::string_view kName = "GoodName";
    constexpr uint32_t kExtraSize = 1;
    CheckpointReqHeader r = {
        .wallTime = 0,
        .duration = 0,
        .length = kName.size(),
    };
    request.resize(cpHeadLen + kName.size());

    // Test if request len is correct.
    // Expect OK
    memcpy(request.data(), &r, cpHeadLen);
    memcpy(request.data() + cpHeadLen, kName.data(), r.length);
    EXPECT_NO_THROW(sendRebootCheckpoint(request, &hMock));

    // Test if request is too long
    // Expect OK
    r.length = kName.size() - kExtraSize;
    memcpy(request.data(), &r, cpHeadLen);
    memcpy(request.data() + cpHeadLen, kName.data(), r.length);
    EXPECT_NO_THROW(sendRebootCheckpoint(request, &hMock));

    // Test if request is too short
    // Expect Error
    r.length = kName.size() + kExtraSize;
    memcpy(request.data(), &r, cpHeadLen);
    memcpy(request.data() + cpHeadLen, kName.data(), kName.size());
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              sendRebootCheckpoint(request, &hMock));
}

TEST(SendRebootCheckpointTest, InvalidName)
{
    HandlerMock hMock;
    constexpr std::string_view kName = "+BadName-";
    std::vector<uint8_t> request(cpHeadLen + kName.size());

    CheckpointReqHeader r = {
        .wallTime = 0,
        .duration = 0,
        .length = kName.size(),
    };
    memcpy(request.data(), &r, cpHeadLen);
    memcpy(request.data() + cpHeadLen, kName.data(), kName.size());
    EXPECT_EQ(::ipmi::responseParmOutOfRange(),
              sendRebootCheckpoint(request, &hMock));
}

TEST(SendRebootCheckpointTest, DBusCallThrow)
{
    HandlerMock hMock;
    constexpr std::string_view kName = "GoodName";
    std::vector<uint8_t> request(cpHeadLen + kName.size());

    CheckpointReqHeader r = {
        .wallTime = 0,
        .duration = 0,
        .length = kName.size(),
    };
    memcpy(request.data(), &r, cpHeadLen);
    memcpy(request.data() + cpHeadLen, kName.data(), kName.size());

    EXPECT_CALL(hMock, sendRebootCheckpoint(kName, r.wallTime, r.duration))
        .WillOnce(Throw(IpmiException(::ipmi::ccUnspecifiedError)));
    EXPECT_EQ(::ipmi::response(
                  IpmiException(::ipmi::ccUnspecifiedError).getIpmiError()),
              sendRebootCheckpoint(request, &hMock));
}

TEST(SendRebootComplete, Success)
{
    HandlerMock hMock;

    EXPECT_NO_THROW(sendRebootComplete(std::vector<uint8_t>{}, &hMock));
}

TEST(SendRebootComplete, DBusCallThrow)
{
    HandlerMock hMock;

    EXPECT_CALL(hMock, sendRebootComplete())
        .WillOnce(Throw(IpmiException(::ipmi::ccUnspecifiedError)));
    EXPECT_EQ(::ipmi::response(
                  IpmiException(::ipmi::ccUnspecifiedError).getIpmiError()),
              sendRebootComplete(std::vector<uint8_t>{}, &hMock));
}

TEST(SendRebootAdditionalDurationTest, RequestLenTest)
{
    HandlerMock hMock;
    std::vector<uint8_t> request = {};

    // Empty request test
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              sendRebootAdditionalDuration(request, &hMock));

    constexpr std::string_view kName = "GoodName";
    constexpr uint32_t kExtraSize = 1;
    DurationReqHeader r = {
        .duration = 0,
        .length = kName.size(),
    };
    request.resize(durHeadLen + kName.size());

    // Test if request len is correct.
    // Expect OK
    memcpy(request.data(), &r, durHeadLen);
    memcpy(request.data() + durHeadLen, kName.data(), r.length);
    EXPECT_NO_THROW(sendRebootAdditionalDuration(request, &hMock));

    // Test if request is too long
    // Expect OK
    r.length = kName.size() - kExtraSize;
    memcpy(request.data(), &r, durHeadLen);
    memcpy(request.data() + durHeadLen, kName.data(), r.length);
    EXPECT_NO_THROW(sendRebootAdditionalDuration(request, &hMock));

    // Test if request is too short
    // Expect Error
    r.length = kName.size() + kExtraSize;
    memcpy(request.data(), &r, durHeadLen);
    memcpy(request.data() + durHeadLen, kName.data(), kName.size());
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              sendRebootAdditionalDuration(request, &hMock));
}

TEST(SendRebootAdditionalDurationTest, InvalidName)
{
    HandlerMock hMock;
    constexpr std::string_view kName = "+BadName-";
    std::vector<uint8_t> request(durHeadLen + kName.size());

    DurationReqHeader r = {
        .duration = 0,
        .length = kName.size(),
    };
    memcpy(request.data(), &r, durHeadLen);
    memcpy(request.data() + durHeadLen, kName.data(), kName.size());
    EXPECT_EQ(::ipmi::responseParmOutOfRange(),
              sendRebootAdditionalDuration(request, &hMock));
}

TEST(SendRebootAdditionalDurationTest, DBusCallThrow)
{
    HandlerMock hMock;
    constexpr std::string_view kName = "GoodName";
    std::vector<uint8_t> request(durHeadLen + kName.size());

    DurationReqHeader r = {
        .duration = 0,
        .length = kName.size(),
    };
    memcpy(request.data(), &r, durHeadLen);
    memcpy(request.data() + durHeadLen, kName.data(), kName.size());

    EXPECT_CALL(hMock, sendRebootAdditionalDuration(kName, r.duration))
        .WillOnce(Throw(IpmiException(::ipmi::ccUnspecifiedError)));
    EXPECT_EQ(::ipmi::response(
                  IpmiException(::ipmi::ccUnspecifiedError).getIpmiError()),
              sendRebootAdditionalDuration(request, &hMock));
}

} // namespace ipmi
} // namespace google
