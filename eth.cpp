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

#include "eth.hpp"

#include "commands.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>
#include <stdplus/print.hpp>

#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <tuple>
#include <vector>

namespace google
{
namespace ipmi
{

// TOOD(venture): The ipmid.h has this macro, which is a header we
// can't normally access.
#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

Resp getEthDevice(std::span<const uint8_t> data,
                  const HandlerInterface* handler)
{
    std::tuple<std::uint8_t, std::string> details =
        handler->getEthDetails(std::string(data.begin(), data.end()));

    std::string device = std::get<1>(details);
    if (device.length() == 0)
    {
        stdplus::print(stderr, "Invalid eth string\n");
        return ::ipmi::responseReqDataLenInvalid();
    }

    if ((sizeof(struct EthDeviceReply) + device.length()) > MAX_IPMI_BUFFER)
    {
        stdplus::print(stderr, "Response would overflow response buffer\n");
        return ::ipmi::responseRetBytesUnavailable();
    }

    std::vector<std::uint8_t> reply;
    reply.reserve(device.length() + sizeof(struct EthDeviceReply));
    reply.emplace_back(std::get<0>(details));                /* channel */
    reply.emplace_back(device.length());                     /* ifNameLength */
    reply.insert(reply.end(), device.begin(), device.end()); /* name */

    return ::ipmi::responseSuccess(SysOEMCommands::SysGetEthDevice, reply);
}

} // namespace ipmi
} // namespace google
