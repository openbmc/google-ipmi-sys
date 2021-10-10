// Copyright 2022 Google LLC
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

#include "pcie_bifurcation.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <fmt/format.h>

#include <ipmid/api-types.hpp>
#include <ipmid/message.hpp>

#include <span>
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

Resp pcieBifurcation(::ipmi::Context::ptr ctx, std::span<const uint8_t> data,
                     HandlerInterface* handler, bool dynamic)
{
    if (data.size() < sizeof(struct PcieBifurcationRequest))
    {
        fmt::print(stderr, "Invalid command length: {}\n",
                   static_cast<uint32_t>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }

    auto bifurcation = handler->pcieBifurcation(ctx, /*index=*/data[0],
                                                dynamic);

    int length = sizeof(struct PcieBifurcationReply) + bifurcation.size();

    if (length > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr, "Response would overflow response buffer\n");
        return ::ipmi::responseInvalidCommand();
    }

    std::vector<std::uint8_t> reply;
    reply.reserve(bifurcation.size() + sizeof(struct PcieBifurcationReply));
    reply.emplace_back(bifurcation.size()); /* bifurcationLength */
    reply.insert(reply.end(), bifurcation.begin(),
                 bifurcation.end());        /* bifurcation */

    return ::ipmi::responseSuccess(SysOEMCommands::SysPCIeSlotBifurcation,
                                   reply);
}
} // namespace ipmi
} // namespace google
