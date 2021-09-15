#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct HostPowerOffRequest
{
    // Delay in seconds.
    uint32_t delay;
} __attribute__((packed));

// Disable the fallback watchdog with given time delay and Power Off Host
Resp hostPowerOff(const std::vector<std::uint8_t>& data,
                  const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
