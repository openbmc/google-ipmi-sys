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

#include <cstdint>
#include <cstring>
#include <experimental/filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>

namespace google
{
namespace ipmi
{
namespace fs = std::experimental::filesystem;

namespace
{

#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

std::vector<std::tuple<uint32_t, std::string>> pcie_i2c_map;

int read_file(const std::string& file_name, std::string& file_content)
{
    std::error_code ec;
    if (fs::exists(file_name, ec))
    {
        std::ifstream ifs;
        ifs.exceptions(std::ifstream::failbit);
        try
        {
            ifs.open(file_name);
            ifs >> file_content;
        }
        catch (std::ios_base::failure& fail)
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

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
        return IPMI_CC_INVALID;
    }

    if (!pcie_i2c_map.empty())
        pcie_i2c_map.clear();

    std::string i2c_dev_dir_path = "/sys/bus/i2c/devices";

    // Build a hashmap with i2c bus to pcie slot mapping.
    for (auto& i2c_dev : fs::directory_iterator(i2c_dev_dir_path))
    {
        std::string i2c_dev_path = i2c_dev.path();
        std::string::size_type i2c_dev_string_number =
            i2c_dev_path.find("i2c-");
        if (i2c_dev_string_number != std::string::npos)
        {
            std::string pcie_slot_path = i2c_dev_path + "/of_node/pcie-slot";
            std::string pcie_slot;
            if (read_file(pcie_slot_path, pcie_slot) < 0)
            {
                continue;
            }
            std::string pcie_slot_name;
            std::string pcie_slot_full_path;
            pcie_slot_full_path.append("/sys/firmware/devicetree/base");
            pcie_slot_full_path.append(
                pcie_slot.substr(0, pcie_slot.size() - 1));
            pcie_slot_full_path.append("/label");
            if (read_file(pcie_slot_full_path, pcie_slot_name) < 0)
            {
                continue;
            }
            std::stringstream i2c_bus_string(
                i2c_dev_path.substr(i2c_dev_string_number + 4));
            uint32_t i2c_bus_number = 0;
            i2c_bus_string >> i2c_bus_number;
            pcie_i2c_map.push_back(
                std::make_tuple(i2c_bus_number, pcie_slot_name));
        }
    }

    struct PcieSlotCountReply reply;
    reply.subcommand = SysPcieSlotCount;
    // Fill the pcie slot count as the number of entries in the hashmap.
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
        return IPMI_CC_INVALID;
    }

    if (pcie_i2c_map.empty())
    {
        return IPMI_CC_INVALID;
    }

    std::memcpy(&request, &reqBuf[0], sizeof(request));

    if (request.entry > pcie_i2c_map.size() || request.entry == 0)
    {
        return IPMI_CC_INVALID;
    }

    uint32_t i2c_bus_number = std::get<0>(pcie_i2c_map[request.entry - 1]);
    std::string pcie_slot_name = std::get<1>(pcie_i2c_map[request.entry - 1]);

    // We are seeing a weird behavior.  In this build, length() on a
    // basic_string is returning a value that includes the null terminator.
    // Hence using "pcie_slot_name.length() - 1".
    int length =
        sizeof(struct PcieSlotI2cBusMappingReply) + pcie_slot_name.length() - 1;

    // TODO (jaghu) : Add a way to dynamically receive the MAX_IPMI_BUFFER
    // value.
    if (length > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr, "Response would overflow response buffer\n");
        return IPMI_CC_INVALID;
    }

    auto reply =
        reinterpret_cast<struct PcieSlotI2cBusMappingReply*>(&replyBuf[0]);
    reply->subcommand = SysPcieSlotI2cBusMapping;
    reply->i2c_bus_number = i2c_bus_number;
    reply->pcie_slot_name_len = pcie_slot_name.length() - 1;
    std::memcpy(reply->pcie_slot_name, pcie_slot_name.c_str(),
                pcie_slot_name.length() - 1);

    // Return the subcommand and the result.
    (*dataLen) = length;
    return IPMI_CC_OK;
}
} // namespace ipmi
} // namespace google
