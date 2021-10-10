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

struct PsuResetRequest
{
    uint8_t subcommand;
    // Delay in seconds.
    uint32_t delay;
} __attribute__((packed));

struct PsuResetOnShutdownRequest
{
    uint8_t subcommand;
} __attribute__((packed));

// Set a time-delayed PSU hard reset.
ipmi_ret_t psuHardReset(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, const HandlerInterface* handler);

// Arm for PSU hard reset on host shutdown.
ipmi_ret_t psuHardResetOnShutdown(const uint8_t* reqBuf, uint8_t* replyBuf,
                                  size_t* dataLen,
                                  const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
