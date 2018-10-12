#pragma once

namespace google
{
namespace ipmi
{

enum SysOEMCommands
{
    // The Sys cable check command.
    SysCableCheck = 0,
    // The Sys cpld version over ipmi command.
    SysCpldVersion = 1,
    // The Sys get eth device command.
    SysGetEthDevice = 2,
    // The Sys psu hard reset command.
    SysPsuHardReset = 3,
    // The Sys pcie slot count command.
    SysPcieSlotCount = 4,
    // The Sys pcie slot to i2c bus mapping command.
    SysPcieSlotI2cBusMapping = 5,
};

} // namespace ipmi
} // namespace google
