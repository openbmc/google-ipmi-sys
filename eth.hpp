#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

// The reply to the ethdevice command specifies the
// IPMI channel number and the if_name used for the
// ncis connection.
struct EthDeviceReply
{
    uint8_t subcommand;
    uint8_t channel;
    // if_name_len doesn't include the null-terminator.
    uint8_t if_name_len;
    uint8_t if_name[0];
} __attribute__((packed));

// Handle the eth query command.
// Sys can query the if_name and IPMI channel of the BMC's NCSI ethernet
// device.
ipmi_ret_t GetEthDevice(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen,
                        const HandlerInterface* handler = &handlerImpl);

} // namespace ipmi
} // namespace google
