#include "commands.hpp"
#include "errors.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"
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

TEST(MachineNameCommandTest, InvalidFile)
{
    std::vector<std::uint8_t> request = {};
    ::testing::StrictMock<HandlerMock> hMock;
    EXPECT_CALL(hMock, getMachineName()).WillOnce(Throw(IpmiException(5)));

    EXPECT_EQ(::ipmi::response(5), getMachineName(request, &hMock));
}

TEST(MachineNameCommandTest, CachesValidRequest)
{
    std::vector<std::uint8_t> request = {};
    const std::string ret = "Machine";

    ::testing::StrictMock<HandlerMock> hMock;
    EXPECT_CALL(hMock, getMachineName()).WillOnce(Return(ret));

    auto reply = getMachineName(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(GetMachineNameReply) + ret.size(), data.size());
    EXPECT_EQ(SysOEMCommands::SysMachineName, result.first);
    EXPECT_EQ(ret.length(), data[0]);
    EXPECT_EQ(ret.data(),
              std::string(data.begin() + sizeof(struct GetMachineNameReply),
                          data.end()));
}

} // namespace ipmi
} // namespace google
