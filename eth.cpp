/*
 * Copyright 2018 Google Inc.
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

#include "eth.hpp"

#include "commands.hpp"
#include "handler.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <tuple>

namespace google
{
namespace ipmi
{

struct EthDeviceRequest
{
    uint8_t subcommand;
} __attribute__((packed));

// TOOD(venture): The ipmid.h has this macro, which is a header we
// can't normally access.
#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

ipmi_ret_t getEthDevice(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, const HandlerInterface* handler)
{
    if ((*dataLen) < sizeof(struct EthDeviceRequest))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    std::tuple<std::uint8_t, std::string> details = handler->getEthDetails();

    std::string device = std::get<1>(details);
    if (device.length() == 0)
    {
        std::fprintf(stderr, "Invalid eth string\n");
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    if ((sizeof(struct EthDeviceReply) + device.length()) > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr, "Response would overflow response buffer\n");
        return IPMI_CC_REQUESTED_TOO_MANY_BYTES;
    }

    // Fill in the response buffer.
    auto reply = reinterpret_cast<struct EthDeviceReply*>(&replyBuf[0]);
    reply->subcommand = SysGetEthDevice;
    reply->channel = std::get<0>(details);
    reply->ifNameLength = device.length();
    std::memcpy(reply->ifName, device.c_str(), device.length());

    (*dataLen) = sizeof(struct EthDeviceReply) + device.length();

    return IPMI_CC_OK;
}

} // namespace ipmi
} // namespace google
