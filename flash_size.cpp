/*
 * Copyright 2021 Google Inc.
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

#include "flash_size.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace google
{
namespace ipmi
{

struct GetFlashSizeRequest
{
    uint8_t subcommand;
} __attribute__((packed));

struct GetFlashSizeReply
{
    uint8_t subcommand;
    uint8_t flashSize;
} __attribute__((packed));

#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

ipmi_ret_t getFlashSize(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, HandlerInterface* handler)
{
    struct GetFlashSizeRequest request;

    if ((*dataLen) < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    std::memcpy(&request, &reqBuf[0], sizeof(request));
    uint8_t flashSize;
    try
    {
        flashSize = handler->getFlashSize();
    }
    catch (const IpmiException& e)
    {
        return e.getIpmiError();
    }

    auto reply = reinterpret_cast<struct GetFlashSizeReply*>(&replyBuf[0]);
    reply->subcommand = SysGetFlashSize;
    reply->flashSize = flashSize;

    (*dataLen) = sizeof(struct GetFlashSizeReply);
    return IPMI_CC_OK;
}
} // namespace ipmi
} // namespace google
