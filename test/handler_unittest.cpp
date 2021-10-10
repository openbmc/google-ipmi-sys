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

#include "config.h"

#include "bifurcation_mock.hpp"
#include "errors.hpp"
#include "handler.hpp"
#include "handler_impl.hpp"
#include "handler_mock.hpp"

#include <systemd/sd-bus.h>

#include <nlohmann/json.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/test/sdbus_mock.hpp>
#include <stdplus/print.hpp>

#include <charconv>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <string>
#include <tuple>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

using testing::_;
using testing::ElementsAre;
using testing::Return;

TEST(HandlerTest, EthCheckValidHappy)
{
    Handler h;
    std::tuple<std::uint8_t, std::string> result = h.getEthDetails("et");
    EXPECT_EQ(12, std::get<0>(result));
    EXPECT_STREQ("et", std::get<1>(result).c_str());
}

TEST(HandlerTest, CableCheckIllegalPath)
{
    Handler h;
    EXPECT_THROW(h.getRxPackets("eth0/../../"), IpmiException);
}

TEST(HandlerTest, readNameFromConfigInstanceVariety)
{
    // Make sure it handles the failures and successes as we expect.
    struct testCase
    {
        std::string type;
        std::uint8_t instance;
        std::string expectedName;
    };

    std::vector<testCase> tests = {
        {"cpu", 5, ""},
        {"cpu", 3, "CPU2"},
    };

    auto j2 = R"(
      {
        "cpu": [
          {"instance": 1, "name": "CPU0"},
          {"instance": 2, "name": "CPU1"},
          {"instance": 3, "name": "CPU2"},
          {"instance": 4, "name": "CPU3"}
        ]
      }
    )"_json;

    for (const auto& test : tests)
    {
        EXPECT_STREQ(test.expectedName.c_str(),
                     readNameFromConfig(test.type, test.instance, j2).c_str());
    }
}

// TODO: If we can test with phosphor-logging in the future, there are more
// failure cases.

TEST(HandlerTest, getEntityNameWithNameNotFoundExcepts)
{
    const char* testFilename = "test.json";
    std::string contents = R"({"cpu": [{"instance": 1, "name": "CPU0"}]})";
    std::ofstream outputJson(testFilename);
    outputJson << contents;
    outputJson.flush();
    outputJson.close();

    Handler h(testFilename);
    EXPECT_THROW(h.getEntityName(0x03, 2), IpmiException);
    (void)std::remove(testFilename);
}

TEST(HandlerTest, getEntityNameWithNameFoundReturnsIt)
{
    const char* testFilename = "test.json";
    std::string contents = R"({"cpu": [{"instance": 1, "name": "CPU0"}]})";
    std::ofstream outputJson(testFilename);
    outputJson << contents;
    outputJson.flush();
    outputJson.close();

    Handler h(testFilename);
    EXPECT_STREQ("CPU0", h.getEntityName(0x03, 1).c_str());
    (void)std::remove(testFilename);
}

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::ContainerEq;
using ::testing::DoAll;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsNull;
using ::testing::MatcherCast;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::SafeMatcherCast;
using ::testing::SetArgPointee;
using ::testing::StrEq;
using ::testing::StrictMock;
using ::testing::StrNe;
using ::testing::WithArg;

class MockDbusHandler : public Handler
{
  public:
    MockDbusHandler(sdbusplus::SdBusMock& mock,
                    const std::string& config = "") :
        Handler(config),
        mock_(&mock)
    {}

  protected:
    sdbusplus::bus_t getDbus() const override
    {
        return sdbusplus::get_mocked_new(
            const_cast<sdbusplus::SdBusMock*>(mock_));
    }

  private:
    sdbusplus::SdBusMock* mock_;
};

ACTION_TEMPLATE(AssignReadVal, HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_1_VALUE_PARAMS(val))
{
    *static_cast<T*>(arg2) = val;
}

ACTION_P(TraceDbus, msg)
{
    stdplus::print(stderr, "{}\n", msg);
}

ACTION_P(TraceDbus2, msg)
{
    stdplus::print(stderr, "{}({:02x})\n", msg,
                   *static_cast<const uint8_t*>(arg2));
}

constexpr char object_path[] = "/com/google/customAccel/test/path";
constexpr char property_grpc[] = "com.google.custom_accel.gRPC";
constexpr char value_port[] = "Port";
constexpr uint32_t port = 5000;

constexpr char SD_BUS_TYPE_BYTE_STR[] = {SD_BUS_TYPE_BYTE, '\0'};

// Returns an object that looks like:
//     "/com/google/customAccel/test/path": {
//         "com.google.custom_accel.gRPC" : {
//             "Port" : {
//                 "type" : "u",
//                 "data" : 5000
//             }
//         }
//     }
void ExpectGetManagedObjects(StrictMock<sdbusplus::SdBusMock>& mock,
                             const char* obj_path = object_path)
{
    ::testing::InSequence s;

    // These must be nullptr or sd_bus_message_unref will seg fault.
    constexpr sd_bus_message* method = nullptr;
    constexpr sd_bus_message* msg = nullptr;

    EXPECT_CALL(mock, sd_bus_message_new_method_call(
                          _,         // sd_bus *bus,
                          NotNull(), // sd_bus_message **m
                          StrEq("com.google.custom_accel"), StrEq("/"),
                          StrEq("org.freedesktop.DBus.ObjectManager"),
                          StrEq("GetManagedObjects")))
        .WillOnce(DoAll(SetArgPointee<1>(method), Return(0)));

    EXPECT_CALL(mock, sd_bus_call(_,           // sd_bus *bus,
                                  method,      // sd_bus_message *m
                                  _,           // uint64_t timeout
                                  NotNull(),   // sd_bus_error *ret_error
                                  NotNull()))  // sd_bus_message **reply
        .WillOnce(DoAll(SetArgPointee<3>(SD_BUS_ERROR_NULL),
                        SetArgPointee<4>(msg), // reply
                        Return(0)));

    EXPECT_CALL(mock, sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY,
                                                     StrEq("{oa{sa{sv}}}")))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0)).WillOnce(Return(0));

    EXPECT_CALL(mock, sd_bus_message_enter_container(
                          msg, SD_BUS_TYPE_DICT_ENTRY, StrEq("oa{sa{sv}}")))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_read_basic(msg, SD_BUS_TYPE_OBJECT_PATH,
                                                NotNull()))
        .WillOnce(DoAll(AssignReadVal<const char*>(obj_path), Return(1)));

    EXPECT_CALL(mock, sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY,
                                                     StrEq("{sa{sv}}")))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0)).WillOnce(Return(0));

    EXPECT_CALL(mock, sd_bus_message_enter_container(
                          msg, SD_BUS_TYPE_DICT_ENTRY, StrEq("sa{sv}")))
        .WillOnce(Return(1));

    EXPECT_CALL(mock,
                sd_bus_message_read_basic(msg, SD_BUS_TYPE_STRING, NotNull()))
        .WillOnce(DoAll(AssignReadVal<const char*>(property_grpc), Return(1)));

    EXPECT_CALL(mock, sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY,
                                                     StrEq("{sv}")))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0)).WillOnce(Return(0));

    EXPECT_CALL(mock, sd_bus_message_enter_container(
                          msg, SD_BUS_TYPE_DICT_ENTRY, StrEq("sv")))
        .WillOnce(Return(1));

    EXPECT_CALL(mock,
                sd_bus_message_read_basic(msg, SD_BUS_TYPE_STRING, NotNull()))
        .WillOnce(DoAll(AssignReadVal<const char*>(value_port), Return(1)));

    EXPECT_CALL(
        mock, sd_bus_message_verify_type(msg, SD_BUS_TYPE_VARIANT, StrNe("u")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    EXPECT_CALL(
        mock, sd_bus_message_verify_type(msg, SD_BUS_TYPE_VARIANT, StrEq("u")))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_enter_container(msg, SD_BUS_TYPE_VARIANT,
                                                     StrEq("u")))
        .WillOnce(Return(1));

    EXPECT_CALL(mock,
                sd_bus_message_read_basic(msg, SD_BUS_TYPE_UINT32, NotNull()))
        .WillOnce(DoAll(AssignReadVal<uint32_t>(port), Return(0)));

    EXPECT_CALL(
        mock, sd_bus_message_verify_type(msg, SD_BUS_TYPE_VARIANT, StrNe("u")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    EXPECT_CALL(mock, sd_bus_message_exit_container(msg))
        .WillOnce(Return(1))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0)).WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_exit_container(msg))
        .WillOnce(Return(1))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0)).WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_exit_container(msg))
        .WillOnce(Return(1))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0)).WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_exit_container(msg)).WillOnce(Return(1));
}

void ExpectSdBusError(StrictMock<sdbusplus::SdBusMock>& mock,
                      const std::string& service, const std::string& objPath,
                      const std::string& interface, const std::string& function)
{
    EXPECT_CALL(
        mock, sd_bus_message_new_method_call(_,         // sd_bus *bus,
                                             NotNull(), // sd_bus_message **m
                                             StrEq(service), StrEq(objPath),
                                             StrEq(interface), StrEq(function)))
        .WillOnce(Return(-ENOTCONN));
}

TEST(HandlerTest, accelOobDeviceCount_Success)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);
    ExpectGetManagedObjects(mock);
    EXPECT_EQ(1, h.accelOobDeviceCount());
}

TEST(HandlerTest, accelOobDeviceCount_Fail)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);
    ExpectSdBusError(mock, "com.google.custom_accel", "/",
                     "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    EXPECT_THROW(h.accelOobDeviceCount(), IpmiException);
}

TEST(HandlerTest, accelOobDeviceName_Success)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);
    ExpectGetManagedObjects(mock);
    EXPECT_EQ(std::string("test/path"), h.accelOobDeviceName(0));
}

TEST(HandlerTest, accelOobDeviceName_Fail)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);
    ExpectSdBusError(mock, "com.google.custom_accel", "/",
                     "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    EXPECT_THROW(h.accelOobDeviceName(0), IpmiException);
}

TEST(HandlerTest, accelOobDeviceName_OutOfRange)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);
    ExpectGetManagedObjects(mock);
    EXPECT_THROW(h.accelOobDeviceName(1), IpmiException);
}

TEST(HandlerTest, accelOobDeviceName_InvalidName)
{
    constexpr char bad_object_path[] = "/com/google/customAccel2/bad/path";
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);
    ExpectGetManagedObjects(mock, bad_object_path);
    EXPECT_THROW(h.accelOobDeviceName(0), IpmiException);
}

constexpr uint8_t NUM_BYTES_RETURNED_EQ_NUM_BYTES = 0xff;
void ExpectRead(StrictMock<sdbusplus::SdBusMock>& mock, uint64_t address,
                uint8_t num_bytes, uint64_t data, int sd_bus_call_return_value,
                uint8_t num_bytes_returned = NUM_BYTES_RETURNED_EQ_NUM_BYTES)
{
    ::testing::InSequence s;

    // These must be nullptr or sd_bus_message_unref will seg fault.
    constexpr sd_bus_message* method = nullptr;
    constexpr sd_bus_message* msg = nullptr;

    EXPECT_CALL(mock, sd_bus_message_new_method_call(
                          _,         // sd_bus *bus,
                          NotNull(), // sd_bus_message **m
                          StrEq("com.google.custom_accel"),
                          StrEq("/com/google/customAccel/test/path"),
                          StrEq("com.google.custom_accel.BAR"), StrEq("Read")))
        .WillOnce(DoAll(SetArgPointee<1>(method), Return(0)));

    EXPECT_CALL(
        mock, sd_bus_message_append_basic(
                  method, SD_BUS_TYPE_UINT64,
                  MatcherCast<const void*>(
                      SafeMatcherCast<const uint64_t*>(Pointee(Eq(address))))))
        .WillOnce(Return(1));

    EXPECT_CALL(mock,
                sd_bus_message_append_basic(
                    method, SD_BUS_TYPE_UINT64,
                    MatcherCast<const void*>(SafeMatcherCast<const uint64_t*>(
                        Pointee(Eq<uint64_t>(num_bytes))))))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_call(_,           // sd_bus *bus,
                                  method,      // sd_bus_message *m
                                  _,           // uint64_t timeout
                                  NotNull(),   // sd_bus_error *ret_error
                                  NotNull()))  // sd_bus_message **reply
        .WillOnce(DoAll(SetArgPointee<3>(SD_BUS_ERROR_NULL),
                        SetArgPointee<4>(msg), // reply
                        Return(sd_bus_call_return_value)));

    if (sd_bus_call_return_value >= 0)
    {
        EXPECT_CALL(mock,
                    sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY,
                                                   StrEq(SD_BUS_TYPE_BYTE_STR)))
            .WillOnce(Return(1));

        if (num_bytes_returned == NUM_BYTES_RETURNED_EQ_NUM_BYTES)
        {
            num_bytes_returned = num_bytes;
        }
        for (auto i = num_bytes_returned - 1; i >= 0; --i)
        {
            EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0))
                .WillOnce(Return(0));

            const uint8_t byte = (i >= 8) ? 0 : (data >> (8 * i)) & 0xff;
            EXPECT_CALL(mock, sd_bus_message_read_basic(msg, SD_BUS_TYPE_BYTE,
                                                        NotNull()))
                .WillOnce(DoAll(AssignReadVal<uint8_t>(byte), Return(1)));
        }

        EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0)).WillOnce(Return(1));

        EXPECT_CALL(mock, sd_bus_message_exit_container(msg))
            .WillOnce(Return(1));
    }
}

TEST(HandlerTest, accelOobRead_Success)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);

    constexpr uint64_t address = 0x123456789abcdef;
    constexpr uint8_t num_bytes = sizeof(uint64_t);
    constexpr int sd_bus_call_return_value = 1;
    constexpr uint64_t data = 0x13579bdf02468ace;

    ExpectRead(mock, address, num_bytes, data, sd_bus_call_return_value);
    EXPECT_EQ(data, h.accelOobRead("test/path", address, num_bytes));
}

TEST(HandlerTest, accelOobRead_Fail)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);

    constexpr uint64_t address = 0x123456789abcdef;
    constexpr uint8_t num_bytes = sizeof(uint64_t);
    constexpr int sd_bus_call_return_value = -ENOTCONN;
    constexpr uint64_t data = 0x13579bdf02468ace;

    ExpectRead(mock, address, num_bytes, data, sd_bus_call_return_value);
    EXPECT_THROW(h.accelOobRead("test/path", address, num_bytes),
                 IpmiException);
}

TEST(HandlerTest, accelOobRead_TooFewBytesReturned)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);

    constexpr uint64_t address = 0x123456789abcdef;
    constexpr uint8_t num_bytes = sizeof(uint64_t);
    constexpr int sd_bus_call_return_value = 1;
    constexpr uint64_t data = 0x13579bdf02468ace;
    constexpr uint8_t num_bytes_returned = num_bytes - 1;

    ExpectRead(mock, address, num_bytes, data, sd_bus_call_return_value,
               num_bytes_returned);
    EXPECT_THROW(h.accelOobRead("test/path", address, num_bytes),
                 IpmiException);
}

TEST(HandlerTest, accelOobRead_TooManyBytesReturned)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);

    constexpr uint64_t address = 0x123456789abcdef;
    constexpr uint8_t num_bytes = sizeof(uint64_t);
    constexpr int sd_bus_call_return_value = 1;
    constexpr uint64_t data = 0x13579bdf02468ace;
    constexpr uint8_t num_bytes_returned = sizeof(uint64_t) + 1;

    ExpectRead(mock, address, num_bytes, data, sd_bus_call_return_value,
               num_bytes_returned);
    EXPECT_THROW(h.accelOobRead("test/path", address, num_bytes),
                 IpmiException);
}

void ExpectWrite(StrictMock<sdbusplus::SdBusMock>& mock, uint64_t address,
                 uint8_t num_bytes, uint64_t data, int sd_bus_call_return_value)
{
    ::testing::InSequence s;

    // These must be nullptr or sd_bus_message_unref will seg fault.
    constexpr sd_bus_message* method = nullptr;

    EXPECT_CALL(mock, sd_bus_message_new_method_call(
                          _,         // sd_bus *bus,
                          NotNull(), // sd_bus_message **m
                          StrEq("com.google.custom_accel"),
                          StrEq("/com/google/customAccel/test/path"),
                          StrEq("com.google.custom_accel.BAR"), StrEq("Write")))
        .WillOnce(DoAll(TraceDbus("sd_bus_message_new_method_call"),
                        SetArgPointee<1>(method), Return(0)));

    EXPECT_CALL(
        mock, sd_bus_message_append_basic(
                  method, SD_BUS_TYPE_UINT64,
                  MatcherCast<const void*>(
                      SafeMatcherCast<const uint64_t*>(Pointee(Eq(address))))))
        .WillOnce(DoAll(TraceDbus("sd_bus_message_append_basic(address) -> 1"),
                        Return(1)));

    EXPECT_CALL(mock,
                sd_bus_message_open_container(method, SD_BUS_TYPE_ARRAY,
                                              StrEq(SD_BUS_TYPE_BYTE_STR)))
        .WillOnce(DoAll(TraceDbus("sd_bus_message_open_container(a, y) -> 0"),
                        Return(0)));

    for (auto i = 0; i < num_bytes; ++i)
    {
        const uint8_t byte = (data >> (8 * i)) & 0xff;

        EXPECT_CALL(
            mock, sd_bus_message_append_basic(
                      method, SD_BUS_TYPE_BYTE,
                      MatcherCast<const void*>(
                          SafeMatcherCast<const uint8_t*>(Pointee(Eq(byte))))))
            .WillOnce(
                DoAll(TraceDbus2("sd_bus_message_append_basic"), Return(1)));
    }

    EXPECT_CALL(mock, sd_bus_message_close_container(method))
        .WillOnce(DoAll(TraceDbus("sd_bus_message_close_container() -> 0"),
                        Return(0)));

    EXPECT_CALL(mock, sd_bus_call(_,         // sd_bus *bus,
                                  method,    // sd_bus_message *m
                                  _,         // uint64_t timeout
                                  NotNull(), // sd_bus_error *ret_error
                                  IsNull())) // sd_bus_message **reply
        .WillOnce(DoAll(TraceDbus("sd_bus_call() -> ret_val"),
                        SetArgPointee<3>(SD_BUS_ERROR_NULL),
                        Return(sd_bus_call_return_value)));
}

TEST(HandlerTest, accelOobWrite_Success)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);

    constexpr uint64_t address = 0x123456789abcdef;
    constexpr uint8_t num_bytes = sizeof(uint64_t);
    constexpr int sd_bus_call_return_value = 1;
    constexpr uint64_t data = 0x13579bdf02468ace;

    ExpectWrite(mock, address, num_bytes, data, sd_bus_call_return_value);
    EXPECT_NO_THROW(h.accelOobWrite("test/path", address, num_bytes, data));
}

TEST(HandlerTest, accelOobRead_TooManyBytesRequested)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);

    constexpr uint64_t address = 0x123456789abcdef;
    constexpr uint8_t num_bytes = sizeof(uint64_t) + 1;
    constexpr uint64_t data = 0x13579bdf02468ace;

    EXPECT_THROW(h.accelOobWrite("test/path", address, num_bytes, data),
                 IpmiException);
}

TEST(HandlerTest, accelOobWrite_Fail)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);

    constexpr uint64_t address = 0x123456789abcdef;
    constexpr uint8_t num_bytes = sizeof(uint64_t);
    constexpr int sd_bus_call_return_value = -ENOTCONN;
    constexpr uint64_t data = 0x13579bdf02468ace;

    ExpectWrite(mock, address, num_bytes, data, sd_bus_call_return_value);
    EXPECT_THROW(h.accelOobWrite("test/path", address, num_bytes, data),
                 IpmiException);
}

TEST(HandlerTest, PcieBifurcationStatic)
{
    const std::string& testJson = "/tmp/test-json";
    auto j = R"(
        {
            "/PE1": [ 1, 3 ],
            "/PE3": [ 3, 6 ],
            "/PE4": [ 3, 4, 1 ],
            "/PE6": [ 8 ]
        }
    )"_json;

    std::ofstream bifurcationJson(testJson);
    bifurcationJson << j.dump();
    bifurcationJson.flush();
    bifurcationJson.close();

    BifurcationStatic bifurcationHelper(testJson);
    Handler h(std::ref(bifurcationHelper));

    std::unordered_map<uint8_t, std::vector<uint8_t>> expectedMapping = {
        {1, {1, 3}}, {3, {3, 6}}, {4, {3, 4, 1}}, {6, {8}}};
    std::vector<uint8_t> invalidIndex = {0, 2, 5, 7};

    for (const auto& [index, output] : expectedMapping)
    {
        EXPECT_THAT(h.pcieBifurcationByIndex(nullptr, index),
                    ContainerEq(output));
        EXPECT_THAT(
            h.pcieBifurcationByName(nullptr, std::format("/PE{}", index)),
            ContainerEq(output));
    }

    for (const auto& index : invalidIndex)
    {
        EXPECT_TRUE(h.pcieBifurcationByIndex(nullptr, index).empty());
        EXPECT_TRUE(
            h.pcieBifurcationByName(nullptr, std::format("/PE{}", index))
                .empty());
    }

    std::filesystem::remove(testJson.data());
    bifurcationHelper = BifurcationStatic(testJson);
    Handler h2(std::ref(bifurcationHelper));
    for (uint8_t i = 0; i < 8; ++i)
    {
        auto bifurcation = h2.pcieBifurcationByIndex(nullptr, i);
        EXPECT_TRUE(bifurcation.empty());
    }
}

struct PhysicalTopology
{
    std::string name;
    uint8_t bus;
    std::optional<uint8_t> lanes;
    std::optional<std::vector<std::string>> association;
};

void createPhysicalAssociation(BifurcationDynamicMock& bifurcationHelper,
                               const std::vector<std::string>& baseAssociation,
                               std::span<PhysicalTopology> topology)
{
    EXPECT_CALL(bifurcationHelper, physicalAssociations(_, MAIN_BOARD))
        .WillRepeatedly(Return(baseAssociation));
    for (const auto& device : topology)
    {
        EXPECT_CALL(bifurcationHelper, i2cBus(_, device.name))
            .WillRepeatedly(Return(device.bus));

        if (device.association == std::nullopt)
        {
            EXPECT_CALL(bifurcationHelper, pcieDeviceMaxLanes(_, device.name))
                .WillRepeatedly(Return(device.lanes));
            continue;
        }

        EXPECT_CALL(bifurcationHelper, pcieDeviceMaxLanes(_, device.name))
            .WillRepeatedly(Return(std::nullopt));
        if (device.lanes == std::nullopt)
        {
            EXPECT_CALL(bifurcationHelper, pcieSlotLanes(_, device.name))
                .WillRepeatedly(Return(std::nullopt));
            continue;
        }
        EXPECT_CALL(bifurcationHelper, pcieSlotLanes(_, device.name))
            .WillRepeatedly(Return(device.lanes));
        EXPECT_CALL(bifurcationHelper, physicalAssociations(_, device.name))
            .WillRepeatedly(Return(device.association.value()));
    }
}

TEST(HandlerTest, PcieBifurcationDynamic)
{
    /*
      slot-10 (16 lanes, 4 max channel) PE0
        |-- slot-30
        |-- slot-40
        |-- slot-50
      slot-30 (4 lanes, 2 max channel)
        |-- device-31 (2 lanes)
        |-- device-32 (2 lanes)
      slot-40 (4 lanes, 2 max channel)
        |-- device-41 (4 lanes)
        |-- device-42 (1 lanes)
      slot-50 (7 lanes, 2 max channel)
        |-- slot-51 (3 lanes)
        |-- device-52 (4 lanes)
      slot-51 (7 lanes, 3 max channel)
        |-- device-53 (1 lanes)
        |-- device-54 (1 lanes)
        |-- device-55 (1 lanes)

      slot-20 (16 lanes, 4 max channel) PE1
        |-- device-60 (8 lanes)
        |-- device-70 (8 lanes)
      slot-70 (8 lanes, 2 max channel)
        |-- slot-71
        |-- device-72 (4 lanes)
      slot-71 (4 lanes, 2 max channel)
        |-- device-73 (2 lanes)

      slot-80 (16 lanes, 4 max channel) PE2
        |-- device-81 (16 lanes)

      slot-90 (16 lanes, 4 max channel) PE3
        |-- slot-91 (16 lanes)
      slot-91 (16 lanes, 4 max channel)
        |-- device-92 (8 lanes)
      slot-95 (4 lanes, 4 max channel) PE4
        |-- device-96 (3 lanes)
        |-- device-97 (3 lanes)

      slot-99 (16 lanes, 4 max channel) PE5

      slot-100 (16 lanes, 1 max channel) PE6
      device-110 (16 lanes) PE7
    */

    std::vector<std::string> baseAssociations = {
        "slot-10", "slot-20", "slot-80",  "slot-90",
        "slot-95", "slot-99", "slot-100", "device-110",
    };
    std::vector<PhysicalTopology> topology = {
        // Slots
        {"slot-10", 10, 16,
         std::vector<std::string>{"slot-30", "slot-40", "slot-50"}},
        {"slot-30", 30, 4, std::vector<std::string>{"device-31", "device-32"}},
        {"slot-40", 40, 5, std::vector<std::string>{"device-41", "device-42"}},
        {"slot-50", 50, 7, std::vector<std::string>{"slot-51", "device-52"}},
        {"slot-51", 51, 3,
         std::vector<std::string>{"device-53", "device-54", "device-55"}},
        {"slot-20", 20, 16, std::vector<std::string>{"device-60", "slot-70"}},
        {"slot-70", 70, 8, std::vector<std::string>{"slot-71", "device-72"}},
        {"slot-71", 71, 4, std::vector<std::string>{"device-73"}},
        {"slot-80", 80, 16, std::vector<std::string>{"device-81"}},
        {"slot-90", 90, 16, std::vector<std::string>{"slot-91"}},
        {"slot-91", 91, 16, std::vector<std::string>{"device-92"}},
        {"slot-95", 95, 4, std::vector<std::string>{"device-96", "device-97"}},
        {"slot-99", 99, 16, std::vector<std::string>{}},
        {"slot-100", 100, std::nullopt, std::vector<std::string>{}},

        // Devices
        {"device-31", 31, 2, std::nullopt},
        {"device-32", 32, 2, std::nullopt},
        {"device-41", 41, 4, std::nullopt},
        {"device-42", 42, 1, std::nullopt},
        {"device-52", 52, 4, std::nullopt},
        {"device-53", 53, 1, std::nullopt},
        {"device-54", 54, 1, std::nullopt},
        {"device-55", 55, 1, std::nullopt},
        {"device-60", 60, 8, std::nullopt},
        {"device-72", 72, 4, std::nullopt},
        {"device-73", 73, 2, std::nullopt},
        {"device-81", 81, 16, std::nullopt},
        {"device-92", 92, 8, std::nullopt},
        {"device-96", 96, 3, std::nullopt},
        {"device-97", 97, 3, std::nullopt},
        {"device-110", 110, 16, std::nullopt},
    };
    HandlerMock hMock;

    nlohmann::json entityJson = nlohmann::json::parse(R"(
        {
            "add_in_card": [
                {"instance": 10, "name": "/PE0"},
                {"instance": 20, "name": "/PE1"},
                {"instance": 80, "name": "/PE2"},
                {"instance": 90, "name": "/PE3"},
                {"instance": 95, "name": "/PE4"},
                {"instance": 99, "name": "/PE5"},
                {"instance": 100, "name": "/PE6"},
                {"instance": 110, "name": "/PE7"}
            ]
        }
    )");

    BifurcationDynamicMock bifurcationHelper(entityJson);
    createPhysicalAssociation(bifurcationHelper, baseAssociations, topology);
    Handler h(std::ref(bifurcationHelper));

    // 16
    // 4-5-7
    // 2-2-5-7
    // 2-2-4-1-3-4
    // 2-2-4-1-1-1-1-4
    auto bifurcation = h.pcieBifurcationByIndex(nullptr, 0);
    // EXPECT_THAT(bifurcation, ElementsAre(2, 2, 4, 1, 1, 1, 1, 4));
    // bifurcation = h.pcieBifurcationByName(nullptr, "/PE0");
    // EXPECT_THAT(bifurcation, ElementsAre(2, 2, 4, 1, 1, 1, 1, 4));

    // // 16
    // // 8-8
    // // 8-4-4
    // // 8-2-2-4
    // // Valid PCIe Slot with 3 levels of bifurcation. Last level only has one
    // // device but should result in a (2, 2)
    // bifurcation = h.pcieBifurcationByIndex(nullptr, 1);
    // EXPECT_THAT(bifurcation, ElementsAre(8, 2, 2, 4));
    // bifurcation = h.pcieBifurcationByName(nullptr, "/PE1");
    // EXPECT_THAT(bifurcation, ElementsAre(8, 2, 2, 4));

    // // 16, Valid PCIe slot with one device taking all lanes.
    // bifurcation = h.pcieBifurcationByIndex(nullptr, 2);
    // EXPECT_THAT(bifurcation, ElementsAre(16));
    // bifurcation = h.pcieBifurcationByName(nullptr, "/PE2");
    // EXPECT_THAT(bifurcation, ElementsAre(16));

    // // 16
    // // 16
    // // 8-8
    // bifurcation = h.pcieBifurcationByIndex(nullptr, 3);
    // EXPECT_THAT(bifurcation, ElementsAre(8, 8));
    // bifurcation = h.pcieBifurcationByName(nullptr, "/PE3");
    // EXPECT_THAT(bifurcation, ElementsAre(8, 8));

    // // 16, Valid PCIe slot with multiple device but exceeded max lanes.
    // bifurcation = h.pcieBifurcationByIndex(nullptr, 4);
    // EXPECT_TRUE(bifurcation.empty());
    // bifurcation = h.pcieBifurcationByName(nullptr, "/PE4");
    // EXPECT_TRUE(bifurcation.empty());

    // // Valid PCIe slot with no device
    // bifurcation = h.pcieBifurcationByIndex(nullptr, 5);
    // EXPECT_THAT(bifurcation, ElementsAre(16));
    // bifurcation = h.pcieBifurcationByName(nullptr, "/PE5");
    // EXPECT_THAT(bifurcation, ElementsAre(16));

    // // Invalid PCIe slot
    // bifurcation = h.pcieBifurcationByIndex(nullptr, 6);
    // EXPECT_TRUE(bifurcation.empty());
    // bifurcation = h.pcieBifurcationByName(nullptr, "/PE6");
    // EXPECT_TRUE(bifurcation.empty());

    // // 16, PCIe Device.
    // bifurcation = h.pcieBifurcationByIndex(nullptr, 7);
    // EXPECT_THAT(bifurcation, ElementsAre(16));
    // bifurcation = h.pcieBifurcationByName(nullptr, "/PE7");
    // EXPECT_THAT(bifurcation, ElementsAre(16));

    // // Invalid PCIe slot
    // bifurcation = h.pcieBifurcationByIndex(nullptr, 8);
    // EXPECT_TRUE(bifurcation.empty());
    // bifurcation = h.pcieBifurcationByName(nullptr, "/PE8");
    // EXPECT_TRUE(bifurcation.empty());
}

// TODO: Add checks for other functions of handler.

} // namespace ipmi
} // namespace google
