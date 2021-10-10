// Copyright 2021 Google LLC
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
};

} // namespace ipmi
} // namespace google
