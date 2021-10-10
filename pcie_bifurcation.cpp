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

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"
#include "pcie_b.hpp"

#include <cstdint>
#include <cstring>
#include <ipmid/api-types.hpp>
#include <string>
#include <vector>

namespace google
{
namespace ipmi
{

namespace
{
#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif
} // namespace

struct PcieBifurcationRequest
{
    uint8_t pcieIndex;
} __attribute__((packed));

Resp pcieBifurcation(const std::vector<std::uint8_t>&,
                     HandlerInterface* handler)
{
    if (data.size() < sizeof(struct PcieBifurcationRequest))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }

    // uint8_t index = data[0];
    // Hardcoded to 5 for now.
    uint8_t bus = 5;
    auto bifurcation = handler->pcieBifurcation(bus);

    int length = sizeof(struct PcieBifurcationReply) + bifurcation.length();

    if (length > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr, "Response would overflow response buffer\n");
        return ::ipmi::responseInvalidCommand();
    }

    std::vector<std::uint8_t> reply;
    reply.reserve(entityName.length() + sizeof(struct PcieBifurcationReply));
    reply.emplace_back(bifurcation.length()); /* bifurcationLength */
    reply.insert(reply.end(), entityName.begin(),
                 entityName.end()); /* bifurcation */

    return ::ipmi::responseSuccess(SysOEMCommands::SysPCIeSlotBifurcation,
                                   reply);
}
} // namespace ipmi
} // namespace google
