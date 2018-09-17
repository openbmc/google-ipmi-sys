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

#include "main.hpp"

#include <cstdint>
#include <string>

namespace google
{
namespace ipmi
{

struct EthDeviceRequest
{
    uint8_t subcommand;
} __attribute__((packed));

// The reply to the ethdevice command specifies the
// IPMI channel number and the if_name used for the
// ncis connection.
struct EthDeviceReply
{
    uint8_t subcommand;
    uint8_t channel;
    // if_name_len doesn't include the null-terminator.
    uint8_t if_name_len;
    uint8_t if_name[0];
} __attribute__((packed));

// The phosphor-host-ipmi daemon requires a configuration that maps
// the if_name to the IPMI LAN channel.  However, that doesn't strictly
// define which is meant to be used for NCSI.
#ifndef NCSI_IPMI_CHANNEL
#define NCSI_IPMI_CHANNEL 1
#endif

#ifndef NCSI_IF_NAME
#define NCSI_IF_NAME eth0
#endif

// TOOD(venture): The ipmid.h has this macro, which is a header we
// can't normally access.
#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

// To deal with receiving a string without quotes.
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define NCSI_IF_NAME_STR STR(NCSI_IF_NAME)

ipmi_ret_t GetEthDevice(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen)
{
    if ((*dataLen) < sizeof(struct EthDeviceRequest))
    {
        fprintf(stderr, "Invalid command length: %lu\n", (*dataLen));
        return IPMI_CC_INVALID;
    }

    std::string device = NCSI_IF_NAME_STR;
    if (device.length() == 0)
    {
        fprintf(stderr, "Invalid eth string\n");
        return IPMI_CC_INVALID;
    }

    if ((sizeof(struct EthDeviceReply) + device.length()) > MAX_IPMI_BUFFER)
    {
        fprintf(stderr, "Response would overflow response buffer\n");
        return IPMI_CC_INVALID;
    }

    // Fill in the response buffer.
    auto reply = reinterpret_cast<struct EthDeviceReply*>(&replyBuf[0]);
    reply->subcommand = SysGetEthDevice;
    reply->channel = NCSI_IPMI_CHANNEL;
    reply->if_name_len = device.length();
    memcpy(reply->if_name, device.c_str(), device.length());

    (*dataLen) = sizeof(struct EthDeviceReply) + device.length();

    return IPMI_CC_OK;
}

} // namespace ipmi
} // namespace google
