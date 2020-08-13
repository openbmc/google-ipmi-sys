#include "commands.hpp"
#include "handler_mock.hpp"
#include "pcie_i2c.hpp"

#include <cstdint>
#include <cstring>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(PcieI2cCommandTest, PcieSlotCountTest)
{
    std::vector<std::uint8_t> request = {SysOEMCommands::SysPcieSlotCount};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];
    size_t expectedSize = 3;

    HandlerMock hMock;
    EXPECT_CALL(hMock, buildI2cPcieMapping());
    EXPECT_CALL(hMock, getI2cPcieMappingSize()).WillOnce(Return(expectedSize));
    EXPECT_EQ(IPMI_CC_OK,
              pcieSlotCount(request.data(), reply, &dataLen, &hMock));
    EXPECT_EQ(expectedSize, reply[1]);
}

TEST(PcieI2cCommandTest, PcieSlotEntryRequestTooShort)
{
    std::vector<std::uint8_t> request = {
        SysOEMCommands::SysPcieSlotI2cBusMapping};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_EQ(IPMI_CC_REQ_DATA_LEN_INVALID,
              pcieSlotI2cBusMapping(request.data(), reply, &dataLen, &hMock));
}

TEST(PcieI2cCommandTest, PcieSlotEntryRequestUnsupportedByPlatform)
{
    // If there is no mapping in the device-tree, then the map is of size zero.
    std::vector<std::uint8_t> request = {
        SysOEMCommands::SysPcieSlotI2cBusMapping, 0};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_CALL(hMock, getI2cPcieMappingSize()).WillOnce(Return(0));
    EXPECT_EQ(IPMI_CC_INVALID_RESERVATION_ID,
              pcieSlotI2cBusMapping(request.data(), reply, &dataLen, &hMock));
}

TEST(PcieI2cCommandTest, PcieSlotEntryRequestInvalidIndex)
{
    // index of 1 is invalid if length is 1.
    std::vector<std::uint8_t> request = {
        SysOEMCommands::SysPcieSlotI2cBusMapping, 1};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];

    HandlerMock hMock;
    EXPECT_CALL(hMock, getI2cPcieMappingSize()).WillOnce(Return(1));
    EXPECT_EQ(IPMI_CC_PARM_OUT_OF_RANGE,
              pcieSlotI2cBusMapping(request.data(), reply, &dataLen, &hMock));
}

TEST(PcieI2cCommandTest, PcieSlotEntryRequestValidIndex)
{
    unsigned int index = 0;
    std::vector<std::uint8_t> request = {
        SysOEMCommands::SysPcieSlotI2cBusMapping,
        static_cast<std::uint8_t>(index)};
    size_t dataLen = request.size();
    std::uint8_t reply[MAX_IPMI_BUFFER];
    std::string slotName = "abcd";
    std::uint32_t busNum = 5;

    HandlerMock hMock;
    EXPECT_CALL(hMock, getI2cPcieMappingSize()).WillOnce(Return(1));
    EXPECT_CALL(hMock, getI2cEntry(index))
        .WillOnce(Return(std::make_tuple(busNum, slotName)));
    EXPECT_EQ(IPMI_CC_OK,
              pcieSlotI2cBusMapping(request.data(), reply, &dataLen, &hMock));
    EXPECT_EQ(busNum, reply[1]);
    EXPECT_EQ(slotName.length(), reply[2]);
    EXPECT_EQ(0, std::memcmp(slotName.c_str(), &reply[3], reply[2]));
}

} // namespace ipmi
} // namespace google
