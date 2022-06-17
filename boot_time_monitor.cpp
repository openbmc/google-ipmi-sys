/*
 * Copyright 2022 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "boot_time_monitor.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>

#include <cstdint>
#include <cstring>
#include <optional>
#include <regex>
#include <span>
#include <string>
#include <vector>

namespace google
{
namespace ipmi
{
Resp sendRebootCheckpoint(std::span<const uint8_t> data,
                          const HandlerInterface* handler)
{
    constexpr auto kNameOffset = sizeof(struct CheckpointReqHeader);
    if (data.size() < kNameOffset ||
        data.size() < kNameOffset + data[offsetof(CheckpointReqHeader, length)])
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<unsigned int>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }
    const auto kNameLen = data[offsetof(CheckpointReqHeader, length)];

    const std::string name{data.data() + kNameOffset,
                           data.data() + kNameOffset + kNameLen};
    std::fprintf(stderr, "name = %s", name.c_str());
    if (std::regex_search(name, std::regex("[^0-9a-zA-Z_]")))
    {
        std::fprintf(stderr,
                     "Invalid checkpoint name, only [0-9a-zA-Z_] are allowed");
        return ::ipmi::responseParmOutOfRange();
    }

    int64_t wallTime = 0, duration = 0;
    memcpy(&wallTime, data.data() + offsetof(CheckpointReqHeader, wallTime),
           sizeof(wallTime));
    memcpy(&duration, data.data() + offsetof(CheckpointReqHeader, duration),
           sizeof(duration));

    try
    {
        handler->sendRebootCheckpoint(name, wallTime, duration);
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    return ::ipmi::responseSuccess(SysOEMCommands::SysSendRebootCheckpoint,
                                   std::vector<uint8_t>{});
}

Resp sendRebootComplete([[maybe_unused]] std::span<const uint8_t> data,
                        const HandlerInterface* handler)
{
    try
    {
        handler->sendRebootComplete();
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    return ::ipmi::responseSuccess(SysOEMCommands::SysSendRebootComplete,
                                   std::vector<uint8_t>{});
}

Resp sendRebootAdditionalDuration(std::span<const uint8_t> data,
                                  const HandlerInterface* handler)
{
    constexpr auto kNameOffset = sizeof(struct DurationReqHeader);
    if (data.size() < kNameOffset ||
        data.size() < kNameOffset + data[offsetof(DurationReqHeader, length)])
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<unsigned int>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }
    const auto kNameLen = data[offsetof(DurationReqHeader, length)];

    const std::string name{data.data() + kNameOffset,
                           data.data() + kNameOffset + kNameLen};
    if (std::regex_search(name, std::regex("[^0-9a-zA-Z_]")))
    {
        std::fprintf(stderr,
                     "Invalid duration name, only [0-9a-zA-Z_] are allowed");
        return ::ipmi::responseParmOutOfRange();
    }

    int64_t duration = 0;
    memcpy(&duration, data.data() + offsetof(DurationReqHeader, duration),
           sizeof(duration));

    try
    {
        handler->sendRebootAdditionalDuration(name, duration);
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    return ::ipmi::responseSuccess(
        SysOEMCommands::SysSendRebootAdditionalDuration,
        std::vector<uint8_t>{});
}

} // namespace ipmi
} // namespace google
