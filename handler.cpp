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
#include <map>
#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>
#include <sstream>
#include <string>
#include <tuple>
#include <xyz/openbmc_project/Common/error.hpp>

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
using Json = nlohmann::json;
using namespace phosphor::logging;
using InternalFailure =
    sdbusplus::xyz::openbmc_project::Common::Error::InternalFailure;

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

std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>
    Handler::getCpldVersion(unsigned int id) const
{
    std::ostringstream opath;
    opath << "/run/cpld" << id << ".version";
    // Check for file

    std::error_code ec;
    if (!fs::exists(opath.str(), ec))
    {
        std::fprintf(stderr, "Path: '%s' doesn't exist.\n",
                     opath.str().c_str());
        throw IpmiException(IPMI_CC_INVALID_FIELD_REQUEST);
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
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }

    // If value parses as expected, return version.
    int major = 0;
    int minor = 0;
    int point = 0;
    int subpoint = 0;

    int num_fields = std::sscanf(value.c_str(), "%d.%d.%d.%d", &major, &minor,
                                 &point, &subpoint);
    if (num_fields == 0)
    {
        std::fprintf(stderr, "Invalid version.\n");
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }

    return std::make_tuple(
        static_cast<uint8_t>(major), static_cast<uint8_t>(minor),
        static_cast<uint8_t>(point), static_cast<uint8_t>(subpoint));
}

static constexpr auto TIME_DELAY_FILENAME = "/run/psu_timedelay";
static constexpr auto SYSTEMD_SERVICE = "org.freedesktop.systemd1";
static constexpr auto SYSTEMD_ROOT = "/org/freedesktop/systemd1";
static constexpr auto SYSTEMD_INTERFACE = "org.freedesktop.systemd1.Manager";
static constexpr auto PSU_HARDRESET_TARGET = "gbmc-psu-hardreset.target";

void Handler::psuResetDelay(std::uint32_t delay) const
{
    using namespace phosphor::logging;

    std::ofstream ofs;
    ofs.open(TIME_DELAY_FILENAME, std::ofstream::out);
    if (!ofs.good())
    {
        std::fprintf(stderr, "Unable to open file for output.\n");
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }

    ofs << "PSU_HARDRESET_DELAY=" << delay << std::endl;
    if (ofs.fail())
    {
        std::fprintf(stderr, "Write failed\n");
        ofs.close();
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }

    // Write succeeded, please continue.
    ofs.flush();
    ofs.close();

    auto bus = sdbusplus::bus::new_default();
    auto method = bus.new_method_call(SYSTEMD_SERVICE, SYSTEMD_ROOT,
                                      SYSTEMD_INTERFACE, "StartUnit");

    method.append(PSU_HARDRESET_TARGET);
    method.append("replace");

    try
    {
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        log<level::ERR>("Failed to call PSU hard reset");
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }
}

static Json config{};
static bool parsed = false;
static const std::map<uint8_t, std::string> entityIdToName{
    {0x03, "cpu"},
    {0x04, "storage_device"},
    {0x06, "system_management_module"},
    {0x08, "memory_module"},
    {0x0B, "add_in_card"},
    {0x17, "system_chassis"},
    {0x20, "memory_device"}};
static constexpr auto configFile =
    "/usr/share/ipmi-entity-association/entity_association_map.json";

Json parse_config()
{
    std::ifstream jsonFile(configFile);
    if (!jsonFile.is_open())
    {
        log<level::ERR>("Entity association JSON file not found");
        elog<InternalFailure>();
    }

    auto data = Json::parse(jsonFile, nullptr, false);
    if (data.is_discarded())
    {
        log<level::ERR>("Entity association JSON parser failure");
        elog<InternalFailure>();
    }

    return data;
}

std::string read(const std::string& type, uint8_t instance, const Json& config)
{
    static const std::vector<Json> empty{};
    std::vector<Json> readings = config.value(type, empty);
    std::string name = "";
    for (const auto& j : readings)
    {
        uint8_t instanceNum = j.value("instance", 0);
        // Not the instance we're interested in
        if (instanceNum != instance)
        {
            continue;
        }

        // Found the instance we're interested in
        name = j.value("name", "");

        break;
    }
    return name;
}

std::string Handler::getEntityName(std::uint8_t id, std::uint8_t instance)
{
    // Check if we support this Entity ID.
    auto it = entityIdToName.find(id);
    if (it == entityIdToName.end())
    {
        log<level::ERR>("Unknown Entity ID", entry("ENTITY_ID=%d", id));
        throw IpmiException(IPMI_CC_INVALID_FIELD_REQUEST);
    }

    std::string entityName;
    try
    {
        // Parse the JSON config file.
        if (!parsed)
        {
            config = parse_config();
            parsed = true;
        }

        // Find the "entity id:entity instance" mapping to entity name.
        entityName = read(it->second, instance, config);
        if (entityName.empty())
            throw IpmiException(IPMI_CC_INVALID_FIELD_REQUEST);
    }
    catch (InternalFailure& e)
    {
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }

    return entityName;
}

Handler handlerImpl;

} // namespace ipmi
} // namespace google
