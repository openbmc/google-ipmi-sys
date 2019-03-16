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

#include "pcie_i2c.hpp"

#include "main.hpp"
#include "util.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>

namespace google
{
namespace ipmi
{
namespace fs = std::filesystem;

namespace
{

#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

std::vector<std::tuple<uint32_t, std::string>> pcie_i2c_map;

} // namespace

struct PcieSlotCountRequest
{
    uint8_t subcommand;
} __attribute__((packed));

struct PcieSlotCountReply
{
    uint8_t subcommand;
    uint8_t value;
} __attribute__((packed));

struct PcieSlotI2cBusMappingRequest
{
    uint8_t subcommand;
    uint8_t entry;
} __attribute__((packed));

struct PcieSlotI2cBusMappingReply
{
    uint8_t subcommand;
    uint8_t i2c_bus_number;
    uint8_t pcie_slot_name_len;
    uint8_t pcie_slot_name[0];
} __attribute__((packed));

ipmi_ret_t PcieSlotCount(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen)
{
    if ((*dataLen) < sizeof(struct PcieSlotCountRequest))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    // If there are already entries in the vector, clear them.
    pcie_i2c_map = buildPcieMap();

    struct PcieSlotCountReply reply;
    reply.subcommand = SysPcieSlotCount;
    // Fill the pcie slot count as the number of entries in the vector.
    reply.value = pcie_i2c_map.size();

    std::memcpy(&replyBuf[0], &reply, sizeof(reply));

    // Return the subcommand and the result.
    (*dataLen) = sizeof(reply);

    return IPMI_CC_OK;
}

ipmi_ret_t PcieSlotI2cBusMapping(const uint8_t* reqBuf, uint8_t* replyBuf,
                                 size_t* dataLen)
{
    struct PcieSlotI2cBusMappingRequest request;

    if ((*dataLen) < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    // If there are no entries in the vector return error.
    if (pcie_i2c_map.empty())
    {
        return IPMI_CC_INVALID_RESERVATION_ID;
    }

    std::memcpy(&request, &reqBuf[0], sizeof(request));

    // The valid entries range from 0 to N - 1, N being the total number of
    // entries in the vector.
    if (request.entry >= pcie_i2c_map.size())
    {
        return IPMI_CC_PARM_OUT_OF_RANGE;
    }

    // Get the i2c bus number and the pcie slot name from the vector.
    uint32_t i2c_bus_number = std::get<0>(pcie_i2c_map[request.entry]);
    std::string pcie_slot_name = std::get<1>(pcie_i2c_map[request.entry]);

    int length =
        sizeof(struct PcieSlotI2cBusMappingReply) + pcie_slot_name.length();

    // TODO (jaghu) : Add a way to dynamically receive the MAX_IPMI_BUFFER
    // value and change error to IPMI_CC_REQUESTED_TOO_MANY_BYTES.
    if (length > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr, "Response would overflow response buffer\n");
        return IPMI_CC_INVALID;
    }

    auto reply =
        reinterpret_cast<struct PcieSlotI2cBusMappingReply*>(&replyBuf[0]);
    reply->subcommand = SysPcieSlotI2cBusMapping;
    // Copy the i2c bus number and the pcie slot name to the reply struct.
    reply->i2c_bus_number = i2c_bus_number;
    reply->pcie_slot_name_len = pcie_slot_name.length();
    std::memcpy(reply->pcie_slot_name, pcie_slot_name.c_str(),
                pcie_slot_name.length());

    // Return the subcommand and the result.
    (*dataLen) = length;
    return IPMI_CC_OK;
}
} // namespace ipmi
} // namespace google
