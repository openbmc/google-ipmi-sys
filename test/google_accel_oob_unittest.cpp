// Copyright 2022 Google LLC
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
#include "errors.hpp"
#include "google_accel_oob.hpp"
#include "handler_mock.hpp"

#include <ipmid/api.h>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

using ::testing::_;
using ::testing::Return;

TEST(GoogleAccelOobTest, DeviceCount_Success)
{
    ::testing::StrictMock<HandlerMock> h;

    uint8_t reqBuf[1]; // Could be 0, but zero-length arrays are an extension

    struct Reply
    {
        uint32_t count;
    } __attribute__((packed));

    constexpr uint32_t kTestDeviceCount = 2;

    EXPECT_CALL(h, accelOobDeviceCount()).WillOnce(Return(kTestDeviceCount));

    Resp r = accelOobDeviceCount(reqBuf, &h);

    const auto response = std::get<0>(r);
    EXPECT_EQ(response, IPMI_CC_OK);

    const auto payload = std::get<1>(r);
    ASSERT_EQ(payload.has_value(), true);
    const auto payload_tuple = payload.value();
    const auto reply_cmd = std::get<0>(payload_tuple);
    EXPECT_EQ(reply_cmd, SysAccelOobDeviceCount);
    const auto reply_buff = std::get<1>(payload_tuple);
    ASSERT_EQ(reply_buff.size(), sizeof(Reply));

    auto* reply = reinterpret_cast<const Reply*>(reply_buff.data());
    EXPECT_EQ(reply->count, kTestDeviceCount);
}

TEST(GoogleAccelOobTest, DeviceName_Success)
{
    ::testing::StrictMock<HandlerMock> h;

    struct Request
    {
        uint32_t index;
    } __attribute__((packed));

    struct Reply
    {
        uint32_t index;
        uint8_t length;
        char name[1];
    } __attribute__((packed));

    constexpr uint32_t kTestDeviceIndex = 0;
    const std::string kTestDeviceName("testDeviceName");

    EXPECT_CALL(h, accelOobDeviceName(kTestDeviceIndex))
        .WillOnce(Return(kTestDeviceName));

    Request reqBuf{kTestDeviceIndex};
    Resp r = accelOobDeviceName(
        std::span(reinterpret_cast<const uint8_t*>(&reqBuf), sizeof(Request)),
        &h);

    const auto response = std::get<0>(r);
    EXPECT_EQ(response, IPMI_CC_OK);

    const auto payload = std::get<1>(r);
    ASSERT_EQ(payload.has_value(), true);
    const auto payload_tuple = payload.value();
    const auto reply_cmd = std::get<0>(payload_tuple);
    EXPECT_EQ(reply_cmd, SysAccelOobDeviceName);
    const auto reply_buff = std::get<1>(payload_tuple);
    ASSERT_GE(reply_buff.size(), sizeof(Reply));

    auto* reply = reinterpret_cast<const Reply*>(reply_buff.data());
    EXPECT_EQ(reply->index, kTestDeviceIndex);
    EXPECT_EQ(reply->length, kTestDeviceName.length());
    EXPECT_STREQ(reply->name, kTestDeviceName.c_str());
}

TEST(GoogleAccelOobTest, Read_Success)
{
    ::testing::StrictMock<HandlerMock> h;

    constexpr char kTestDeviceName[] = "testDeviceName";
    constexpr uint8_t kTestDeviceNameLength =
        (sizeof(kTestDeviceName) / sizeof(*kTestDeviceName)) - 1;
    constexpr uint8_t kTestToken = 0xAB;
    constexpr uint64_t kTestAddress = 0;
    constexpr uint8_t kTestReadSize = 8;
    constexpr uint64_t kTestData = 0x12345678;

    struct Request
    {
        uint8_t nameLength;
        char name[kTestDeviceNameLength];
        uint8_t token;
        uint64_t address;
        uint8_t num_bytes;
    } __attribute__((packed));

    struct Reply
    {
        uint8_t nameLength;
        char name[kTestDeviceNameLength];
        uint8_t token;
        uint64_t address;
        uint8_t num_bytes;
        uint64_t data;
    } __attribute__((packed));

    const std::string_view kTestDeviceNameStr(kTestDeviceName,
                                              kTestDeviceNameLength);
    EXPECT_CALL(h,
                accelOobRead(kTestDeviceNameStr, kTestAddress, kTestReadSize))
        .WillOnce(Return(kTestData));

    Request reqBuf{kTestDeviceNameLength, "", kTestToken, kTestAddress,
                   kTestReadSize};
    memcpy(reqBuf.name, kTestDeviceName, kTestDeviceNameLength);
    Resp r = accelOobRead(
        std::span(reinterpret_cast<const uint8_t*>(&reqBuf), sizeof(Request)),
        &h);

    const auto response = std::get<0>(r);
    EXPECT_EQ(response, IPMI_CC_OK);

    const auto payload = std::get<1>(r);
    ASSERT_EQ(payload.has_value(), true);
    const auto payload_tuple = payload.value();
    const auto reply_cmd = std::get<0>(payload_tuple);
    EXPECT_EQ(reply_cmd, SysAccelOobRead);
    const auto reply_buff = std::get<1>(payload_tuple);
    ASSERT_GE(reply_buff.size(), sizeof(Reply));

    auto* reply = reinterpret_cast<const Reply*>(reply_buff.data());
    EXPECT_EQ(reply->nameLength, kTestDeviceNameLength);
    EXPECT_EQ(std::string_view(reply->name, reply->nameLength),
              kTestDeviceNameStr);
    EXPECT_EQ(reply->token, kTestToken);
    EXPECT_EQ(reply->address, kTestAddress);
    EXPECT_EQ(reply->num_bytes, kTestReadSize);
    EXPECT_EQ(reply->data, kTestData);
}

TEST(GoogleAccelOobTest, Write_Success)
{
    ::testing::StrictMock<HandlerMock> h;

    constexpr char kTestDeviceName[] = "testDeviceName";
    constexpr uint8_t kTestDeviceNameLength =
        (sizeof(kTestDeviceName) / sizeof(*kTestDeviceName)) - 1;
    constexpr uint8_t kTestToken = 0xAB;
    constexpr uint64_t kTestAddress = 0;
    constexpr uint8_t kTestWriteSize = 8;
    constexpr uint64_t kTestData = 0x12345678;

    struct Request
    {
        uint8_t nameLength;
        char name[kTestDeviceNameLength];
        uint8_t token;
        uint64_t address;
        uint8_t num_bytes;
        uint64_t data;
    } __attribute__((packed));

    struct Reply
    {
        uint8_t nameLength;
        char name[kTestDeviceNameLength];
        uint8_t token;
        uint64_t address;
        uint8_t num_bytes;
        uint64_t data;
    } __attribute__((packed));

    const std::string_view kTestDeviceNameStr(kTestDeviceName,
                                              kTestDeviceNameLength);
    EXPECT_CALL(h, accelOobWrite(kTestDeviceNameStr, kTestAddress,
                                 kTestWriteSize, kTestData))
        .WillOnce(Return());

    Request reqBuf{kTestDeviceNameLength, "",       kTestToken, kTestAddress,
                   kTestWriteSize,        kTestData};
    memcpy(reqBuf.name, kTestDeviceName, kTestDeviceNameLength);
    Resp r = accelOobWrite(
        std::span(reinterpret_cast<const uint8_t*>(&reqBuf), sizeof(Request)),
        &h);

    const auto response = std::get<0>(r);
    EXPECT_EQ(response, IPMI_CC_OK);

    const auto payload = std::get<1>(r);
    ASSERT_EQ(payload.has_value(), true);
    const auto payload_tuple = payload.value();
    const auto reply_cmd = std::get<0>(payload_tuple);
    EXPECT_EQ(reply_cmd, SysAccelOobWrite);
    const auto reply_buff = std::get<1>(payload_tuple);
    ASSERT_GE(reply_buff.size(), sizeof(Reply));

    auto* reply = reinterpret_cast<const Reply*>(reply_buff.data());
    EXPECT_EQ(reply->nameLength, kTestDeviceNameLength);
    EXPECT_EQ(std::string_view(reply->name, reply->nameLength),
              kTestDeviceNameStr);
    EXPECT_EQ(reply->token, kTestToken);
    EXPECT_EQ(reply->address, kTestAddress);
    EXPECT_EQ(reply->num_bytes, kTestWriteSize);
    EXPECT_EQ(reply->data, kTestData);
}

TEST(GoogleAccelOobTest, SetVrSettings_Success)
{
    ::testing::StrictMock<HandlerMock> h;
    constexpr uint8_t kChipId = 2;
    constexpr uint8_t kSettingsId = 1;
    constexpr uint16_t kTestValue = 0xAABB;

    std::vector<uint8_t> testData = {kChipId, kSettingsId, 0xBB, 0xAA};

    EXPECT_CALL(h, accelSetVrSettings(kChipId, kSettingsId, kTestValue))
        .WillOnce(Return());

    Resp r = accelSetVrSettings(testData, &h);

    const auto response = std::get<0>(r);
    EXPECT_EQ(response, IPMI_CC_OK);

    const auto payload = std::get<1>(r);
    ASSERT_EQ(payload.has_value(), true);
    const auto payload_tuple = payload.value();
    const auto reply_cmd = std::get<0>(payload_tuple);
    EXPECT_EQ(reply_cmd, SysSetAccelVrSettings);
    const auto reply_buff = std::get<1>(payload_tuple);
    ASSERT_EQ(reply_buff.size(), 0);
}

TEST(GoogleAccelOobTest, SetVrSettings_HandleIncorrectDataSize)
{
    ::testing::StrictMock<HandlerMock> h;
    constexpr uint8_t kChipId = 2;
    uint8_t kSettingsId = 1;

    std::vector<uint8_t> testData = {kChipId, kSettingsId};

    EXPECT_CALL(h, accelSetVrSettings(_, _, _)).Times(0);

    Resp r = accelSetVrSettings(testData, &h);

    const auto response = std::get<0>(r);
    EXPECT_EQ(response, IPMI_CC_REQ_DATA_LEN_INVALID);

    const auto payload = std::get<1>(r);
    ASSERT_EQ(payload.has_value(), false);
}

TEST(GoogleAccelOobTest, GetVrSettings_Success)
{
    ::testing::StrictMock<HandlerMock> h;
    constexpr uint8_t kChipId = 3;
    constexpr uint8_t kSettingsId = 2;

    std::vector<uint8_t> testData = {kChipId, kSettingsId};

    EXPECT_CALL(h, accelGetVrSettings(kChipId, kSettingsId))
        .WillOnce(Return(0xAABB));

    Resp r = accelGetVrSettings(testData, &h);

    const auto response = std::get<0>(r);
    EXPECT_EQ(response, IPMI_CC_OK);

    const auto payload = std::get<1>(r);
    ASSERT_EQ(payload.has_value(), true);
    const auto payload_tuple = payload.value();
    const auto reply_cmd = std::get<0>(payload_tuple);
    EXPECT_EQ(reply_cmd, SysGetAccelVrSettings);
    const auto reply_buff = std::get<1>(payload_tuple);
    ASSERT_EQ(reply_buff.size(), 2);

    EXPECT_EQ(reply_buff.at(0), 0xBB);
    EXPECT_EQ(reply_buff.at(1), 0xAA);
}

TEST(GoogleAccelOobTest, GetVrSettings_HandleIncorrectDataSize)
{
    ::testing::StrictMock<HandlerMock> h;
    constexpr uint8_t kChipId = 2;
    uint8_t kSettingsId = 1;

    std::vector<uint8_t> testData = {kChipId, kSettingsId, 0xCC};

    EXPECT_CALL(h, accelGetVrSettings(_, _)).Times(0);

    Resp r = accelSetVrSettings(testData, &h);

    const auto response = std::get<0>(r);
    EXPECT_EQ(response, IPMI_CC_REQ_DATA_LEN_INVALID);

    const auto payload = std::get<1>(r);
    ASSERT_EQ(payload.has_value(), false);
}
} // namespace ipmi
} // namespace google
