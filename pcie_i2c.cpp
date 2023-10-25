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

#include "pcie_i2c.hpp"

#include "commands.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>
#include <stdplus/print.hpp>

#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <tuple>
#include <vector>

namespace google
{
namespace ipmi
{

#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

struct PcieSlotI2cBusMappingRequest
{
    uint8_t entry;
} __attribute__((packed));

Resp pcieSlotCount(std::span<const uint8_t>, HandlerInterface* handler)
{
    // If there are already entries in the vector, clear them.
    handler->buildI2cPcieMapping();

    // Fill the pcie slot count as the number of entries in the vector.
    std::uint8_t value = handler->getI2cPcieMappingSize();

    return ::ipmi::responseSuccess(SysOEMCommands::SysPcieSlotCount,
                                   std::vector<std::uint8_t>{value});
}

Resp pcieSlotI2cBusMapping(std::span<const uint8_t> data,
                           HandlerInterface* handler)
{
    struct PcieSlotI2cBusMappingRequest request;

    if (data.size() < sizeof(request))
    {
        stdplus::print(stderr, "Invalid command length: {}\n", data.size());
        return ::ipmi::responseReqDataLenInvalid();
    }

    // If there are no entries in the vector return error.
    size_t mapSize = handler->getI2cPcieMappingSize();
    if (mapSize == 0)
    {
        return ::ipmi::responseInvalidReservationId();
    }

    std::memcpy(&request, data.data(), sizeof(request));

    // The valid entries range from 0 to N - 1, N being the total number of
    // entries in the vector.
    if (request.entry >= mapSize)
    {
        return ::ipmi::responseParmOutOfRange();
    }

    // Get the i2c bus number and the pcie slot name from the vector.
    auto i2cEntry = handler->getI2cEntry(request.entry);
    uint32_t i2c_bus_number = std::get<0>(i2cEntry);
    std::string pcie_slot_name = std::get<1>(i2cEntry);

    int length = sizeof(struct PcieSlotI2cBusMappingReply) +
                 pcie_slot_name.length();

    // TODO (jaghu) : Add a way to dynamically receive the MAX_IPMI_BUFFER
    // value and change error to IPMI_CC_REQUESTED_TOO_MANY_BYTES.
    if (length > MAX_IPMI_BUFFER)
    {
        stdplus::print(stderr, "Response would overflow response buffer\n");
        return ::ipmi::responseInvalidCommand();
    }

    std::vector<std::uint8_t> reply;
    reply.reserve(pcie_slot_name.length() +
                  sizeof(struct PcieSlotI2cBusMappingReply));
    // Copy the i2c bus number and the pcie slot name to the reply struct.
    reply.emplace_back(i2c_bus_number);          /* i2c_bus_number */
    reply.emplace_back(pcie_slot_name.length()); /* pcie_slot_name length */
    reply.insert(reply.end(), pcie_slot_name.begin(),
                 pcie_slot_name.end());          /* pcie_slot_name */

    return ::ipmi::responseSuccess(SysOEMCommands::SysPcieSlotI2cBusMapping,
                                   reply);
}
} // namespace ipmi
} // namespace google
