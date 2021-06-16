#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

struct HostPowerOffRequest
{
    uint8_t subcommand;
    // Delay in seconds.
    uint32_t delay;
} __attribute__((packed));

// Disable the fallback watchdog with given time delay and Power Off Host
ipmi_ret_t hostPowerOff(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
