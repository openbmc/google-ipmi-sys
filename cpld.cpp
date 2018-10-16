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

#include "main.hpp"

#include <cstring>
#include <experimental/filesystem>
#include <fstream>
#include <sstream>

namespace google
{
namespace ipmi
{
namespace fs = std::experimental::filesystem;

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
ipmi_ret_t CpldVersion(const uint8_t* reqBuf, uint8_t* replyBuf,
                       size_t* dataLen)
{
    struct CpldRequest request;

    if ((*dataLen) < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_INVALID;
    }

    // reqBuf[0] is the subcommand.
    // reqBuf[1] is the CPLD id. "/run/cpld{id}.version" is what we read.
    // Verified that this cast actually returns the value 255 and not something
    // negative in the case where reqBuf[1] is 0xff.  However, it looks weird
    // since I would expect int(uint8(0xff)) to be -1.  So, just cast it
    // unsigned. we're casting to an int width to avoid it thinking it's a
    // letter, because it does that.
    std::memcpy(&request, &reqBuf[0], sizeof(request));

    std::ostringstream opath;
    opath << "/run/cpld" << static_cast<unsigned int>(request.id) << ".version";
    // Check for file

    std::error_code ec;
    if (!fs::exists(opath.str(), ec))
    {
        std::fprintf(stderr, "Path: '%s' doesn't exist.\n",
                     opath.str().c_str());
        return IPMI_CC_INVALID;
    }
    // We're uninterested in the state of ec.

    // If file exists, read.
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::failbit);
    std::string value;
    try
    {
        ifs.open(opath.str());
        ifs >> value;
    }
    catch (std::ios_base::failure& fail)
    {
        return IPMI_CC_INVALID;
    }

    // If value parses as expected, return version.
    int major = 0;
    int minor = 0;
    int point = 0;
    int subpoint = 0;

    int num_fields =
        sscanf(value.c_str(), "%d.%d.%d.%d", &major, &minor, &point, &subpoint);
    if (num_fields == 0)
    {
        std::fprintf(stderr, "Invalid version.\n");
        return IPMI_CC_INVALID;
    }

    // Truncate if the version is too high (documented).
    struct CpldReply reply;
    reply.subcommand = SysCpldVersion;
    reply.major = static_cast<uint8_t>(major);
    reply.minor = static_cast<uint8_t>(minor);
    reply.point = static_cast<uint8_t>(point);
    reply.subpoint = static_cast<uint8_t>(subpoint);

    std::memcpy(&replyBuf[0], &reply, sizeof(reply));
    (*dataLen) = sizeof(reply);

    return IPMI_CC_OK;
}

} // namespace ipmi
} // namespace google
