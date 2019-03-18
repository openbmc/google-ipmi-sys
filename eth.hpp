#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

// The reply to the ethdevice command specifies the
// IPMI channel number and the ifName used for the
// ncis connection.
struct EthDeviceReply
{
    uint8_t subcommand;
    uint8_t channel;
    // ifNameLength doesn't include the null-terminator.
    uint8_t ifNameLength;
    uint8_t ifName[0];
} __attribute__((packed));

// Handle the eth query command.
// Sys can query the ifName and IPMI channel of the BMC's NCSI ethernet
// device.
ipmi_ret_t getEthDevice(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen,
                        const HandlerInterface* handler = &handlerImpl);

} // namespace ipmi
} // namespace google
