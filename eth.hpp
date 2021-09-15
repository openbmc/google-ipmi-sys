#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

// The reply to the ethdevice command specifies the
// IPMI channel number and the ifName used for the
// ncis connection.
struct EthDeviceReply
{
    uint8_t channel;
    // ifNameLength doesn't include the null-terminator.
    uint8_t ifNameLength;
} __attribute__((packed));

// Handle the eth query command.
// Sys can query the ifName and IPMI channel of the BMC's NCSI ethernet
// device.
Resp getEthDevice(const std::vector<std::uint8_t>& data,
                  const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
