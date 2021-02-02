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
#include "handler_impl.hpp"
#include "util.hpp"

#include <ipmid/api.h>

#include <cinttypes>
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
#include <string_view>
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

VersionTuple Handler::getCpldVersion(unsigned int id) const
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
    VersionTuple version = std::make_tuple(0, 0, 0, 0);

    int num_fields =
        std::sscanf(value.c_str(), "%" SCNu8 ".%" SCNu8 ".%" SCNu8 ".%" SCNu8,
                    &std::get<0>(version), &std::get<1>(version),
                    &std::get<2>(version), &std::get<3>(version));
    if (num_fields == 0)
    {
        std::fprintf(stderr, "Invalid version.\n");
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }

    return version;
}

static constexpr auto TIME_DELAY_FILENAME = "/run/psu_timedelay";
static constexpr auto SYSTEMD_SERVICE = "org.freedesktop.systemd1";
static constexpr auto SYSTEMD_ROOT = "/org/freedesktop/systemd1";
static constexpr auto SYSTEMD_INTERFACE = "org.freedesktop.systemd1.Manager";
static constexpr auto PSU_HARDRESET_TARGET = "gbmc-psu-hardreset.target";

void Handler::psuResetDelay(std::uint32_t delay) const
{
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

static constexpr auto RESET_ON_SHUTDOWN_FILENAME = "/run/powercycle_on_s5";

void Handler::psuResetOnShutdown() const
{
    std::ofstream ofs;
    ofs.open(RESET_ON_SHUTDOWN_FILENAME, std::ofstream::out);
    if (!ofs.good())
    {
        std::fprintf(stderr, "Unable to open file for output.\n");
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }
    ofs.close();
}

std::string Handler::getEntityName(std::uint8_t id, std::uint8_t instance)
{
    // Check if we support this Entity ID.
    auto it = _entityIdToName.find(id);
    if (it == _entityIdToName.end())
    {
        log<level::ERR>("Unknown Entity ID", entry("ENTITY_ID=%d", id));
        throw IpmiException(IPMI_CC_INVALID_FIELD_REQUEST);
    }

    std::string entityName;
    try
    {
        // Parse the JSON config file.
        if (!_entityConfigParsed)
        {
            _entityConfig = parseConfig(_configFile);
            _entityConfigParsed = true;
        }

        // Find the "entity id:entity instance" mapping to entity name.
        entityName = readNameFromConfig(it->second, instance, _entityConfig);
        if (entityName.empty())
        {
            throw IpmiException(IPMI_CC_INVALID_FIELD_REQUEST);
        }
    }
    catch (InternalFailure& e)
    {
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }

    return entityName;
}

std::string Handler::getMachineName()
{
    const char* path = "/etc/os-release";
    std::ifstream ifs(path);
    if (ifs.fail())
    {
        std::fprintf(stderr, "Failed to open: %s\n", path);
        throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
    }

    std::string line;
    while (true)
    {
        std::getline(ifs, line);
        if (ifs.eof())
        {
            std::fprintf(stderr, "Failed to find OPENBMC_TARGET_MACHINE: %s\n",
                         path);
            throw IpmiException(IPMI_CC_INVALID);
        }
        if (ifs.fail())
        {
            std::fprintf(stderr, "Failed to read: %s\n", path);
            throw IpmiException(IPMI_CC_UNSPECIFIED_ERROR);
        }
        std::string_view lineView(line);
        constexpr std::string_view prefix = "OPENBMC_TARGET_MACHINE=";
        if (lineView.substr(0, prefix.size()) != prefix)
        {
            continue;
        }
        lineView.remove_prefix(prefix.size());
        lineView.remove_prefix(
            std::min(lineView.find_first_not_of('"'), lineView.size()));
        lineView.remove_suffix(
            lineView.size() - 1 -
            std::min(lineView.find_last_not_of('"'), lineView.size() - 1));
        return std::string(lineView);
    }
}

std::string readNameFromConfig(const std::string& type, uint8_t instance,
                               const Json& config)
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

void Handler::buildI2cPcieMapping()
{
    _pcie_i2c_map = buildPcieMap();
}

size_t Handler::getI2cPcieMappingSize() const
{
    return _pcie_i2c_map.size();
}

std::tuple<std::uint32_t, std::string>
    Handler::getI2cEntry(unsigned int entry) const
{
    return _pcie_i2c_map[entry];
}

} // namespace ipmi
} // namespace google
