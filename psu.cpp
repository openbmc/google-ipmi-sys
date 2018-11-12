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

#include "psu.hpp"

#include "main.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>

namespace google
{
namespace ipmi
{

using namespace phosphor::logging;

struct PsuResetRequest
{
    uint8_t subcommand;
    // Delay in seconds.
    uint32_t delay;
} __attribute__((packed));

static constexpr auto TIME_DELAY_FILENAME = "/run/psu_timedelay";
static constexpr auto SYSTEMD_SERVICE = "org.freedesktop.systemd1";
static constexpr auto SYSTEMD_ROOT = "/org/freedesktop/systemd1";
static constexpr auto SYSTEMD_INTERFACE = "org.freedesktop.systemd1.Manager";
static constexpr auto PSU_HARDRESET_TARGET = "gbmc-psu-hardreset.target";

ipmi_ret_t PsuHardReset(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen)
{
    struct PsuResetRequest request;

    if ((*dataLen) < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    std::memcpy(&request, &reqBuf[0], sizeof(struct PsuResetRequest));

    std::ofstream ofs;
    ofs.open(TIME_DELAY_FILENAME, std::ofstream::out);
    if (!ofs.good())
    {
        std::fprintf(stderr, "Unable to open file for output.\n");
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    ofs << "PSU_HARDRESET_DELAY=" << request.delay << std::endl;
    if (ofs.fail())
    {
        std::fprintf(stderr, "Write failed\n");
        ofs.close();
        return IPMI_CC_UNSPECIFIED_ERROR;
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
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    replyBuf[0] = SysPsuHardReset;
    (*dataLen) = sizeof(uint8_t);

    return IPMI_CC_OK;
}

} // namespace ipmi
} // namespace google
