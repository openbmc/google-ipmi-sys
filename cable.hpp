#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct CableReply
{
    uint8_t value;
} __attribute__((packed));

//
// Handle the cablecheck.  Sys must supply which ethernet device they're
// interested in.
Resp cableCheck(const std::vector<std::uint8_t>& data,
                const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
