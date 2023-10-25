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

#include "psu.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>
#include <stdplus/print.hpp>

#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace google
{
namespace ipmi
{

Resp psuHardReset(std::span<const uint8_t> data,
                  const HandlerInterface* handler)
{
    struct PsuResetRequest request;

    if (data.size() < sizeof(request))
    {
        stdplus::print(stderr, "Invalid command length: {}\n", data.size());
        return ::ipmi::responseReqDataLenInvalid();
    }

    std::memcpy(&request, data.data(), sizeof(struct PsuResetRequest));
    try
    {
        handler->psuResetDelay(request.delay);
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    return ::ipmi::responseSuccess(SysOEMCommands::SysPsuHardReset,
                                   std::vector<uint8_t>{});
}

Resp psuHardResetOnShutdown(std::span<const uint8_t>,
                            const HandlerInterface* handler)
{
    try
    {
        handler->psuResetOnShutdown();
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    return ::ipmi::responseSuccess(SysOEMCommands::SysPsuHardResetOnShutdown,
                                   std::vector<uint8_t>{});
}

} // namespace ipmi
} // namespace google
