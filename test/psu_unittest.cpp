#include "commands.hpp"
#include "handler_mock.hpp"
#include "psu.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(PsuCommandTest, InvalidRequestLength)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysPsuHardReset};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              psuHardReset(request.data(), reply, &dataLen, &hMock));
}

TEST(PsuCommandTest, ValidRequest)
{
    std::uint32_t delayValue = 0x45;
    struct PsuResetRequest requestContents;
    requestContents.subcommand = SysOEMCommands::SysPsuHardReset;
    requestContents.delay = delayValue;

    std::vector<std::uint8_t> request(sizeof(requestContents));
    std::memcpy(request.data(), &requestContents, sizeof(requestContents));
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_CALL(hMock, psuResetDelay(delayValue));
    EXPECT_EQ(IPMI_CC_OK,
              psuHardReset(request.data(), reply, &dataLen, &hMock));
}

TEST(PsuCommandTest, InvalidRequestLength)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysPsuHardResetOnShutdown};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              psuHardReset(request.data(), reply, &dataLen, &hMock));
}

TEST(PsuCommandTest, ValidRequest)
{
    struct PsuResetOnShutdownRequest requestContents;
    requestContents.subcommand = SysOEMCommands::SysPsuHardReset;
    requestContents.delay = delayValue;

    std::vector<std::uint8_t> request(sizeof(requestContents));
    std::memcpy(request.data(), &requestContents, sizeof(requestContents));
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_CALL(hMock, psuResetOnShutdown());
    EXPECT_EQ(IPMI_CC_OK,
              psuHardResetOnShutdown(request.data(), reply, &dataLen, &hMock));
}


} // namespace ipmi
} // namespace google
