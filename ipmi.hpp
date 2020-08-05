#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

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
};

// Handle the google-ipmi-sys IPMI OEM commands.
ipmi_ret_t handleSysCommand(HandlerInterface* handler, ipmi_cmd_t cmd,
                            const uint8_t* reqBuf, uint8_t* replyCmdBuf,
                            size_t* dataLen);

} // namespace ipmi
} // namespace google
