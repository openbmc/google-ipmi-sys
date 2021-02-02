#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

struct PsuResetRequest
{
    uint8_t subcommand;
    // Delay in seconds.
    uint32_t delay;
} __attribute__((packed));

struct PsuResetOnShutdownRequest
{
    uint8_t subcommand;
} __attribute__((packed));

// Set a time-delayed PSU hard reset.
ipmi_ret_t psuHardReset(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, const HandlerInterface* handler);

// Arm for PSU hard reset on host shutdown.
ipmi_ret_t psuHardResetOnShutdown(const uint8_t* reqBuf, uint8_t* replyBuf,
                                  size_t* dataLen,
                                  const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
