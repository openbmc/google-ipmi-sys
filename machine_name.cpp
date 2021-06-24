/*
 * Copyright 2020 Google Inc.
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

#include "machine_name.hpp"

#include "errors.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>

namespace google
{
namespace ipmi
{

struct GetMachineNameRequest
{
    uint8_t subcommand;
} __attribute__((packed));

struct GetMachineNameReply
{
    uint8_t subcommand;
    uint8_t machineNameLength;
} __attribute__((packed));

ipmi_ret_t getMachineName(const uint8_t* reqBuf, uint8_t* replyBuf,
                          size_t* dataLen, HandlerInterface* handler)
{
    GetMachineNameRequest request;
    if (*dataLen < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %zu\n", *dataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }
    std::memcpy(&request, reqBuf, sizeof(request));

    static std::optional<std::string> machineName;
    if (!machineName)
    {
        try
        {
            machineName = handler->getMachineName();
        }
        catch (const IpmiException& e)
        {
            return e.getIpmiError();
        }
    }

    GetMachineNameReply reply;
    size_t len = sizeof(reply) + machineName->size();
    if (len > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr, "Response would overflow response buffer\n");
        return IPMI_CC_INVALID;
    }
    reply.subcommand = request.subcommand;
    reply.machineNameLength = machineName->size();
    std::memcpy(replyBuf, &reply, sizeof(reply));
    std::memcpy(replyBuf + sizeof(reply), machineName->data(),
                machineName->size());
    (*dataLen) = len;
    return IPMI_CC_OK;
}

} // namespace ipmi
} // namespace google
