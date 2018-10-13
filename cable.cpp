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

#include "cable.hpp"

#include "main.hpp"

#include <cstdint>
#include <cstring>
#include <experimental/filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

namespace google
{
namespace ipmi
{
namespace fs = std::experimental::filesystem;

struct CableRequest
{
    uint8_t subcommand;
    uint8_t if_name_len;
    uint8_t if_name[0];
} __attribute__((packed));

struct CableReply
{
    uint8_t subcommand;
    uint8_t value;
} __attribute__((packed));

ipmi_ret_t CableCheck(const uint8_t* reqBuf, uint8_t* replyBuf, size_t* dataLen)
{
    // There is an IPMI LAN channel statistics command which could be used for
    // this type of check, however, we're not able to wait for the OpenBMC
    // implementation to stabilize related to the network management.
    //
    // There is a link status file, but it is "unknown" to start with...
    // The path we're checking: /sys/class/net/eth1/statistics/rx_packets

    // This command is expecting: [0x00][len][if_name]
    if ((*dataLen) < sizeof(struct CableRequest) + sizeof(uint8_t))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_INVALID;
    }

    const auto request =
        reinterpret_cast<const struct CableRequest*>(&reqBuf[0]);

    // Sanity check the object contents.
    if (request->if_name_len == 0)
    {
        std::fprintf(stderr, "Invalid string length: %d\n",
                     request->if_name_len);
        return IPMI_CC_INVALID;
    }

    // Verify the request buffer contains the object and the string.
    if ((*dataLen) < (sizeof(struct CableRequest) + request->if_name_len))
    {
        std::fprintf(stderr, "*dataLen too small: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_INVALID;
    }

    // Maximum length one can specify, plus null terminator.
    char nameBuf[256] = {};
    std::ostringstream opath;

    // Copy the string out of the request buffer.
    std::memcpy(&nameBuf[0], request->if_name, request->if_name_len);
    std::string name = nameBuf;

    // Minor sanity & security check (of course, I'm less certain if unicode
    // comes into play here.
    //
    // Basically you can't easily inject ../ or /../ into the path below.
    if (name.find("/") != std::string::npos)
    {
        std::fprintf(stderr, "Invalid or illegal name: '%s'\n", nameBuf);
        return IPMI_CC_INVALID;
    }

    opath << "/sys/class/net/" << name << "/statistics/rx_packets";
    std::string path = opath.str();

    std::error_code ec;
    if (!fs::exists(path, ec))
    {
        std::fprintf(stderr, "Path: '%s' doesn't exist.\n", path.c_str());
        return IPMI_CC_INVALID;
    }
    // We're uninterested in the state of ec.

    // Read the file and check the result.
    int64_t count = 0;
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::failbit);
    try
    {
        ifs.open(path);
        ifs >> count;
    }
    catch (std::ios_base::failure& fail)
    {
        return IPMI_CC_INVALID;
    }

    struct CableReply reply;
    reply.subcommand = SysCableCheck;

    // If we have received packets then there is a cable present.
    reply.value = (count > 0) ? 1 : 0;

    // Return the subcommand and the result.
    std::memcpy(&replyBuf[0], &reply, sizeof(struct CableReply));
    (*dataLen) = sizeof(struct CableReply);

    return IPMI_CC_OK;
}

} // namespace ipmi
} // namespace google
