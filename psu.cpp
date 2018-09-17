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
    if ((*dataLen) < sizeof(struct PsuResetRequest))
    {
        fprintf(stderr, "Invalid command length: %lu\n", (*dataLen));
        return IPMI_CC_INVALID;
    }

    struct PsuResetRequest request;
    memcpy(&request, &reqBuf[0], sizeof(struct PsuResetRequest));

    std::ofstream ofs;
    ofs.open(TIME_DELAY_FILENAME, std::ofstream::out);
    if (!ofs.good())
    {
        fprintf(stderr, "Unable to open file for output.\n");
        return IPMI_CC_INVALID;
    }

    ofs << "PSU_HARDRESET_DELAY=" << request.delay << std::endl;
    if (ofs.fail())
    {
        fprintf(stderr, "Write failed\n");
        ofs.close();
        return IPMI_CC_INVALID;
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
        return IPMI_CC_INVALID;
    }

    replyBuf[0] = SysPsuHardReset;
    (*dataLen) = sizeof(uint8_t);

    return IPMI_CC_OK;
}

} // namespace ipmi
} // namespace google
