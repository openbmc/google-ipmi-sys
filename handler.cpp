/*
 * Copyright 2019 Google Inc.
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

#include "handler.hpp"

#include "errors.hpp"

#include <ipmid/api.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>

// The phosphor-host-ipmi daemon requires a configuration that maps
// the if_name to the IPMI LAN channel.  However, that doesn't strictly
// define which is meant to be used for NCSI.
#ifndef NCSI_IPMI_CHANNEL
#define NCSI_IPMI_CHANNEL 1
#endif

#ifndef NCSI_IF_NAME
#define NCSI_IF_NAME eth0
#endif

// To deal with receiving a string without quotes.
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define NCSI_IF_NAME_STR STR(NCSI_IF_NAME)

namespace google
{
namespace ipmi
{
namespace fs = std::filesystem;

std::tuple<std::uint8_t, std::string> Handler::getEthDetails() const
{
    return std::make_tuple(NCSI_IPMI_CHANNEL, NCSI_IF_NAME_STR);
}

std::int64_t Handler::getRxPackets(const std::string& name) const
{
    std::ostringstream opath;
    opath << "/sys/class/net/" << name << "/statistics/rx_packets";
    std::string path = opath.str();

    // Minor sanity & security check (of course, I'm less certain if unicode
    // comes into play here.
    //
    // Basically you can't easily inject ../ or /../ into the path below.
    if (name.find("/") != std::string::npos)
    {
        std::fprintf(stderr, "Invalid or illegal name: '%s'\n", name.c_str());
        throw IpmiException(IPMI_CC_INVALID_FIELD_REQUEST);
    }

    std::error_code ec;
    if (!fs::exists(path, ec))
    {
        std::fprintf(stderr, "Path: '%s' doesn't exist.\n", path.c_str());
        throw IpmiException(IPMI_CC_INVALID_FIELD_REQUEST);
    }
    // We're uninterested in the state of ec.

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
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }

    return count;
}

Handler handlerImpl;

} // namespace ipmi
} // namespace google
