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

#include "cpld.hpp"

#include "errors.hpp"
#include "handler.hpp"
#include "main.hpp"

#include <cstring>

namespace google
{
namespace ipmi
{

struct CpldRequest
{
    uint8_t subcommand;
    uint8_t id;
} __attribute__((packed));

struct CpldReply
{
    uint8_t subcommand;
    uint8_t major;
    uint8_t minor;
    uint8_t point;
    uint8_t subpoint;
} __attribute__((packed));

//
// Handle reading the cpld version from the tmpfs.
//
ipmi_ret_t cpldVersion(const uint8_t* reqBuf, uint8_t* replyBuf,
                       size_t* dataLen, const HandlerInterface* handler)
{
    struct CpldRequest request;

    if ((*dataLen) < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    // reqBuf[0] is the subcommand.
    // reqBuf[1] is the CPLD id. "/run/cpld{id}.version" is what we read.
    // Verified that this cast actually returns the value 255 and not something
    // negative in the case where reqBuf[1] is 0xff.  However, it looks weird
    // since I would expect int(uint8(0xff)) to be -1.  So, just cast it
    // unsigned. we're casting to an int width to avoid it thinking it's a
    // letter, because it does that.
    std::memcpy(&request, &reqBuf[0], sizeof(request));

    try
    {
        auto values =
            handler->getCpldVersion(static_cast<unsigned int>(request.id));

        // Truncate if the version is too high (documented).
        struct CpldReply reply;
        reply.subcommand = SysCpldVersion;
        reply.major = std::get<0>(values);
        reply.minor = std::get<1>(values);
        reply.point = std::get<2>(values);
        reply.subpoint = std::get<3>(values);

        std::memcpy(&replyBuf[0], &reply, sizeof(reply));
        (*dataLen) = sizeof(reply);
        return IPMI_CC_OK;
    }
    catch (const IpmiException& e)
    {
        return e.getIpmiError();
    }
}

} // namespace ipmi
} // namespace google
