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

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct PcieSlotCountReply
{
    uint8_t value;
} __attribute__((packed));

struct PcieSlotI2cBusMappingReply
{
    uint8_t i2c_bus_number;
    uint8_t pcie_slot_name_len;
} __attribute__((packed));

//  Handle the pcie slot count command.
//  Sys can query the number of pcie slots.
Resp pcieSlotCount(const std::vector<std::uint8_t>& data,
                   HandlerInterface* handler);

// Handle the pcie slot to i2c bus mapping command.
// Sys can query which i2c bus is routed to which pcie slot.
Resp pcieSlotI2cBusMapping(const std::vector<std::uint8_t>& data,
                           HandlerInterface* handler);

} // namespace ipmi
} // namespace google
