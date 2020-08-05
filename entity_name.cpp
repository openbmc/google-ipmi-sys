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

#include "entity_name.hpp"

#include "errors.hpp"
#include "handler.hpp"
#include "ipmi.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace google
{
namespace ipmi
{

namespace
{

// TODO (jaghu) : Add a call to get getChannelMaxTransferSize.
#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

} // namespace

struct GetEntityNameRequest
{
    uint8_t subcommand;
    uint8_t entityId;
    uint8_t entityInstance;
} __attribute__((packed));

struct GetEntityNameReply
{
    uint8_t subcommand;
    uint8_t entityNameLength;
    uint8_t entityName[0];
} __attribute__((packed));

ipmi_ret_t getEntityName(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen, HandlerInterface* handler)
{
    struct GetEntityNameRequest request;

    if ((*dataLen) < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    std::memcpy(&request, &reqBuf[0], sizeof(request));
    std::string entityName;
    try
    {
        entityName =
            handler->getEntityName(request.entityId, request.entityInstance);
    }
    catch (const IpmiException& e)
    {
        return e.getIpmiError();
    }

    int length = sizeof(struct GetEntityNameReply) + entityName.length();

    // TODO (jaghu) : Add a call to get getChannelMaxTransferSize.
    if (length > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr, "Response would overflow response buffer\n");
        return IPMI_CC_INVALID;
    }

    auto reply = reinterpret_cast<struct GetEntityNameReply*>(&replyBuf[0]);
    reply->subcommand = SysEntityName;
    reply->entityNameLength = entityName.length();
    std::memcpy(reply->entityName, entityName.c_str(), entityName.length());

    (*dataLen) = length;
    return IPMI_CC_OK;
}
} // namespace ipmi
} // namespace google
