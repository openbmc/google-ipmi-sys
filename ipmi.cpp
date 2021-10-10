// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "config.h"

#include "ipmi.hpp"

#include "bmc_mode.hpp"
#include "cable.hpp"
#include "commands.hpp"
#include "cpld.hpp"
#include "entity_name.hpp"
#include "eth.hpp"
#include "flash_size.hpp"
#include "google_accel_oob.hpp"
#include "handler.hpp"
#include "host_power_off.hpp"
#include "linux_boot_done.hpp"
#include "machine_name.hpp"
#include "pcie_bifurcation.hpp"
#include "pcie_i2c.hpp"
#include "psu.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <ipmid/message.hpp>
#include <stdplus/print.hpp>

#include <cstdint>
#include <cstdio>
#include <optional>
#include <span>

namespace google
{
namespace ipmi
{

Resp handleSysCommand(HandlerInterface* handler, ::ipmi::Context::ptr ctx,
                      uint8_t cmd, std::span<const uint8_t> data)
{
    switch (cmd)
    {
        case SysGetBmcMode:
            return getBmcMode(data, handler);
        case SysCableCheck:
            return cableCheck(data, handler);
        case SysCpldVersion:
            return cpldVersion(data, handler);
        case SysGetEthDevice:
            return getEthDevice(data, handler);
        case SysPsuHardReset:
            return psuHardReset(data, handler);
        case SysPcieSlotCount:
            return pcieSlotCount(data, handler);
        case SysPcieSlotI2cBusMapping:
            return pcieSlotI2cBusMapping(data, handler);
        case SysEntityName:
            return getEntityName(data, handler);
        case SysMachineName:
            return getMachineName(data, handler);
        case SysPsuHardResetOnShutdown:
            return psuHardResetOnShutdown(data, handler);
        case SysGetFlashSize:
            return getFlashSize(data, handler);
        case SysHostPowerOff:
            return hostPowerOff(data, handler);
        case SysAccelOobDeviceCount:
            return accelOobDeviceCount(data, handler);
        case SysAccelOobDeviceName:
            return accelOobDeviceName(data, handler);
        case SysAccelOobRead:
            return accelOobRead(data, handler);
        case SysAccelOobWrite:
            return accelOobWrite(data, handler);
        case SysPCIeSlotBifurcation:
            return pcieBifurcation(ctx, data, handler);
        case SysLinuxBootDone:
            return linuxBootDone(data, handler);
        case SysGetAccelVrSettings:
            return accelGetVrSettings(ctx, data, handler);
        case SysSetAccelVrSettings:
            return accelSetVrSettings(ctx, data, handler);
        default:
            stdplus::print(stderr, "Invalid subcommand: {:#x}\n", cmd);
            return ::ipmi::responseInvalidCommand();
    }
}

} // namespace ipmi
} // namespace google
