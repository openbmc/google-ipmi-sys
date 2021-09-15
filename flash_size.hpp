#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct GetFlashSizeReply
{
    uint32_t flashSize;
} __attribute__((packed));

Resp getFlashSize(const std::vector<std::uint8_t>& data,
                  HandlerInterface* handler);

} // namespace ipmi
} // namespace google
