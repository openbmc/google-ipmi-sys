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
#include <span>

namespace google
{
namespace ipmi
{

struct HostBootTimeSetDurationRequestHeaderTailer
{
    uint8_t length;       // header
    uint64_t duration_ms; // tailer
} __attribute__((packed));

struct HostBootTimeSetDurationReply
{
    uint8_t setDurationCommandResult;
} __attribute__((packed));

struct HostBootTimeNotifyRequest
{
    uint8_t checkpointCode;
} __attribute__((packed));

struct HostBootTimeNotifyReply
{
    uint64_t timetamp_ms;
} __attribute__((packed));

Resp hostBootTimeSetDuration(std::span<const uint8_t> data,
                             const HandlerInterface* handler);
Resp hostBootTimeNotify(std::span<const uint8_t> data,
                        const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
