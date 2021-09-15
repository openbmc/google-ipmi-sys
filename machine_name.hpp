#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct GetMachineNameReply
{
    uint8_t machineNameLength;
} __attribute__((packed));

// Handle the machine name command.
Resp getMachineName(const std::vector<std::uint8_t>& data,
                    HandlerInterface* handler);

} // namespace ipmi
} // namespace google
