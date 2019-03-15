#include "cable.hpp"
#include "handler_mock.hpp"
#include "main.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;
using ::testing::StrEq;

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
    // Minimum is three bytes, but a length of zero for the string is invalid.
    std::vector<std::uint8_t> request = {SysOEMCommands::SysCableCheck, 0x00,
                                         'a'};

    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;

    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              CableCheck(request.data(), reply, &dataLen, &hMock));
}

TEST(CableCommandTest, LengthTooLongForPacket)
{
    // The length of a the string, as specified is longer than string provided.
    std::vector<std::uint8_t> request = {SysOEMCommands::SysCableCheck, 0x02,
                                         'a'};

    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;

    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              CableCheck(request.data(), reply, &dataLen, &hMock));
}

TEST(CableCommandTest, ValidRequestValidReturn)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysCableCheck, 0x01,
                                         'a'};

    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;

    EXPECT_CALL(hMock, getRxPackets(StrEq("a"))).WillOnce(Return(0));
    EXPECT_EQ(IPMI_CC_OK, CableCheck(request.data(), reply, &dataLen, &hMock));

    // Check results.
    struct CableReply expectedReply, actualReply;
    EXPECT_EQ(sizeof(expectedReply), dataLen);
    std::memcpy(&actualReply, reply, dataLen);
    EXPECT_EQ(0,
              std::memcmp(&expectedReply, &actualReply, sizeof(expectedReply)));
}

} // namespace ipmi
} // namespace google
