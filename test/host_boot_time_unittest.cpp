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
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

using ::testing::Return;
using ::testing::Throw;

struct HostBootTimeSetDurationRequestFixedNameLen4
{
    uint8_t length; // header
    char name[4];
    uint64_t duration_ms; // tailer
} __attribute__((packed));

TEST(HostBootTimeSetDurationTest, EmptyRequest)
{
    HandlerMock hMock;
    std::vector<uint8_t> request = {};
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              hostBootTimeSetDuration(request, &hMock));
}

TEST(HostBootTimeSetDurationTest, EmptyDurationName)
{
    HandlerMock hMock;
    std::vector<uint8_t> request(
        sizeof(struct HostBootTimeSetDurationRequestHeaderTailer));
    HostBootTimeSetDurationRequestHeaderTailer r = {.length = 0,
                                                    .duration_ms = 900000};
    memcpy(request.data(), &r,
           sizeof(struct HostBootTimeSetDurationRequestHeaderTailer));
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              hostBootTimeSetDuration(request, &hMock));
}

TEST(HostBootTimeSetDurationTest, InvalidDurationName)
{
    HandlerMock hMock;
    std::vector<uint8_t> request(
        sizeof(struct HostBootTimeSetDurationRequestFixedNameLen4));
    HostBootTimeSetDurationRequestFixedNameLen4 r = {
        .length = 4, .name = "+-*", .duration_ms = 900000};
    memcpy(request.data(), &r,
           sizeof(struct HostBootTimeSetDurationRequestFixedNameLen4));
    EXPECT_EQ(::ipmi::responseParmOutOfRange(),
              hostBootTimeSetDuration(request, &hMock));
}

TEST(HostBootTimeSetDurationTest, SetDurationThrowException)
{
    HandlerMock hMock;
    std::vector<uint8_t> request(
        sizeof(struct HostBootTimeSetDurationRequestFixedNameLen4));
    HostBootTimeSetDurationRequestFixedNameLen4 r = {
        .length = 4, .name = "AAA", .duration_ms = 900000};
    memcpy(request.data(), &r,
           sizeof(struct HostBootTimeSetDurationRequestFixedNameLen4));
    request[4] = 'A';

    EXPECT_CALL(hMock, hostBootTimeSetDuration("AAAA" r.duration_ms))
        .WillOnce(Throw(IpmiException(IPMI_CC_UNSPECIFIED_ERROR)));
    EXPECT_EQ(::ipmi::response(
                  IpmiException(IPMI_CC_UNSPECIFIED_ERROR).getIpmiError()),
              hostBootTimeSetDuration(request, &hMock));
}

TEST(HostBootTimeSetDurationTest, Success)
{
    HandlerMock hMock;
    std::vector<uint8_t> request(
        sizeof(struct HostBootTimeSetDurationRequestFixedNameLen4));
    HostBootTimeSetDurationRequestFixedNameLen4 r = {
        .length = 4, .name = "AAA", .duration_ms = 900000};
    memcpy(request.data(), &r,
           sizeof(struct HostBootTimeSetDurationRequestFixedNameLen4));
    request[4] = 'A';

    std::vector<uint8_t> reply(sizeof(struct HostBootTimeSetDurationReply));
    reply[0] = 1;

    EXPECT_CALL(hMock, hostBootTimeSetDuration("AAAA" r.duration_ms))
        .WillOnce(Return(1));
    EXPECT_EQ(::ipmi::responseSuccess(
                  SysOEMCommands::SysHostBootTimeSetDuration, reply),
              hostBootTimeSetDuration(request, &hMock));
}

TEST(HostBootTimeNotifyTest, InvalidRequestLength)
{
    HandlerMock hMock;
    std::vector<uint8_t> request = {};
    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              hostBootTimeNotify(request, &hMock));
}

TEST(HostBootTimeNotifyTest, CheckpointOutOfRange)
{
    HandlerMock hMock;
    std::vector<uint8_t> request(sizeof(struct HostBootTimeNotifyRequest));
    HostBootTimeNotifyRequest r = {.checkpointCode = 10};
    memcpy(request.data(), &r, sizeof(struct HostBootTimeNotifyRequest));
    EXPECT_EQ(::ipmi::responseParmOutOfRange(),
              hostBootTimeNotify(request, &hMock));
}

TEST(HostBootTimeNotifyTest, CheckpointWrongOrder)
{
    HandlerMock hMock;
    std::vector<uint8_t> request(sizeof(struct HostBootTimeNotifyRequest));
    HostBootTimeNotifyRequest r = {.checkpointCode = 1};
    memcpy(request.data(), &r, sizeof(struct HostBootTimeNotifyRequest));
    EXPECT_CALL(hMock, hostBootTimeNotify(r.checkpointCode))
        .WillOnce(Return(std::nullopt));
    EXPECT_EQ(::ipmi::responseParmOutOfRange(),
              hostBootTimeNotify(request, &hMock));
}

TEST(HostBootTimeNotifyTest, NotifyThrowException)
{
    HandlerMock hMock;
    std::vector<uint8_t> request(sizeof(struct HostBootTimeNotifyRequest));
    HostBootTimeNotifyRequest r = {.checkpointCode = 1};
    memcpy(request.data(), &r, sizeof(struct HostBootTimeNotifyRequest));
    EXPECT_CALL(hMock, hostBootTimeNotify(r.checkpointCode))
        .WillOnce(Throw(IpmiException(IPMI_CC_UNSPECIFIED_ERROR)));
    EXPECT_EQ(::ipmi::response(
                  IpmiException(IPMI_CC_UNSPECIFIED_ERROR).getIpmiError()),
              hostBootTimeNotify(request, &hMock));
}

TEST(HostBootTimeNotifyTest, Success)
{
    HandlerMock hMock;
    std::vector<uint8_t> request(sizeof(struct HostBootTimeNotifyRequest));
    HostBootTimeNotifyRequest r = {.checkpointCode = 1};
    memcpy(request.data(), &r, sizeof(struct HostBootTimeNotifyRequest));

    std::vector<uint8_t> reply(sizeof(struct HostBootTimeNotifyReply));
    HostBootTimeNotifyReply rr = {.timetamp_ms = 900000};
    memcpy(reply.data(), &rr, sizeof(struct HostBootTimeNotifyReply));

    EXPECT_CALL(hMock, hostBootTimeNotify(r.checkpointCode))
        .WillOnce(Return(rr.timetamp_ms));
    EXPECT_EQ(
        ::ipmi::responseSuccess(SysOEMCommands::SysHostBootTimeNotify, reply));
}

} // namespace ipmi
} // namespace google
