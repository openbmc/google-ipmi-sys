#include "errors.hpp"
#include "handler.hpp"
#include "handler_impl.hpp"

#include <systemd/sd-bus.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/test/sdbus_mock.hpp>
#include <string>
#include <tuple>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

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
using ::testing::DoAll;
using ::testing::ElementsAre;
using ::testing::Eq;
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
    {
    }

  protected:
    sdbusplus::bus::bus accelOobGetDbus() const override
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
::testing::Sequence
    ExpectGetManagedObjects(StrictMock<sdbusplus::SdBusMock>& mock)
{
    ::testing::Sequence s;

    // These must be nullptr or sd_bus_message_unref will seg fault.
    constexpr sd_bus_message* method = nullptr;
    constexpr sd_bus_message* msg = nullptr;

    EXPECT_CALL(mock, sd_bus_message_new_method_call(
                          _,         // sd_bus *bus,
                          NotNull(), // sd_bus_message **m
                          StrEq("com.google.custom_accel"), StrEq("/"),
                          StrEq("org.freedesktop.DBus.ObjectManager"),
                          StrEq("GetManagedObjects")))
        .InSequence(s)
        .WillOnce(DoAll(SetArgPointee<1>(method), Return(0)));

    EXPECT_CALL(mock, sd_bus_call(_,          // sd_bus *bus,
                                  method,     // sd_bus_message *m
                                  _,          // uint64_t timeout
                                  NotNull(),  // sd_bus_error *ret_error
                                  NotNull())) // sd_bus_message **reply
        .InSequence(s)
        .WillOnce(DoAll(SetArgPointee<3>(SD_BUS_ERROR_NULL),
                        SetArgPointee<4>(msg), // reply
                        Return(0)));

    EXPECT_CALL(mock, sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY,
                                                     StrEq("{oa{sa{sv}}}")))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0))
        .InSequence(s)
        .WillOnce(Return(0));

    EXPECT_CALL(mock, sd_bus_message_enter_container(
                          msg, SD_BUS_TYPE_DICT_ENTRY, StrEq("oa{sa{sv}}")))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_read_basic(msg, SD_BUS_TYPE_OBJECT_PATH,
                                                NotNull()))
        .InSequence(s)
        .WillOnce(DoAll(AssignReadVal<const char*>(object_path), Return(1)));

    EXPECT_CALL(mock, sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY,
                                                     StrEq("{sa{sv}}")))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0))
        .InSequence(s)
        .WillOnce(Return(0));

    EXPECT_CALL(mock, sd_bus_message_enter_container(
                          msg, SD_BUS_TYPE_DICT_ENTRY, StrEq("sa{sv}")))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock,
                sd_bus_message_read_basic(msg, SD_BUS_TYPE_STRING, NotNull()))
        .InSequence(s)
        .WillOnce(DoAll(AssignReadVal<const char*>(property_grpc), Return(1)));

    EXPECT_CALL(mock, sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY,
                                                     StrEq("{sv}")))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0))
        .InSequence(s)
        .WillOnce(Return(0));

    EXPECT_CALL(mock, sd_bus_message_enter_container(
                          msg, SD_BUS_TYPE_DICT_ENTRY, StrEq("sv")))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock,
                sd_bus_message_read_basic(msg, SD_BUS_TYPE_STRING, NotNull()))
        .InSequence(s)
        .WillOnce(DoAll(AssignReadVal<const char*>(value_port), Return(1)));

    EXPECT_CALL(
        mock, sd_bus_message_verify_type(msg, SD_BUS_TYPE_VARIANT, StrNe("u")))
        .Times(AnyNumber())
        .InSequence(s)
        .WillRepeatedly(Return(0));

    EXPECT_CALL(
        mock, sd_bus_message_verify_type(msg, SD_BUS_TYPE_VARIANT, StrEq("u")))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_enter_container(msg, SD_BUS_TYPE_VARIANT,
                                                     StrEq("u")))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock,
                sd_bus_message_read_basic(msg, SD_BUS_TYPE_UINT32, NotNull()))
        .InSequence(s)
        .WillOnce(DoAll(AssignReadVal<uint32_t>(port), Return(0)));

    EXPECT_CALL(
        mock, sd_bus_message_verify_type(msg, SD_BUS_TYPE_VARIANT, StrNe("u")))
        .Times(AnyNumber())
        .InSequence(s)
        .WillRepeatedly(Return(0));

    EXPECT_CALL(mock, sd_bus_message_exit_container(msg))
        .InSequence(s)
        .WillOnce(Return(1))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_exit_container(msg))
        .InSequence(s)
        .WillOnce(Return(1))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_exit_container(msg))
        .InSequence(s)
        .WillOnce(Return(1))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_exit_container(msg))
        .InSequence(s)
        .WillOnce(Return(1));

    return s;
}

void ExpectSdBusError(StrictMock<sdbusplus::SdBusMock>& mock)
{
    EXPECT_CALL(mock, sd_bus_message_new_method_call(
                          _,         // sd_bus *bus,
                          NotNull(), // sd_bus_message **m
                          StrEq("com.google.custom_accel"), StrEq("/"),
                          StrEq("org.freedesktop.DBus.ObjectManager"),
                          StrEq("GetManagedObjects")))
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
    ExpectSdBusError(mock);
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
    ExpectSdBusError(mock);
    EXPECT_THROW(h.accelOobDeviceName(0), IpmiException);
}

TEST(HandlerTest, accelOobDeviceName_OutOfRange)
{
    StrictMock<sdbusplus::SdBusMock> mock;
    MockDbusHandler h(mock);
    ExpectGetManagedObjects(mock);
    EXPECT_THROW(h.accelOobDeviceName(1), IpmiException);
}

constexpr uint8_t NUM_BYTES_RETURNED_EQUALS_NUM_BYTES = 0xff;
::testing::Sequence
    ExpectRead(StrictMock<sdbusplus::SdBusMock>& mock, uint64_t address,
               uint8_t num_bytes, uint64_t data, int sd_bus_call_return_value,
               uint8_t num_bytes_returned = NUM_BYTES_RETURNED_EQUALS_NUM_BYTES)
{
    ::testing::Sequence s;

    // These must be nullptr or sd_bus_message_unref will seg fault.
    constexpr sd_bus_message* method = nullptr;
    constexpr sd_bus_message* msg = nullptr;

    EXPECT_CALL(mock, sd_bus_message_new_method_call(
                          _,         // sd_bus *bus,
                          NotNull(), // sd_bus_message **m
                          StrEq("com.google.custom_accel"),
                          StrEq("/com/google/customAccel/test/path"),
                          StrEq("com.google.custom_accel.BAR"), StrEq("Read")))
        .InSequence(s)
        .WillOnce(DoAll(SetArgPointee<1>(method), Return(0)));

    EXPECT_CALL(
        mock, sd_bus_message_append_basic(
                  method, SD_BUS_TYPE_UINT64,
                  MatcherCast<const void*>(
                      SafeMatcherCast<const uint64_t*>(Pointee(Eq(address))))))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock,
                sd_bus_message_append_basic(
                    method, SD_BUS_TYPE_UINT64,
                    MatcherCast<const void*>(SafeMatcherCast<const uint64_t*>(
                        Pointee(Eq<uint64_t>(num_bytes))))))
        .InSequence(s)
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_call(_,          // sd_bus *bus,
                                  method,     // sd_bus_message *m
                                  _,          // uint64_t timeout
                                  NotNull(),  // sd_bus_error *ret_error
                                  NotNull())) // sd_bus_message **reply
        .InSequence(s)
        .WillOnce(DoAll(SetArgPointee<3>(SD_BUS_ERROR_NULL),
                        SetArgPointee<4>(msg), // reply
                        Return(sd_bus_call_return_value)));

    if (sd_bus_call_return_value >= 0)
    {
        EXPECT_CALL(mock,
                    sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY,
                                                   StrEq(SD_BUS_TYPE_BYTE_STR)))
            .InSequence(s)
            .WillOnce(Return(1));

        if (num_bytes_returned == NUM_BYTES_RETURNED_EQUALS_NUM_BYTES)
        {
            num_bytes_returned = num_bytes;
        }
        for (auto i = num_bytes_returned - 1; i >= 0; --i)
        {
            EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0))
                .InSequence(s)
                .WillOnce(Return(0));

            const uint8_t byte = (data >> (8 * i)) & 0xff;
            EXPECT_CALL(mock, sd_bus_message_read_basic(msg, SD_BUS_TYPE_BYTE,
                                                        NotNull()))
                .InSequence(s)
                .WillOnce(DoAll(AssignReadVal<uint8_t>(byte), Return(1)));
        }

        EXPECT_CALL(mock, sd_bus_message_at_end(msg, 0))
            .InSequence(s)
            .WillOnce(Return(1));

        EXPECT_CALL(mock, sd_bus_message_exit_container(msg))
            .InSequence(s)
            .WillOnce(Return(1));
    }

    return s;
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

::testing::Sequence ExpectWrite(StrictMock<sdbusplus::SdBusMock>& mock,
                                uint64_t address, uint8_t num_bytes,
                                uint64_t data, int sd_bus_call_return_value)
{
    ::testing::Sequence s;

    // These must be nullptr or sd_bus_message_unref will seg fault.
    constexpr sd_bus_message* method = nullptr;
    constexpr sd_bus_message* msg = nullptr;

    EXPECT_CALL(mock, sd_bus_message_new_method_call(
                          _,         // sd_bus *bus,
                          NotNull(), // sd_bus_message **m
                          StrEq("com.google.custom_accel"),
                          StrEq("/com/google/customAccel/test/path"),
                          StrEq("com.google.custom_accel.BAR"), StrEq("Write")))
        .InSequence(s)
        .WillOnce(DoAll(TraceDbus("sd_bus_message_new_method_call"),
                        SetArgPointee<1>(method), Return(0)));

    EXPECT_CALL(
        mock, sd_bus_message_append_basic(
                  method, SD_BUS_TYPE_UINT64,
                  MatcherCast<const void*>(
                      SafeMatcherCast<const uint64_t*>(Pointee(Eq(address))))))
        .InSequence(s)
        .WillOnce(DoAll(TraceDbus("sd_bus_message_append_basic(address) -> 1"),
                        Return(1)));

    EXPECT_CALL(mock,
                sd_bus_message_open_container(method, SD_BUS_TYPE_ARRAY,
                                              StrEq(SD_BUS_TYPE_BYTE_STR)))
        .InSequence(s)
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
            .InSequence(s)
            .WillOnce(
                DoAll(TraceDbus2("sd_bus_message_append_basic"), Return(1)));
    }

    EXPECT_CALL(mock, sd_bus_message_close_container(method))
        .InSequence(s)
        .WillOnce(DoAll(TraceDbus("sd_bus_message_close_container() -> 0"),
                        Return(0)));

    EXPECT_CALL(mock, sd_bus_call(_,          // sd_bus *bus,
                                  method,     // sd_bus_message *m
                                  _,          // uint64_t timeout
                                  NotNull(),  // sd_bus_error *ret_error
                                  NotNull())) // sd_bus_message **reply
        .InSequence(s)
        .WillOnce(DoAll(TraceDbus("sd_bus_call() -> ret_val"),
                        SetArgPointee<3>(SD_BUS_ERROR_NULL),
                        SetArgPointee<4>(msg), // reply
                        Return(sd_bus_call_return_value)));

    return s;
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

// TODO: Add checks for other functions of handler.

} // namespace ipmi
} // namespace google
