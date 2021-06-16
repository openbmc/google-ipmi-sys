#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

struct HostS5PowerOffRequest
{
    uint8_t subcommand;
    // Delay in seconds.
    uint32_t delay;
} __attribute__((packed));

// Host S5 disable the fallback watchdog and Power Off Host
ipmi_ret_t hostS5PowerOff(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
