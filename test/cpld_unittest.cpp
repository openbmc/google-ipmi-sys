#include "cpld.hpp"
#include "handler_mock.hpp"
#include "main.hpp"

#include <cstdint>
#include <vector>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(CpldCommandTest, RequestTooSmall)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysCableCheck};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              CpldVersion(request.data(), reply, &dataLen, &hMock));
}

} // namespace ipmi
} // namespace google
