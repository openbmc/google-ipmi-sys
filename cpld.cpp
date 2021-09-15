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

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <cstring>
#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct CpldRequest
{
    uint8_t id;
} __attribute__((packed));

//
// Handle reading the cpld version from the tmpfs.
//
Resp cpldVersion(const std::vector<std::uint8_t>& data,
                 const HandlerInterface* handler)
{
    struct CpldRequest request;

    if (data.size() < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }

    // data[0] is the CPLD id. "/run/cpld{id}.version" is what we read.
    // Verified that this cast actually returns the value 255 and not something
    // negative in the case where data[0] is 0xff.  However, it looks weird
    // since I would expect int(uint8(0xff)) to be -1.  So, just cast it
    // unsigned. we're casting to an int width to avoid it thinking it's a
    // letter, because it does that.
    std::memcpy(&request, &data[0], sizeof(request));

    try
    {
        auto values =
            handler->getCpldVersion(static_cast<unsigned int>(request.id));

        // Truncate if the version is too high (documented).
        auto major = std::get<0>(values);
        auto minor = std::get<1>(values);
        auto point = std::get<2>(values);
        auto subpoint = std::get<3>(values);

        return ::ipmi::responseSuccess(
            SysOEMCommands::SysCpldVersion,
            std::vector<std::uint8_t>{major, minor, point, subpoint});
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }
}

} // namespace ipmi
} // namespace google
