#include "commands.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"
#include "host_power_off.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

TEST(PowerOffCommandTest, InvalidRequestLength)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysHostPowerOff};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              hostPowerOff(request, &hMock));
}

TEST(PowerOffCommandTest, ValidRequest)
{
    // Set the dealy to 15 mins
    std::uint32_t delayValue = 0x384;
    struct HostPowerOffRequest requestContents;
    requestContents.delay = delayValue;

    std::vector<std::uint8_t> request(sizeof(requestContents));
    std::memcpy(request.data(), &requestContents, sizeof(requestContents));

    HandlerMock hMock;
    EXPECT_CALL(hMock, hostPowerOffDelay(delayValue));

    auto reply = hostPowerOff(request, &hMock);
    auto result = ValidateReply(reply, false);
    EXPECT_EQ(SysOEMCommands::SysHostPowerOff, result.first);
}

} // namespace ipmi
} // namespace google
