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

namespace google
{
namespace ipmi
{

//  Handle the pcie slot count command.
//  Sys can query the number of pcie slots.
ipmi_ret_t pcieSlotCount(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen, HandlerInterface* handler);

// Handle the pcie slot to i2c bus mapping command.
// Sys can query which i2c bus is routed to which pcie slot.
ipmi_ret_t pcieSlotI2cBusMapping(const uint8_t* reqBuf, uint8_t* replyBuf,
                                 size_t* dataLen, HandlerInterface* handler);

} // namespace ipmi
} // namespace google
