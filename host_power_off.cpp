/*
 * Copyright 2021 Google Inc.
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

#include "host_power_off.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <cstdint>
#include <cstring>
#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

Resp hostPowerOff(const std::vector<uint8_t>& data,
                  const HandlerInterface* handler)
{
    struct HostPowerOffRequest request;

    if (data.size() < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }

    std::memcpy(&request, data.data(), sizeof(struct HostPowerOffRequest));
    try
    {
        handler->hostPowerOffDelay(request.delay);
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    return ::ipmi::responseSuccess(SysOEMCommands::SysHostPowerOff,
                                   std::vector<uint8_t>{});
}
} // namespace ipmi
} // namespace google
