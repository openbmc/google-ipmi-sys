#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

//  Handle the pcie slot count command.
//  Sys can query the number of pcie slots.
ipmi_ret_t pcieSlotCount(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen, HandlerInterface* handler);

// Handle the pcie slot to i2c bus mapping command.
// Sys can query which i2c bus is routed to which pcie slot.
ipmi_ret_t pcieSlotI2cBusMapping(const uint8_t* reqBuf, uint8_t* replyBuf,
                                 size_t* dataLen, HandlerInterface* handler);

} // namespace ipmi
} // namespace google
