#include "commands.hpp"
#include "handler_mock.hpp"
#include "host_s5_power_off.hpp"

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

TEST(S5CommandTest, InvalidRequestLength)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysHostS5PowerOff};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              hostS5PowerOff(request.data(), reply, &dataLen, &hMock));
}

TEST(S5CommandTest, ValidRequest)
{
    // Set the dealy to 15 mins
    std::uint32_t delayValue = 0x384;
    struct HostS5PowerOffRequest requestContents;
    requestContents.subcommand = SysOEMCommands::SysHostS5PowerOff;
    requestContents.delay = delayValue;

    std::vector<std::uint8_t> request(sizeof(requestContents));
    std::memcpy(request.data(), &requestContents, sizeof(requestContents));
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_CALL(hMock, hostS5PowerOffDelay(delayValue));
    EXPECT_EQ(IPMI_CC_OK,
              hostS5PowerOff(request.data(), reply, &dataLen, &hMock));
}

} // namespace ipmi
} // namespace google
