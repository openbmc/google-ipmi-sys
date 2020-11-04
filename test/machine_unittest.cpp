#include "commands.hpp"
#include "errors.hpp"
#include "handler_mock.hpp"
#include "machine_name.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;
using ::testing::Throw;

namespace google
{
namespace ipmi
{

TEST(MachineNameCommandTest, InvalidCommandLength)
{
    std::vector<std::uint8_t> request = {};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    ::testing::StrictMock<HandlerMock> hMock;

    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              getMachineName(request.data(), reply, &dataLen, &hMock));
}

TEST(MachineNameCommandTest, InvalidFile)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysMachineName};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    ::testing::StrictMock<HandlerMock> hMock;
    EXPECT_CALL(hMock, getMachineName()).WillOnce(Throw(IpmiException(5)));

    EXPECT_EQ(5, getMachineName(request.data(), reply, &dataLen, &hMock));
}

TEST(MachineNameCommandTest, CachesValidRequest)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysMachineName};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];
    const std::string ret = "Machine";

    ::testing::StrictMock<HandlerMock> hMock;
    EXPECT_CALL(hMock, getMachineName()).WillOnce(Return(ret));

    EXPECT_EQ(IPMI_CC_OK,
              getMachineName(request.data(), reply, &dataLen, &hMock));
    EXPECT_EQ(SysOEMCommands::SysMachineName, reply[0]);
    EXPECT_EQ(ret.size(), reply[1]);
    EXPECT_EQ(0, std::memcmp(&reply[2], ret.data(), ret.size()));

    dataLen = request.size();
    memset(reply, 0, sizeof(reply));
    EXPECT_EQ(IPMI_CC_OK,
              getMachineName(request.data(), reply, &dataLen, &hMock));
    EXPECT_EQ(SysOEMCommands::SysMachineName, reply[0]);
    EXPECT_EQ(ret.size(), reply[1]);
    EXPECT_EQ(0, std::memcmp(&reply[2], ret.data(), ret.size()));
}

} // namespace ipmi
} // namespace google
