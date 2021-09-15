#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct PcieSlotCountReply
{
    uint8_t value;
} __attribute__((packed));

struct PcieSlotI2cBusMappingReply
{
    uint8_t i2c_bus_number;
    uint8_t pcie_slot_name_len;
} __attribute__((packed));

//  Handle the pcie slot count command.
//  Sys can query the number of pcie slots.
Resp pcieSlotCount(const std::vector<std::uint8_t>& data,
                   HandlerInterface* handler);

// Handle the pcie slot to i2c bus mapping command.
// Sys can query which i2c bus is routed to which pcie slot.
Resp pcieSlotI2cBusMapping(const std::vector<std::uint8_t>& data,
                           HandlerInterface* handler);

} // namespace ipmi
} // namespace google
