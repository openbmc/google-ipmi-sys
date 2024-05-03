// Copyright 2024 Google LLC
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

#include "bm_instance.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>
#include <stdplus/print.hpp>

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

struct BMInstancePropertyRequest
{
    std::uint8_t bmInstancePropertyType;
} __attribute__((packed));

Resp getBMInstanceProperty(std::span<const uint8_t> data,
                           HandlerInterface* handler)
{
    if (data.size() < sizeof(struct BMInstancePropertyRequest))
    {
        stdplus::print(stderr, "Invalid command length: {}\n",
                       static_cast<uint32_t>(data.size()));
        return ::ipmi::responseReqDataLenInvalid();
    }

    std::string bmInstanceProperty =
        handler->getBMInstanceProperty(/*type=*/data[0]);

    const size_t length = sizeof(struct BMInstancePropertyReply) +
                          bmInstanceProperty.size();

    if (length > MAX_IPMI_BUFFER)
    {
        stdplus::print(stderr, "Response would overflow response buffer\n");
        return ::ipmi::responseInvalidCommand();
    }

    std::vector<std::uint8_t> reply;
    reply.reserve(length);
    reply.emplace_back(bmInstanceProperty.size());
    reply.insert(reply.end(), bmInstanceProperty.begin(),
                 bmInstanceProperty.end());

    return ::ipmi::responseSuccess(SysOEMCommands::SysGetBMInstanceProperty,
                                   reply);
}
} // namespace ipmi
} // namespace google
