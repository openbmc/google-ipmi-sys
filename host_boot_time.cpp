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

#include "host_boot_time.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <cstdint>
#include <cstring>
#include <ipmid/api-types.hpp>
#include <optional>
#include <regex>
#include <span>
#include <string>
#include <vector>

namespace google
{
namespace ipmi
{

Resp hostBootTimeSetDuration(std::span<const uint8_t> data,
                             const HandlerInterface* handler)
{
    if (data.size() < sizeof(struct HostBootTimeSetDurationRequestHeaderTailer))
    {
        std::fprintf(stderr, "Invalid command length: %u\n", data.size());
        return ::ipmi::responseReqDataLenInvalid();
    }
    const uint8_t length = data[0];
    if (data.size() <
        sizeof(struct HostBootTimeSetDurationRequestHeaderTailer) + length)
    {
        std::fprintf(stderr, "Invalid command length: %u\n", data.size());
        return ::ipmi::responseReqDataLenInvalid();
    }

    const std::string name(data.data() + 1, data.data() + 1 + length);
    // name must >= 1 byte also allow [0-9a-zA-Z_] only.
    if (!std::regex_match(name, std::regex("^[0-9a-zA-Z_]+$")))
    {
        std::fprintf(stderr,
                     "Invalid duration name, only [0-9a-zA-Z_] are allowed");
        return ::ipmi::responseParmOutOfRange();
    }
    uint64_t duration_ms = 0;
    memcpy(&duration_ms, data.data() + 1 + length, sizeof(duration_ms));

    std::vector<uint8_t> reply(sizeof(struct HostBootTimeSetDurationReply));
    try
    {
        reply[0] = handler->hostBootTimeSetDuration(name, duration_ms);
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    return ::ipmi::responseSuccess(SysOEMCommands::SysHostBootTimeSetDuration,
                                   reply);
}

Resp hostBootTimeNotify(std::span<const uint8_t> data,
                        const HandlerInterface* handler)
{
    constexpr const int kCheckPointCodeEnd = 5;

    if (data.size() < sizeof(struct HostBootTimeNotifyRequest))
    {
        std::fprintf(stderr, "Invalid command length: %u\n", data.size());
        return ::ipmi::responseReqDataLenInvalid();
    }

    const uint8_t checkPointCode = data[0];
    if (checkPointCode >= kCheckPointCodeEnd)
    {
        std::fprintf(stderr, "Undefined Host boot time checkpoint code: %u\n",
                     checkPointCode);
        return ::ipmi::responseParmOutOfRange();
    }

    std::optional<uint64_t> timestamp_ms;
    try
    {
        timestamp_ms = handler->hostBootTimeNotify(checkPointCode);
        if (timestamp_ms == std::nullopt)
        {
            std::fprintf(
                stderr,
                "Notify failed, check point(%u) shouldn't be sent now.\n",
                checkPointCode);
            return ::ipmi::responseParmOutOfRange();
        }
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    std::vector<uint8_t> reply(sizeof(struct HostBootTimeNotifyReply));
    memcpy(reply.data(), &(*timestamp_ms),
           sizeof(struct HostBootTimeNotifyReply));
    return ::ipmi::responseSuccess(SysOEMCommands::SysHostBootTimeNotify,
                                   reply);
}

} // namespace ipmi
} // namespace google
