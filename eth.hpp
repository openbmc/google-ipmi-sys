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

// The reply to the ethdevice command specifies the
// IPMI channel number and the ifName used for the
// ncis connection.
struct EthDeviceReply
{
    uint8_t subcommand;
    uint8_t channel;
    // ifNameLength doesn't include the null-terminator.
    uint8_t ifNameLength;
} __attribute__((packed));

// Handle the eth query command.
// Sys can query the ifName and IPMI channel of the BMC's NCSI ethernet
// device.
ipmi_ret_t getEthDevice(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
