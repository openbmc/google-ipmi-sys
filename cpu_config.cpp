// Copyright 2024 Google LLC
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

#include "cpu_config.hpp"

#include "commands.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>
#include <ipmid/api.hpp>
#include <stdplus/print.hpp>

#include <cstdint>
#include <vector>

namespace google
{
namespace ipmi
{

Resp getCoreCount(std::span<const uint8_t> data, HandlerInterface* handler)
{
    // data is not used in this function but is kept for consistency with other
    // handlers.
    (void)data;
    int coreCountInt = handler->getCoreCount();
    if (coreCountInt < 0 || coreCountInt > UINT16_MAX)
    {
        // Log error or return an IPMI error code if the count is out of range
        stdplus::print(stderr, "Core count out of range for uint16_t: \n",
                       coreCountInt);
        return ::ipmi::responseUnspecifiedError();
    }
    uint16_t coreCount = static_cast<uint16_t>(coreCountInt);
    std::vector<uint8_t> reply;
    reply.push_back(static_cast<uint8_t>(coreCount & 0xFF));        // LSB
    reply.push_back(static_cast<uint8_t>((coreCount >> 8) & 0xFF)); // MSB
    return ::ipmi::responseSuccess(SysOEMCommands::SysGetCoreCount, reply);
}

} // namespace ipmi
} // namespace google
