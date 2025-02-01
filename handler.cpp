// Copyright 2022 Google LLC
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

#include "handler.hpp"

#include "bm_config.h"

#include "bm_instance.hpp"
#include "bmc_mode_enum.hpp"
#include "errors.hpp"
#include "handler_impl.hpp"
#include "util.hpp"

#include <fcntl.h>
#include <ipmid/api.h>
#include <mtd/mtd-abi.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>
#include <stdplus/print.hpp>
#include <xyz/openbmc_project/Common/error.hpp>

#include <cinttypes>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>

#ifndef NCSI_IF_NAME
#define NCSI_IF_NAME eth0
#endif

// To deal with receiving a string without quotes.
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define NCSI_IF_NAME_STR STR(NCSI_IF_NAME)

namespace ipmi
{
std::uint8_t getChannelByName(const std::string& chName);
}

namespace google
{
namespace ipmi
{
using Json = nlohmann::json;
using namespace phosphor::logging;
using InternalFailure =
    sdbusplus::xyz::openbmc_project::Common::Error::InternalFailure;
using Value = std::variant<double>;

uint8_t isBmcInBareMetalMode(const std::unique_ptr<FileSystemInterface>& fs)
{
#if BARE_METAL
    return static_cast<uint8_t>(BmcMode::BM_MODE);
#else
    std::error_code ec;

    if (fs->exists(bmDriveCleaningDoneAckFlagPath, ec))
    {
        stdplus::print(
            stderr,
            "{} exists so we acked cleaning done and must be in BM mode\n",
            bmDriveCleaningDoneAckFlagPath);
        return static_cast<uint8_t>(BmcMode::BM_MODE);
    }

    if (fs->exists(bmDriveCleaningDoneFlagPath, ec))
    {
        fs->rename(bmDriveCleaningDoneFlagPath, bmDriveCleaningDoneAckFlagPath,
                   ec);
        stdplus::print(
            stderr,
            "{} exists so we just finished cleaning and must be in BM mode\n",
            bmDriveCleaningDoneFlagPath);
        return static_cast<uint8_t>(BmcMode::BM_MODE);
    }

    if (fs->exists(BM_SIGNAL_PATH, ec))
    {
        if (!fs->exists(bmDriveCleaningFlagPath, ec))
        {
            fs->create(bmDriveCleaningFlagPath);
        }

        stdplus::print(
            stderr,
            "{} exists and no done/ack flag, we must be in BM cleaning mode\n",
            BM_SIGNAL_PATH);
        return static_cast<uint8_t>(BmcMode::BM_CLEANING_MODE);
    }

    stdplus::print(
        stderr,
        "Unable to find any BM state files so we must not be in BM mode\n");
    return static_cast<uint8_t>(BmcMode::NON_BM_MODE);
#endif
}

uint8_t Handler::getBmcMode()
{
    // BM_CLEANING_MODE is not implemented yet
    return isBmcInBareMetalMode(this->getFs());
}

std::tuple<std::uint8_t, std::string> Handler::getEthDetails(
    std::string intf) const
{
    if (intf.empty())
    {
        intf = NCSI_IF_NAME_STR;
    }
    return std::make_tuple(::ipmi::getChannelByName(intf), std::move(intf));
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
        stdplus::print(stderr, "Invalid or illegal name: '{}'\n", name);
        throw IpmiException(::ipmi::ccInvalidFieldRequest);
    }

    std::error_code ec;
    if (!this->getFs()->exists(path, ec))
    {
        stdplus::print(stderr, "Path: '{}' doesn't exist.\n", path);
        throw IpmiException(::ipmi::ccInvalidFieldRequest);
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
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    return count;
}

VersionTuple Handler::getCpldVersion(unsigned int id) const
{
    std::ostringstream opath;
    opath << "/run/cpld" << id << ".version";
    // Check for file

    std::error_code ec;
    if (!this->getFs()->exists(opath.str(), ec))
    {
        stdplus::print(stderr, "Path: '{}' doesn't exist.\n", opath.str());
        throw IpmiException(::ipmi::ccInvalidFieldRequest);
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
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    // If value parses as expected, return version.
    VersionTuple version = std::make_tuple(0, 0, 0, 0);

    int num_fields =
        std::sscanf(value.c_str(), "%" SCNu8 ".%" SCNu8 ".%" SCNu8 ".%" SCNu8,
                    &std::get<0>(version), &std::get<1>(version),
                    &std::get<2>(version), &std::get<3>(version));
    if (num_fields == 0)
    {
        stdplus::print(stderr, "Invalid version.\n");
        throw IpmiException(::ipmi::ccUnspecifiedError);
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
        stdplus::print(stderr, "Unable to open file for output.\n");
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    ofs << "PSU_HARDRESET_DELAY=" << delay << std::endl;
    if (ofs.fail())
    {
        stdplus::print(stderr, "Write failed\n");
        ofs.close();
        throw IpmiException(::ipmi::ccUnspecifiedError);
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
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }
}

static constexpr auto RESET_ON_SHUTDOWN_FILENAME = "/run/powercycle_on_s5";

void Handler::psuResetOnShutdown() const
{
    std::ofstream ofs;
    ofs.open(RESET_ON_SHUTDOWN_FILENAME, std::ofstream::out);
    if (!ofs.good())
    {
        stdplus::print(stderr, "Unable to open file for output.\n");
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }
    ofs.close();
}

uint32_t Handler::getFlashSize()
{
    mtd_info_t info;
    int fd = open("/dev/mtd0", O_RDONLY);
    int err = ioctl(fd, MEMGETINFO, &info);
    close(fd);

    if (err)
    {
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }
    return info.size;
}

std::string Handler::getEntityName(std::uint8_t id, std::uint8_t instance)
{
    // Check if we support this Entity ID.
    auto it = _entityIdToName.find(id);
    if (it == _entityIdToName.end())
    {
        log<level::ERR>("Unknown Entity ID", entry("ENTITY_ID=%d", id));
        throw IpmiException(::ipmi::ccInvalidFieldRequest);
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
            throw IpmiException(::ipmi::ccInvalidFieldRequest);
        }
    }
    catch (InternalFailure& e)
    {
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    return entityName;
}

std::string Handler::getMachineName()
{
    const char* path = "/etc/os-release";
    std::ifstream ifs(path);
    if (ifs.fail())
    {
        stdplus::print(stderr, "Failed to open: {}\n", path);
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    std::string line;
    while (true)
    {
        std::getline(ifs, line);
        if (ifs.eof())
        {
            stdplus::print(stderr,
                           "Failed to find OPENBMC_TARGET_MACHINE: {}\n", path);
            throw IpmiException(::ipmi::ccInvalidCommand);
        }
        if (ifs.fail())
        {
            stdplus::print(stderr, "Failed to read: {}\n", path);
            throw IpmiException(::ipmi::ccUnspecifiedError);
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

static constexpr auto HOST_TIME_DELAY_FILENAME = "/run/host_poweroff_delay";
static constexpr auto HOST_POWEROFF_TARGET = "gbmc-host-poweroff.target";

void Handler::hostPowerOffDelay(std::uint32_t delay) const
{
    // Set time delay
    std::ofstream ofs;
    ofs.open(HOST_TIME_DELAY_FILENAME, std::ofstream::out);
    if (!ofs.good())
    {
        stdplus::print(stderr, "Unable to open file for output.\n");
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    ofs << "HOST_POWEROFF_DELAY=" << delay << std::endl;
    ofs.close();
    if (ofs.fail())
    {
        stdplus::print(stderr, "Write failed\n");
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    // Write succeeded, please continue.
    auto bus = sdbusplus::bus::new_default();
    auto method = bus.new_method_call(SYSTEMD_SERVICE, SYSTEMD_ROOT,
                                      SYSTEMD_INTERFACE, "StartUnit");

    method.append(HOST_POWEROFF_TARGET);
    method.append("replace");

    try
    {
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        log<level::ERR>("Failed to call Power Off",
                        entry("WHAT=%s", ex.what()));
        throw IpmiException(::ipmi::ccUnspecifiedError);
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

std::tuple<std::uint32_t, std::string> Handler::getI2cEntry(
    unsigned int entry) const
{
    return _pcie_i2c_map[entry];
}

namespace
{

static constexpr std::string_view ACCEL_OOB_ROOT = "/com/google/customAccel/";
static constexpr char ACCEL_OOB_SERVICE[] = "com.google.custom_accel";
static constexpr char ACCEL_OOB_INTERFACE[] = "com.google.custom_accel.BAR";

// C type for "a{oa{sa{sv}}}" from DBus.ObjectManager::GetManagedObjects()
using AnyType = std::variant<std::string, uint8_t, uint32_t, uint64_t>;
using AnyTypeList = std::vector<std::pair<std::string, AnyType>>;
using NamedArrayOfAnyTypeLists =
    std::vector<std::pair<std::string, AnyTypeList>>;
using ArrayOfObjectPathsAndTieredAnyTypeLists = std::vector<
    std::pair<sdbusplus::message::object_path, NamedArrayOfAnyTypeLists>>;

} // namespace

sdbusplus::bus_t Handler::getDbus() const
{
    return sdbusplus::bus::new_default();
}

const std::unique_ptr<FileSystemInterface>& Handler::getFs() const
{
    return this->fsPtr;
}

uint32_t Handler::accelOobDeviceCount() const
{
    ArrayOfObjectPathsAndTieredAnyTypeLists data;

    try
    {
        auto bus = getDbus();
        auto method = bus.new_method_call(ACCEL_OOB_SERVICE, "/",
                                          "org.freedesktop.DBus.ObjectManager",
                                          "GetManagedObjects");
        bus.call(method).read(data);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        log<level::ERR>(
            "Failed to call GetManagedObjects on com.google.custom_accel",
            entry("WHAT=%s", ex.what()));
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    return data.size();
}

std::string Handler::accelOobDeviceName(size_t index) const
{
    ArrayOfObjectPathsAndTieredAnyTypeLists data;

    try
    {
        auto bus = getDbus();
        auto method = bus.new_method_call(ACCEL_OOB_SERVICE, "/",
                                          "org.freedesktop.DBus.ObjectManager",
                                          "GetManagedObjects");
        bus.call(method).read(data);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        log<level::ERR>(
            "Failed to call GetManagedObjects on com.google.custom_accel",
            entry("WHAT=%s", ex.what()));
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    if (index >= data.size())
    {
        log<level::WARNING>(
            "Requested index is larger than the number of entries.",
            entry("INDEX=%zu", index), entry("NUM_NAMES=%zu", data.size()));
        throw IpmiException(::ipmi::ccParmOutOfRange);
    }

    std::string_view name(data[index].first.str);
    if (!name.starts_with(ACCEL_OOB_ROOT))
    {
        throw IpmiException(::ipmi::ccInvalidCommand);
    }
    name.remove_prefix(ACCEL_OOB_ROOT.length());
    return std::string(name);
}

uint64_t Handler::accelOobRead(std::string_view name, uint64_t address,
                               uint8_t num_bytes) const
{
    static constexpr char ACCEL_OOB_METHOD[] = "Read";

    std::string object_name(ACCEL_OOB_ROOT);
    object_name.append(name);

    auto bus = getDbus();
    auto method = bus.new_method_call(ACCEL_OOB_SERVICE, object_name.c_str(),
                                      ACCEL_OOB_INTERFACE, ACCEL_OOB_METHOD);
    method.append(address, static_cast<uint64_t>(num_bytes));

    std::vector<uint8_t> bytes;

    try
    {
        bus.call(method).read(bytes);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        log<level::ERR>("Failed to call Read on com.google.custom_accel",
                        entry("WHAT=%s", ex.what()),
                        entry("DBUS_SERVICE=%s", ACCEL_OOB_SERVICE),
                        entry("DBUS_OBJECT=%s", object_name.c_str()),
                        entry("DBUS_INTERFACE=%s", ACCEL_OOB_INTERFACE),
                        entry("DBUS_METHOD=%s", ACCEL_OOB_METHOD),
                        entry("DBUS_ARG_ADDRESS=%016llx", address),
                        entry("DBUS_ARG_NUM_BYTES=%zu", (size_t)num_bytes));
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    if (bytes.size() < num_bytes)
    {
        log<level::ERR>(
            "Call to Read on com.google.custom_accel didn't return the expected"
            " number of bytes.",
            entry("DBUS_SERVICE=%s", ACCEL_OOB_SERVICE),
            entry("DBUS_OBJECT=%s", object_name.c_str()),
            entry("DBUS_INTERFACE=%s", ACCEL_OOB_INTERFACE),
            entry("DBUS_METHOD=%s", ACCEL_OOB_METHOD),
            entry("DBUS_ARG_ADDRESS=%016llx", address),
            entry("DBUS_ARG_NUM_BYTES=%zu", (size_t)num_bytes),
            entry("DBUS_RETURN_SIZE=%zu", bytes.size()));
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    if (bytes.size() > sizeof(uint64_t))
    {
        log<level::ERR>(
            "Call to Read on com.google.custom_accel returned more than 8B.",
            entry("DBUS_SERVICE=%s", ACCEL_OOB_SERVICE),
            entry("DBUS_OBJECT=%s", object_name.c_str()),
            entry("DBUS_INTERFACE=%s", ACCEL_OOB_INTERFACE),
            entry("DBUS_METHOD=%s", ACCEL_OOB_METHOD),
            entry("DBUS_ARG_ADDRESS=%016llx", address),
            entry("DBUS_ARG_NUM_BYTES=%zu", (size_t)num_bytes),
            entry("DBUS_RETURN_SIZE=%zu", bytes.size()));
        throw IpmiException(::ipmi::ccReqDataTruncated);
    }

    uint64_t data = 0;
    for (size_t i = 0; i < num_bytes; ++i)
    {
        data = (data << 8) | bytes[i];
    }

    return data;
}

void Handler::accelOobWrite(std::string_view name, uint64_t address,
                            uint8_t num_bytes, uint64_t data) const
{
    static constexpr std::string_view ACCEL_OOB_METHOD = "Write";

    std::string object_name(ACCEL_OOB_ROOT);
    object_name.append(name);

    if (num_bytes > sizeof(data))
    {
        log<level::ERR>(
            "Call to Write on com.google.custom_accel requested more than 8B.",
            entry("DBUS_SERVICE=%s", ACCEL_OOB_SERVICE),
            entry("DBUS_OBJECT=%s", object_name.c_str()),
            entry("DBUS_INTERFACE=%s", ACCEL_OOB_INTERFACE),
            entry("DBUS_METHOD=%s", ACCEL_OOB_METHOD.data()),
            entry("DBUS_ARG_ADDRESS=%016llx", address),
            entry("DBUS_ARG_NUM_BYTES=%zu", (size_t)num_bytes),
            entry("DBUS_ARG_DATA=%016llx", data));
        throw IpmiException(::ipmi::ccParmOutOfRange);
    }

    std::vector<uint8_t> bytes;
    bytes.reserve(num_bytes);
    for (size_t i = 0; i < num_bytes; ++i)
    {
        bytes.emplace_back(data & 0xff);
        data >>= 8;
    }

    try
    {
        auto bus = getDbus();
        auto method =
            bus.new_method_call(ACCEL_OOB_SERVICE, object_name.c_str(),
                                ACCEL_OOB_INTERFACE, ACCEL_OOB_METHOD.data());
        method.append(address, bytes);
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        log<level::ERR>("Failed to call Write on com.google.custom_accel",
                        entry("WHAT=%s", ex.what()),
                        entry("DBUS_SERVICE=%s", ACCEL_OOB_SERVICE),
                        entry("DBUS_OBJECT=%s", object_name.c_str()),
                        entry("DBUS_INTERFACE=%s", ACCEL_OOB_INTERFACE),
                        entry("DBUS_METHOD=%s", ACCEL_OOB_METHOD.data()),
                        entry("DBUS_ARG_ADDRESS=%016llx", address),
                        entry("DBUS_ARG_NUM_BYTES=%zu", (size_t)num_bytes),
                        entry("DBUS_ARG_DATA=%016llx", data));
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }
}

std::vector<uint8_t> Handler::pcieBifurcation(uint8_t index)
{
    return bifurcationHelper.get().getBifurcation(index).value_or(
        std::vector<uint8_t>{});
}

static constexpr auto BARE_METAL_TARGET = "gbmc-bare-metal-active@0.target";

void Handler::linuxBootDone() const
{
    if (isBmcInBareMetalMode(this->fsPtr) !=
        static_cast<uint8_t>(BmcMode::BM_MODE))
    {
        return;
    }

    log<level::INFO>("LinuxBootDone: Disabling IPMI");

    // Start the bare metal active systemd target.
    auto bus = sdbusplus::bus::new_default();
    auto method = bus.new_method_call(SYSTEMD_SERVICE, SYSTEMD_ROOT,
                                      SYSTEMD_INTERFACE, "StartUnit");

    method.append(BARE_METAL_TARGET);
    method.append("replace");

    try
    {
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        log<level::ERR>("Failed to start bare metal active systemd target",
                        entry("WHAT=%s", ex.what()));
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }
}

static constexpr char ACCEL_POWER_SERVICE[] = "com.google.AccelPower";
static constexpr char ACCEL_POWER_PATH_PREFIX[] =
    "/com/google/accelPower/accel_power_";
static constexpr char POWER_MODE_IFC[] = "com.google.accelPower.Mode";

void Handler::accelSetVrSettings(::ipmi::Context::ptr ctx, uint8_t chip_id,
                                 uint8_t settings_id, uint16_t value) const
{
    int vrSettingsReq;
    boost::system::error_code ec;
    if (_vrSettingsMap.find(settings_id) == _vrSettingsMap.end())
    {
        log<level::ERR>("Settings ID is not supported",
                        entry("settings_id=%d", settings_id));
        throw IpmiException(::ipmi::ccParmOutOfRange);
    }

    vrSettingsReq = static_cast<int>(settings_id | value << 8);
    std::string object_name(
        std::format("{}{}", ACCEL_POWER_PATH_PREFIX, chip_id));

    std::variant<int> val = vrSettingsReq;
    ctx->bus->yield_method_call(
        ctx->yield, ec, ACCEL_POWER_SERVICE, object_name.c_str(),
        "org.freedesktop.DBus.Properties", "Set", POWER_MODE_IFC, "PowerMode",
        val);
    if (ec)
    {
        log<level::ERR>("Failed to set PowerMode property");
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }
}

static constexpr char EXTERNAL_SENSOR_SERVICE[] =
    "xyz.openbmc_project.ExternalSensor";
static constexpr char EXTERNAL_SENSOR_PATH_PREFIX[] =
    "/xyz/openbmc_project/sensors/power/";
static constexpr char SENSOR_VALUE_IFC[] = "xyz.openbmc_project.Sensor.Value";

uint16_t Handler::accelGetVrSettings(::ipmi::Context::ptr ctx, uint8_t chip_id,
                                     uint8_t settings_id) const
{
    Value value;
    boost::system::error_code ec;
    std::string object_name(
        std::format("{}{}{}", EXTERNAL_SENSOR_PATH_PREFIX,
                    _vrSettingsMap.at(settings_id), chip_id));

    if (_vrSettingsMap.find(settings_id) == _vrSettingsMap.end())
    {
        log<level::ERR>("Settings ID is not supported",
                        entry("settings_id=%d", settings_id));
        throw IpmiException(::ipmi::ccParmOutOfRange);
    }

    value = ctx->bus->yield_method_call<std::variant<double>>(
        ctx->yield, ec, EXTERNAL_SENSOR_SERVICE, object_name.c_str(),
        "org.freedesktop.DBus.Properties", "Get", SENSOR_VALUE_IFC, "Value");
    if (ec)
    {
        log<level::ERR>("accelGetVrSettings: Failed to call GetObject ");
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }

    return static_cast<uint16_t>(std::get<double>(value));
}

std::string Handler::getBMInstanceProperty(uint8_t propertyType) const
{
    std::string propertyTypeString;
    if (auto it = bmInstanceTypeStringMap.find(propertyType);
        it == bmInstanceTypeStringMap.end())
    {
        stdplus::print(stderr, "PropertyType: '{}' is invalid.\n",
                       propertyType);
        throw IpmiException(::ipmi::ccInvalidFieldRequest);
    }
    else
    {
        propertyTypeString = it->second;
    }
    std::string opath = std::format("/run/bm-instance/{}", propertyTypeString);
    // Check for file

    std::error_code ec;
    // TODO(brandonkim@google.com): Fix this to use stdplus::ManagedFd
    if (!this->getFs()->exists(opath, ec))
    {
        stdplus::print(stderr, "Path: '{}' doesn't exist.\n", opath);
        throw IpmiException(::ipmi::ccInvalidFieldRequest);
    }

    std::ifstream ifs;
    ifs.exceptions(std::ifstream::failbit);
    std::string property;
    try
    {
        ifs.open(opath);
        std::getline(ifs, property);
    }
    catch (std::ios_base::failure& fail)
    {
        stdplus::print(stderr, "Failed to read: '{}'.\n", opath);
        throw IpmiException(::ipmi::ccUnspecifiedError);
    }
    return property;
}

} // namespace ipmi
} // namespace google
