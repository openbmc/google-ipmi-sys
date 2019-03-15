#include "cable.hpp"
#include "handler_mock.hpp"
#include "main.hpp"

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(CableCommandTest, RequestTooSmall)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysCableCheck};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;

    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              CableCheck(request.data(), reply, &dataLen, &hMock));
}

TEST(CableCommandTest, FailsLengthSanityCheck)
{
}

TEST(CableCommandTest, LengthTooLongForPacket)
{
}

TEST(CableCommandTest, ValidRequestValidReturn)
{
}

} // namespace ipmi
} // namespace google
