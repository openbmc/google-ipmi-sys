#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

// Handle the "entity id:entity instance" to entity name mapping command.
// Sys can query the entity name for a particular "entity id:entity instance".
ipmi_ret_t getEntityName(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen, HandlerInterface* handler);

} // namespace ipmi
} // namespace google
