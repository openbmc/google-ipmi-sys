#include "commands.hpp"
#include "flash_size.hpp"
#include "handler_mock.hpp"

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

TEST(FlashSizeCommandTest, InvalidCommandLength)
{
    // GetFlashSizeRequest is one byte, let's send 0.
    std::vector<std::uint8_t> request = {};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              getFlashSize(request.data(), reply, &dataLen, &hMock));
}

TEST(FlashSizeCommandTest, ValidRequest)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysGetFlashSize};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];
    uint32_t flashSize = 128;

    HandlerMock hMock;
    EXPECT_CALL(hMock, getFlashSize()).WillOnce(Return(flashSize));
    EXPECT_EQ(IPMI_CC_OK,
              getFlashSize(request.data(), reply, &dataLen, &hMock));
    EXPECT_EQ(dataLen, 5);
    EXPECT_EQ(reply[1], 0);
    EXPECT_EQ(reply[2], 0);
    EXPECT_EQ(reply[3], 0);
    EXPECT_EQ(reply[4], flashSize);
}

} // namespace ipmi
} // namespace google
