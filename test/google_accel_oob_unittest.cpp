/*
//  Handle the Accel OOB device count command
ipmi_ret_t accelOobDeviceCount(const uint8_t* reqBuf, uint8_t* replyBuf,
                               size_t* dataLen, HandlerInterface* handler);

ipmi_ret_t accelOobDeviceName(const uint8_t* reqBuf, uint8_t* replyBuf,
                              size_t* dataLen, HandlerInterface* handler);

ipmi_ret_t accelOobRead(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, HandlerInterface* handler);

ipmi_ret_t accelOobWrite(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen, HandlerInterface* handler);
*/

#include "commands.hpp"
#include "errors.hpp"
#include "google_accel_oob.hpp"
#include "handler_mock.hpp"

#include <gtest/gtest.h>
#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

using ::testing::Return;

TEST(GoogleAccelOobTest, DeviceCount_Success)
{
    ::testing::StrictMock<HandlerMock> h;

    uint8_t reqBuf[1] = {SysAccelOobDevceCount};

    struct {
        uint8_t subcommand;
        uint32_t count;
    } __attribute__((packed)) replyBuf;

    constexpr uint32_t kTestDeviceCount = 2;
    size_t dataLen = sizeof(reqBuf);

    EXPECT_CALL(h, accelOobDeviceCount()).WillOnce(Return(kTestDeviceCount));

    ipmi_ret_t r = accelOobDeviceCount(reqBuf,
                                       reinterpret_cast<uint8_t*>(&replyBuf),
                                       &dataLen, &h);
    EXPECT_EQ(r, IPMI_CC_OK);
    EXPECT_EQ(dataLen, sizeof(replyBuf));
    EXPECT_EQ(replyBuf.subcommand, SysAccelOobDevceCount);
    EXPECT_EQ(replyBuf.count, kTestDeviceCount);
}

} // namespace ipmi
} // namespace google
