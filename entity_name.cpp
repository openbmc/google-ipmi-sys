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

#include "entity_name.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>

#include <cstdint>
#include <cstring>
#include <span>
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
    uint8_t entityId;
    uint8_t entityInstance;
} __attribute__((packed));

Resp getEntityName(std::span<const uint8_t> data, HandlerInterface* handler)
{
    struct GetEntityNameRequest request;

    if (data.size() < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }

    std::memcpy(&request, data.data(), sizeof(request));
    std::string entityName;
    try
    {
        entityName = handler->getEntityName(request.entityId,
                                            request.entityInstance);
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    int length = sizeof(struct GetEntityNameReply) + entityName.length();

    // TODO (jaghu) : Add a call to get getChannelMaxTransferSize.
    if (length > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr, "Response would overflow response buffer\n");
        return ::ipmi::responseInvalidCommand();
    }

    std::vector<std::uint8_t> reply;
    reply.reserve(entityName.length() + sizeof(struct GetEntityNameReply));
    reply.emplace_back(entityName.length()); /* entityNameLength */
    reply.insert(reply.end(), entityName.begin(),
                 entityName.end());          /* entityName */

    return ::ipmi::responseSuccess(SysOEMCommands::SysEntityName, reply);
}
} // namespace ipmi
} // namespace google
