#include "eth.hpp"
#include "handler_mock.hpp"
#include "main.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(EthCommandTest, ValidRequestReturnsSuccess)
{
    // This command requests no input, therefore it will just return what it
    // knows.
    std::vector<std::uint8_t> request = {SysOEMCommands::SysGetEthDevice};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];
    const std::uint8_t expectedAnswer[4] = {'e', 't', 'h', '0'};
    const std::uint8_t expectedChannel = 1;

    HandlerMock hMock;
    EXPECT_CALL(hMock, getEthDetails())
        .WillOnce(Return(std::make_tuple(
            expectedChannel,
            std::string(expectedAnswer,
                        expectedAnswer + sizeof(expectedAnswer)))));

    EXPECT_EQ(IPMI_CC_OK,
              getEthDevice(request.data(), &reply[0], &dataLen, &hMock));
    struct EthDeviceReply check;
    std::memcpy(&check, &reply[0], sizeof(check));
    EXPECT_EQ(check.subcommand, SysOEMCommands::SysGetEthDevice);
    EXPECT_EQ(check.channel, expectedChannel);
    EXPECT_EQ(check.ifNameLength, sizeof(expectedAnswer));
    EXPECT_EQ(0, std::memcmp(expectedAnswer, &reply[sizeof(check)],
                             sizeof(expectedAnswer)));
}

} // namespace ipmi
} // namespace google
