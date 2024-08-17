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
    SysAccelOobDeviceCount = 11,
    // Google CustomAccel service - get the name of a single device
    SysAccelOobDeviceName = 12,
    // Google CustomAccel service - read from a device
    SysAccelOobRead = 13,
    // Google CustomAccel service - write to a device
    SysAccelOobWrite = 14,
    // The Sys PCIe Slot Bifurcation information command.
    SysPCIeSlotBifurcation = 15,
    // The Sys get BMC Mode command
    SysGetBmcMode = 16,
    // The Sys Linux Boot Done command
    SysLinuxBootDone = 17,
    // Send reboot checkpoint to BMC to monitor the reboot process.
    SysSendRebootCheckpoint = 18,
    // Send reboot end event to BMC to notify BMC the reboot is completed.
    SysSendRebootComplete = 19,
    // Send Additional duration to BMC to monitor the reboot process.
    SysSendRebootAdditionalDuration = 20,
    // Google CustomAccel Get VR Settings
    SysGetAccelVrSettings = 21,
    // Google CustomAccel Set VR Settings
    SysSetAccelVrSettings = 22,
    // Get BM instance property info
    SysGetBMInstanceProperty = 23,
    // Read OEM BIOS Setting
    SysReadBiosSetting = 24,
};

} // namespace ipmi
} // namespace google
