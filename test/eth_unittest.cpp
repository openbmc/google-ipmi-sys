#include "eth.hpp"
#include "main.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

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

    EXPECT_EQ(IPMI_CC_OK, GetEthDevice(request.data(), &reply[0], &dataLen));
    struct EthDeviceReply check;
    std::memcpy(&check, &reply[0], sizeof(check));
    EXPECT_EQ(check.subcommand, SysOEMCommands::SysGetEthDevice);
    EXPECT_EQ(check.channel, 1);
    EXPECT_EQ(check.if_name_len, 4);

    // TODO: Validate the results.
}

} // namespace ipmi
} // namespace google
