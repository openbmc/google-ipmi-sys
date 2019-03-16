#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

//  Handle the pcie slot count command.
//  Sys can query the number of pcie slots.
ipmi_ret_t PcieSlotCount(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen,
                         HandlerInterface* handler = &handlerImpl);

// Handle the pcie slot to i2c bus mapping command.
// Sys can query which i2c bus is routed to which pcie slot.
ipmi_ret_t PcieSlotI2cBusMapping(const uint8_t* reqBuf, uint8_t* replyBuf,
                                 size_t* dataLen,
                                 HandlerInterface* handler = &handlerImpl);

} // namespace ipmi
} // namespace google
