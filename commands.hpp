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
    // The Sys "entity id:entity instance" to entity name mapping command.
    SysEntityName = 6,
    // Returns the machine name of the image
    SysMachineName = 7,
    // Arm for psu reset on host shutdown
    SysPsuHardResetOnShutdown = 8,
    // The Sys get flash size command
    SysGetFlashSize = 9,
    // The Sys Host Power Off with disabled fallback watchdog
    SysHostPowerOff = 10,
    // Google CustomAccel service - get the number of devices available
    SysAccelOobDevceCount = 11,
    // Google CustomAccel service - get the name of a single device
    SysAccelOobDevceName = 12,
    // Google CustomAccel service - read from a device
    SysAccelOobRead = 13,
    // Google CustomAccel service - write to a device
    SysAccelOobWrite = 14,
};

} // namespace ipmi
} // namespace google
