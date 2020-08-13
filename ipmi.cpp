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

#include "ipmi.hpp"

#include "cable.hpp"
#include "commands.hpp"
#include "cpld.hpp"
#include "entity_name.hpp"
#include "eth.hpp"
#include "handler_impl.hpp"
#include "pcie_i2c.hpp"
#include "psu.hpp"

#include <ipmid/api.h>

#include <cstdint>
#include <cstdio>

namespace google
{
namespace ipmi
{

ipmi_ret_t handleSysCommand(HandlerInterface* handler, ipmi_cmd_t cmd,
                            const uint8_t* reqBuf, uint8_t* replyCmdBuf,
                            size_t* dataLen)
{
    // Verify it's at least as long as it needs to be for a subcommand.
    if ((*dataLen) < 1)
    {
        std::fprintf(stderr, "*dataLen too small: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    switch (reqBuf[0])
    {
        case SysCableCheck:
            return cableCheck(reqBuf, replyCmdBuf, dataLen, handler);
        case SysCpldVersion:
            return cpldVersion(reqBuf, replyCmdBuf, dataLen, handler);
        case SysGetEthDevice:
            return getEthDevice(reqBuf, replyCmdBuf, dataLen, handler);
        case SysPsuHardReset:
            return psuHardReset(reqBuf, replyCmdBuf, dataLen, handler);
        case SysPcieSlotCount:
            return pcieSlotCount(reqBuf, replyCmdBuf, dataLen, handler);
        case SysPcieSlotI2cBusMapping:
            return pcieSlotI2cBusMapping(reqBuf, replyCmdBuf, dataLen, handler);
        case SysEntityName:
            return getEntityName(reqBuf, replyCmdBuf, dataLen, handler);
        default:
            std::fprintf(stderr, "Invalid subcommand: 0x%x\n", reqBuf[0]);
            return IPMI_CC_INVALID;
    }
}

} // namespace ipmi
} // namespace google
