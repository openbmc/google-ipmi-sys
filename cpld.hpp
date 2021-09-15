#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct CpldReply
{
    uint8_t major;
    uint8_t minor;
    uint8_t point;
    uint8_t subpoint;
} __attribute__((packed));

// Given a cpld identifier, return a version if available.
Resp cpldVersion(const std::vector<std::uint8_t>& data,
                 const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
