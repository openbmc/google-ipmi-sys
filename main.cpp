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

#include "main.hpp"

#include "cable.hpp"
#include "cpld.hpp"
#include "eth.hpp"
#include "psu.hpp"

#include <host-ipmid/ipmid-api.h>

#include <cstdint>
#include <experimental/filesystem>
#include <fstream>
#include <host-ipmid/iana.hpp>
#include <host-ipmid/oemrouter.hpp>
#include <sstream>
#include <string>
#include <system_error>

namespace oem
{
namespace google
{
constexpr int sysCmd = 50;
} // namespace google
} // namespace oem

namespace google
{
namespace ipmi
{

static ipmi_ret_t HandleSysCommand(ipmi_cmd_t cmd, const uint8_t* reqBuf,
                                   uint8_t* replyCmdBuf, size_t* dataLen)
{
    // Verify it's at least as long as it needs to be for a subcommand.
    if ((*dataLen) < 1)
    {
        std::fprintf(stderr, "*dataLen too small: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_INVALID;
    }

    switch (reqBuf[0])
    {
        case SysCableCheck:
            return CableCheck(reqBuf, replyCmdBuf, dataLen);
        case SysCpldVersion:
            return CpldVersion(reqBuf, replyCmdBuf, dataLen);
        case SysGetEthDevice:
            return GetEthDevice(reqBuf, replyCmdBuf, dataLen);
        case SysPsuHardReset:
            return PsuHardReset(reqBuf, replyCmdBuf, dataLen);
        default:
            std::fprintf(stderr, "Invalid subcommand: 0x%x\n", reqBuf[0]);
            return IPMI_CC_INVALID;
    }
}

void setupGlobalOemCableCheck() __attribute__((constructor));

void setupGlobalOemCableCheck()
{
    oem::Router* oemRouter = oem::mutableRouter();

    std::fprintf(stderr,
                 "Registering OEM:[%#08X], Cmd:[%#04X] for Sys Commands\n",
                 oem::googOemNumber, oem::google::sysCmd);

    oemRouter->registerHandler(oem::googOemNumber, oem::google::sysCmd,
                               HandleSysCommand);
}

} // namespace ipmi
} // namespace google
