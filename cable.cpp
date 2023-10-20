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

#include "cable.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>
#include <stdplus/print.hpp>

#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <vector>

namespace google
{
namespace ipmi
{

struct CableRequest
{
    uint8_t ifNameLength;
} __attribute__((packed));

Resp cableCheck(std::span<const uint8_t> data, const HandlerInterface* handler)
{
    // There is an IPMI LAN channel statistics command which could be used for
    // this type of check, however, we're not able to wait for the OpenBMC
    // implementation to stabilize related to the network management.
    //
    // There is a link status file, but it is "unknown" to start with...
    // The path we're checking: /sys/class/net/eth1/statistics/rx_packets

    // This command is expecting: [0x00][len][ifName]
    // data should have [len][ifName]
    if (data.size() < sizeof(struct CableRequest))
    {
        stdplus::print(stderr, "Invalid command length: %u\n",
                       static_cast<uint32_t>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }

    const auto request =
        reinterpret_cast<const struct CableRequest*>(data.data());

    // Sanity check the object contents.
    if (request->ifNameLength == 0)
    {
        stdplus::print(stderr, "Invalid string length: %d\n",
                       request->ifNameLength);
        return ::ipmi::responseReqDataLenInvalid();
    }

    // Verify the request buffer contains the object and the string.
    if (data.size() < (sizeof(struct CableRequest) + request->ifNameLength))
    {
        stdplus::print(stderr, "*dataLen too small: %u\n",
                       static_cast<uint32_t>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }

    // Maximum length one can specify, plus null terminator.
    char nameBuf[256] = {};
    // Copy the string out of the request buffer.
    std::memcpy(&nameBuf[0], request + 1, request->ifNameLength);
    std::string name = nameBuf;
    int64_t count;

    try
    {
        count = handler->getRxPackets(name);
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    // If we have received packets then there is a cable present.
    std::uint8_t value = (count > 0) ? 1 : 0;

    return ::ipmi::responseSuccess(SysOEMCommands::SysCableCheck,
                                   std::vector<std::uint8_t>{value});
}

} // namespace ipmi
} // namespace google
