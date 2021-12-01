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

#include "google_version.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <ipmid/api-types.hpp>
#include <ipmid/message.hpp>
#include <ipmid/utils.hpp>
#include <nlohmann/json.hpp>
#include <regex>
#include <sdbusplus/message/types.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <xyz/openbmc_project/State/BMC/server.hpp>

#ifndef GBMC_VERSION_PREFIX
#define GBMC_VERSION_PREFIX "gbmc-release-"
#endif

constexpr auto bmc_state_interface = "xyz.openbmc_project.State.BMC";
constexpr auto bmc_state_property = "CurrentBMCState";

typedef struct
{
    uint16_t major;
    uint16_t minor;
    uint16_t d[2];
} Revision;

static int gbmcConverVersion(VersionReader& version_reader, Revision& rev)
{
    int major = 0;
    int minor = 0;
    int point = 0;
    int subpoint = 0;
    if (!version_reader.ReadVersion(&major, &minor, &point, &subpoint))
    {
        return -1;
    }
    if (major > 0x7F)
    {
        printf("major version %d unrepresentable in IPMI device ID\n", major);
    }
    if (minor > 99)
    {
        printf("minor version %d unrepresentable in IPMI device ID\n", minor);
    }
    if (point > 0xFFFF)
    {
        printf("point version %d unrepresentable in IPMI device ID\n", point);
    }
    if (subpoint > 0xFFFF)
    {
        printf("subpoint version %d unrepresentable in IPMI device ID\n",
               subpoint);
    }
    rev.major = major;
    rev.minor = minor;
    rev.d[0] = htole16(point);
    rev.d[1] = htole16(subpoint);
    return 0;
}

static bool getCurrentBmcState()
{
    sdbusplus::bus::bus bus = sdbusplus::bus::new_default();

    // Get the Inventory object implementing the BMC interface
    ipmi::DbusObjectInfo bmcObject =
        ::ipmi::getDbusObject(bus, bmc_state_interface);
    const auto& value =
        ::ipmi::getDbusProperty(bus, bmcObject.second, bmcObject.first,
                                bmc_state_interface, bmc_state_property);

    return std::holds_alternative<std::string>(value) &&
           sdbusplus::xyz::openbmc_project::State::server::BMC::
                   convertBMCStateFromString(std::get<std::string>(value)) ==
               sdbusplus::xyz::openbmc_project::State::server::BMC::BMCState::
                   Ready;
}

static bool getCurrentBmcStateWithFallback(const bool fallbackAvailability)
{
    try
    {
        return getCurrentBmcState();
    }
    catch (...)
    {
        // Nothing provided the BMC interface, therefore return whatever was
        // configured as the default.
        return fallbackAvailability;
    }
}

::ipmi::RspType<uint8_t,  // Device ID
                uint8_t,  // Device Revision
                uint8_t,  // Firmware Revision Major
                uint8_t,  // Firmware Revision minor
                uint8_t,  // IPMI version
                uint8_t,  // Additional device support
                uint24_t, // MFG ID
                uint16_t, // Product ID
                uint32_t  // AUX info
                >
    ipmiAppGetGBMCDeviceId(::ipmi::Context::ptr)
{
    int r = -1;
    Revision rev = {0, 0, 0, 0};
    static struct
    {
        uint8_t id;
        uint8_t revision;
        uint8_t fw[2];
        uint8_t ipmiVer;
        uint8_t addnDevSupport;
        uint24_t manufId;
        uint16_t prodId;
        uint32_t aux;
    } devId{};
    static bool devIdInitialized = false;
    static bool defaultActivationSetting = true;
    const char* filename = "/usr/share/ipmi-providers/dev_id.json";
    constexpr auto ipmiDevIdStateShift = 7;
    constexpr auto ipmiDevIdFw1Mask = ~(1 << ipmiDevIdStateShift);

    if (!devIdInitialized)
    {

        // For Google: read the revision directly from /etc/os-release.
        OsReleaseReader osReleaseReader;
        GoogleVersionReader versionReader(&osReleaseReader);
        printf("Parsing release info as gBMC\n");
        r = gbmcConverVersion(versionReader, rev);

        // "IANA Enterprise Number for Google, Inc."
        devId.manufId = 0x002B79;

        devId.revision = 0x80;

        if (r >= 0)
        {
            if (rev.major > 0x7F)
            {
                rev.major = 0x7F;
            }
            devId.fw[0] = 0x7F & rev.major;
            if (rev.minor > 99)
            {
                rev.minor = 99;
            }
            devId.fw[1] = rev.minor % 10 + (rev.minor / 10) * 16;
            std::memcpy(&devId.aux, rev.d, 4);
        }

        devId.ipmiVer = 2;

        std::ifstream devIdFile(filename);
        if (devIdFile.is_open())
        {
            auto data = nlohmann::json::parse(devIdFile, nullptr, false);
            if (!data.is_discarded())
            {
                devId.revision |= data.value("revision", 0);
                devId.prodId = data.value("prod_id", 0);

                // Don't read the file every time if successful
                devIdInitialized = true;
            }
            else
            {
                std::fprintf(stderr, "Device ID JSON parser failure\n");
                return ::ipmi::responseUnspecifiedError();
            }
        }
        else
        {
            std::fprintf(stderr, "Device ID file not found\n");
            return ::ipmi::responseUnspecifiedError();
        }

        devId.addnDevSupport = 0x8D;
    }

    // Set availability to the actual current BMC state
    devId.fw[0] &= ipmiDevIdFw1Mask;
    if (!getCurrentBmcStateWithFallback(defaultActivationSetting))
    {
        devId.fw[0] |= (1 << ipmiDevIdStateShift);
    }

    return ::ipmi::responseSuccess(
        devId.id, devId.revision, devId.fw[0], devId.fw[1], devId.ipmiVer,
        devId.addnDevSupport, devId.manufId, devId.prodId, devId.aux);
}

GoogleVersionReader::GoogleVersionReader(ReleaseReader* release_reader) :
    os_release_items_(release_reader->ReadReleaseFile())
{
}

// Parameters renamed here in definition for clarity, whereas it's unambiguous
// in the declaration.
bool GoogleVersionReader::ReadVersion(int* major_out, int* minor_out,
                                      int* point_out, int* subpoint_out)
{
    int major = 0;
    int minor = 0;
    int point = 0;
    int subpoint = 0;

    const std::string version = ReadVersion();
    int num_fields = sscanf(version.c_str(), GBMC_VERSION_PREFIX "%d.%d.%d.%d",
                            &major, &minor, &point, &subpoint);
    // Don't write the output unless parse was successful.
    switch (num_fields)
    {
        case 4:
            *subpoint_out = subpoint;
            [[fallthrough]];
        case 3:
            *point_out = point;
            [[fallthrough]];
        case 2:
            *minor_out = minor;
            *major_out = major;
            break;
        default:
            return false;
    }
    return true;
}

std::string GoogleVersionReader::ReadVersion()
{
    return GetOsReleaseValue("VERSION_ID");
}

std::string GoogleVersionReader::ReadDistro()
{
    return GetOsReleaseValue("ID");
}

// Returns a default-constructed string for map keys that are not found, without
// inserting a value like operator[] does.
std::string GoogleVersionReader::GetOsReleaseValue(const std::string& key) const
{
    auto iter = os_release_items_.find(key);
    if (iter == os_release_items_.end())
    {
        return std::string();
    }
    return iter->second;
}

const char* const OsReleaseReader::kOsReleaseDefaultPath = "/etc/os-release";

OsReleaseReader::OsReleaseReader(const std::string& os_release_path) :
    os_release_path(os_release_path)
{
}

std::unordered_map<std::string, std::string> OsReleaseReader::ReadReleaseFile()
{
    std::unordered_map<std::string, std::string> items;
    std::ifstream os_release_file(os_release_path);
    if (!os_release_file)
    {
        std::fprintf(stderr, "Failed to open %s for release version\n",
                     os_release_path.c_str());
        return items;
    }
    // Matches KEY="VALUE" where the quotes are optional.
    std::regex key_value_re(
        "(\\w)" // Alphanumeric/underscore chars (group 0)
        "="
        "\"?"   // Optional quote
        "(.*?)" // Any characters, excluding the optional quotes (group 1)
        "\"?"); // Optional quote
    while (os_release_file)
    {
        std::string line;
        std::getline(os_release_file,
                     line); // If this fails, line will be empty.
        std::smatch key_value_match;
        if (std::regex_match(line, key_value_match, key_value_re))
        {
            // First submatch is KEY. Second submatch is VALUE.
            items[key_value_match[1]] = key_value_match[2];
        }
    }
    return items;
}