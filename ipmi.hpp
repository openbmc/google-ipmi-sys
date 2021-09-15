#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <ipmid/message.hpp>
#include <optional>
#include <vector>

namespace google
{
namespace ipmi
{

// Handle the google-ipmi-sys IPMI OEM commands.
Resp handleSysCommand(HandlerInterface* handler, ::ipmi::Context::ptr ctx,
                      uint8_t cmd, std::optional<std::vector<uint8_t>> data);

} // namespace ipmi
} // namespace google
