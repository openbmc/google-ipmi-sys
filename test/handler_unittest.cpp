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

#include "errors.hpp"
#include "handler.hpp"
#include "handler_impl.hpp"

#include <fmt/format.h>
#include <systemd/sd-bus.h>

#include <nlohmann/json.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/test/sdbus_mock.hpp>

#include <charconv>
#include <filesystem>
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
    std::fprintf(stderr, "%s\n", msg);
}

ACTION_P(TraceDbus2, msg)
{
    std::fprintf(stderr, "%s(%02x)\n", msg, *static_cast<const uint8_t*>(arg2));
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
            "1": [ 1, 3 ],
            "3": [ 3, 6 ],
            "4": [ 3, 4, 1 ],
            "6": [ 8 ]
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
    std::vector<uint8_t> invalidBus = {0, 2, 5, 7};

    for (const auto& [bus, output] : expectedMapping)
    {
        EXPECT_THAT(h.pcieBifurcation(bus, false), ContainerEq(output));
    }

    for (const auto& bus : invalidBus)
    {
        EXPECT_TRUE(h.pcieBifurcation(bus, false).empty());
    }

    std::filesystem::remove(testJson.data());
    bifurcationHelper = BifurcationStatic(testJson);
    Handler h2(std::ref(bifurcationHelper));
    for (uint8_t i = 0; i < 8; ++i)
    {
        auto bifurcation = h2.pcieBifurcation(i, false);
        EXPECT_TRUE(bifurcation.empty());
    }
}

struct I2CTree
{
    std::string_view name;
    uint8_t lanes;
    std::optional<std::string_view> mux;
    std::vector<std::string_view> channels;
};

void createI2CTree(std::string_view path, std::vector<I2CTree> tree)
{
    std::filesystem::create_directory(path);

    for (const auto& device : tree)
    {
        std::filesystem::create_directory(
            fmt::format("{}/{}", path, device.name));
        if (device.mux)
        {
            auto muxPath = fmt::format("{}/{}/{}", path, device.name,
                                       *device.mux);
            std::filesystem::create_directory(muxPath);

            for (size_t i = 0; i < device.channels.size(); ++i)
            {
                std::filesystem::create_directory(
                    fmt::format("{}/channel-{}", muxPath, i));
                std::filesystem::create_directory(
                    fmt::format("{}/channel-{}/i2c-dev", muxPath, i));
                std::filesystem::create_directory(
                    fmt::format("{}/channel-{}/i2c-dev/{}", muxPath, i,
                                device.channels[i]));
            }
        }
    }
}

TEST(HandlerTest, PcieBifurcationDynamic)
{
    /*
      i2c-10 (16 lanes) PE0
      `-- 10-0077 (4-channel I2C MUX at 0x77)
          |-- i2c-30 (channel-0) (4 lanes)
          |-- i2c-40 (channel-1) (5 lanes)
          |-- i2c-50 (channel-2) (7 lanes)
          `-- i2c-11 (channel-3) (invalid bus)
      i2c-30 (4 lanes)
      `-- 30-0070 (2-channel I2C MUX at 0x70)
          |-- i2c-31 (channel-0) (2 lanes)
          |-- i2c-32 (channel-1) (2 lanes)
      i2c-40 (5 lanes)
      `-- 40-0070 (2-channel I2C MUX at 0x70)
          |-- i2c-41 (channel-0) (4 lanes)
          |-- i2c-42 (channel-1) (1 lanes)
      i2c-50 (7 lanes)
      `-- 0-0070 (4-channel I2C MUX at 0x70)
          |-- i2c-51 (channel-0) (3 lanes)
          |-- i2c-52 (channel-1) (4 lanes)
      i2c-51 (3 lanes)
      `-- 51-0070 (4-channel I2C MUX at 0x70)
          |-- i2c-53 (channel-0) (1 lanes)
          |-- i2c-54 (channel-1) (1 lanes)
          |-- i2c-55 (channel-2) (1 lanes)

      i2c-20 (16 lanes) PE1
      `-- 20-0077 (4-channel I2C MUX at 0x77)
          |-- i2c-60 (channel-0) (8 lanes)
          |-- i2c-70 (channel-1) (8 lanes)
          |-- i2c-21 (channel-2) (invalid bus)
          `-- i2c-22 (channel-3) (invalid bus)
      i2c-70 (8 lanes)
      `-- 70-0070 (2-channel I2C MUX at 0x70)
          |-- i2c-71 (channel-0) (4 lanes)
          |-- i2c-72 (channel-1) (4 lanes)
      i2c-71 (4 lanes)
      `-- 71-0070 (2-channel I2C MUX at 0x70)
          |-- i2c-73 (channel-0) (2 lanes)
          |-- i2c-74 (channel-1) (2 lanes)

      i2c-80 (16 lanes) PE2
      `-- 80-0077 (4-channel I2C MUX at 0x77)
          |-- i2c-81 (channel-0) (16 lanes)
          |-- i2c-82 (channel-1) (invalid bus)
          |-- i2c-83 (channel-2) (invalid bus)
          `-- i2c-84 (channel-3) (invalid bus)

      i2c-99 (16 lanes) PE3
      `-- 99-0077 (4-channel I2C MUX at 0x77)
          |-- i2c-100 (channel-0) (invalid bus)

      i2c-101 (16 lanes) PE4
      `-- 101-0077 (4-channel I2C MUX at 0x77)
          |-- i2c-102 (channel-0) (invalid bus)
          |-- i2c-103 (channel-1) (invalid bus)
          |-- i2c-104 (channel-2) (invalid bus)
          `-- i2c-105 (channel-3) (invalid bus)
      i2c-110 (16 lanes) PE4
      `-- 110-0077 (0-channel I2C MUX at 0x77)
    */

    std::vector<I2CTree> i2cTree = {
        // Slots
        {"i2c-10", 16, "10-0077", {"i2c-30", "i2c-40", "i2c-50"}},
        {"i2c-30", 4, "30-0070", {"i2c-31", "i2c-32"}},
        {"i2c-40", 5, "40-0070", {"i2c-41", "i2c-42"}},
        {"i2c-50", 7, "50-0070", {"i2c-51", "i2c-52"}},
        {"i2c-51", 3, "51-0070", {"i2c-53", "i2c-54", "i2c-55"}},
        {"i2c-20", 16, "20-0077", {"i2c-60", "i2c-70"}},
        {"i2c-70", 8, "70-0070", {"i2c-71", "i2c-72"}},
        {"i2c-71", 4, "71-0070", {"i2c-73", "i2c-74"}},
        {"i2c-80", 16, "80-0070", {"i2c-81"}},
        {"i2c-99", 16, "90-0070", {"i2c-100"}},
        {"i2c-101",
         16,
         "101-0070",
         {"i2c-102", "i2c-103", "i2c-104", "i2c-105"}},
        {"i2c-110", 16, "110-0070", {}},

        // Devices
        {"i2c-31", 2, std::nullopt, {}},
        {"i2c-32", 2, std::nullopt, {}},
        {"i2c-41", 4, std::nullopt, {}},
        {"i2c-42", 1, std::nullopt, {}},
        {"i2c-52", 4, std::nullopt, {}},
        {"i2c-53", 1, std::nullopt, {}},
        {"i2c-54", 1, std::nullopt, {}},
        {"i2c-55", 1, std::nullopt, {}},
        {"i2c-60", 8, std::nullopt, {}},
        {"i2c-72", 4, std::nullopt, {}},
        {"i2c-73", 2, std::nullopt, {}},
        {"i2c-74", 2, std::nullopt, {}},
        {"i2c-81", 16, std::nullopt, {}},
        {"i2c-110", 16, std::nullopt, {}},
    };

    const std::string& testI2CPath = "./test/i2c/";
    createI2CTree(testI2CPath, i2cTree);

    BifurcationDynamic bifurcationHelper(false, testI2CPath);

    for (const auto& tree : i2cTree)
    {
        uint8_t bus = 0;
        auto result = std::from_chars(tree.name.data() + 4,
                                      tree.name.data() + tree.name.size(), bus);
        if (result.ec == std::errc::invalid_argument)
        {
            continue;
        }

        std::optional<bool> shouldUpdate = bifurcationHelper.needsUpdate(bus);
        if (!shouldUpdate.value_or(true))
        {
            continue;
        }
        // Has value and we should update. Erase existing resource
        else if (shouldUpdate.has_value())
        {
            bifurcationHelper.pcieResources.erase(bus);
        }

        bifurcationHelper.pcieResources.emplace(
            bus,
            PCIe{tree.mux.has_value() ? PCIe::Type::Slot : PCIe::Type::Device,
                 tree.lanes, static_cast<uint8_t>(tree.channels.size())});
    }

    const char* testFilename = "test.json";
    std::string contents = R"(
        {
            "add_in_card": [
                {"instance": 10, "name": "/PE0"},
                {"instance": 20, "name": "/PE1"},
                {"instance": 80, "name": "/PE2"},
                {"instance": 99, "name": "/PE3"},
                {"instance": 100, "name": "/PE4"},
                {"instance": 110, "name": "/PE5"}
            ]
        }
    )";
    std::ofstream outputJson(testFilename);
    outputJson << contents;
    outputJson.flush();
    outputJson.close();

    Handler h(std::ref(bifurcationHelper), testFilename);

    // 16
    // 4-5-7
    // 2-2-5-7
    // 2-2-4-1-3-4
    // 2-2-4-1-1-1-1-4
    std::vector<uint8_t> expectedOutput = {2, 2, 4, 1, 1, 1, 1, 4};
    auto bifurcation = h.pcieBifurcation(0, true);

    ASSERT_EQ(bifurcation.size(), expectedOutput.size());
    for (size_t i = 0; i < bifurcation.size(); ++i)
    {
        EXPECT_EQ(bifurcation[i], expectedOutput[i]);
    }

    // 16
    // 8-8
    // 8-4-4
    // 8-2-2-4
    expectedOutput = {8, 2, 2, 4};
    bifurcation = h.pcieBifurcation(1, true);
    fmt::print(stderr, "Test {} {} {}\n", bifurcation[0], bifurcation[1],
               bifurcation[2]);
    ASSERT_EQ(bifurcation.size(), expectedOutput.size());
    for (size_t i = 0; i < bifurcation.size(); ++i)
    {
        EXPECT_EQ(bifurcation[i], expectedOutput[i]);
    }

    // 16, with only one valid channel behind MUX
    bifurcation = h.pcieBifurcation(2, true);
    ASSERT_EQ(bifurcation.size(), 1);
    EXPECT_EQ(bifurcation[0], 16);

    // Valid PCIe slot with no device, one channel
    bifurcation = h.pcieBifurcation(3, true);
    ASSERT_EQ(bifurcation.size(), 0);

    // Valid PCIe slot with no device, four channels
    bifurcation = h.pcieBifurcation(4, true);
    ASSERT_EQ(bifurcation.size(), 0);

    // 16, with no channel behind MUX
    bifurcation = h.pcieBifurcation(5, true);
    ASSERT_EQ(bifurcation.size(), 1);
    EXPECT_EQ(bifurcation[0], 16);

    // Invalid PCIe slot
    bifurcation = h.pcieBifurcation(6, true);
    ASSERT_EQ(bifurcation.size(), 0);

    std::remove(testFilename);
    std::filesystem::remove_all(testI2CPath);
}

// TODO: Add checks for other functions of handler.

} // namespace ipmi
} // namespace google
