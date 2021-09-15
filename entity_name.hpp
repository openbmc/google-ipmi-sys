#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct GetEntityNameReply
{
    uint8_t entityNameLength;
} __attribute__((packed));

// Handle the "entity id:entity instance" to entity name mapping command.
// Sys can query the entity name for a particular "entity id:entity instance".
Resp getEntityName(const std::vector<std::uint8_t>& data,
                   HandlerInterface* handler);

} // namespace ipmi
} // namespace google
