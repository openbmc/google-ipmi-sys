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

#include "errors.hpp"
#include "handler.hpp"
#include "ipmi.hpp"

#include <cstdint>
#include <cstring>

namespace google
{
namespace ipmi
{

ipmi_ret_t psuHardReset(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, const HandlerInterface* handler)
{
    struct PsuResetRequest request;

    if ((*dataLen) < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    std::memcpy(&request, &reqBuf[0], sizeof(struct PsuResetRequest));
    try
    {
        handler->psuResetDelay(request.delay);
    }
    catch (const IpmiException& e)
    {
        return e.getIpmiError();
    }

    replyBuf[0] = SysPsuHardReset;
    (*dataLen) = sizeof(uint8_t);

    return IPMI_CC_OK;
}

} // namespace ipmi
} // namespace google
