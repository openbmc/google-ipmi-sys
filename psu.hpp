#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct PsuResetRequest
{
    // Delay in seconds.
    uint32_t delay;
} __attribute__((packed));

struct PsuResetOnShutdownRequest
{
} __attribute__((packed));

// Set a time-delayed PSU hard reset.
Resp psuHardReset(const std::vector<std::uint8_t>& data,
                  const HandlerInterface* handler);

// Arm for PSU hard reset on host shutdown.
Resp psuHardResetOnShutdown(const std::vector<std::uint8_t>& data,
                            const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
