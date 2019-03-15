#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

//
// Handle the cablecheck.  Sys must supply which ethernet device they're
// interested in.
ipmi_ret_t CableCheck(const uint8_t* reqBuf, uint8_t* replyBuf, size_t* dataLen,
                      const HandlerInterface* handler = &handlerImpl);

} // namespace ipmi
} // namespace google
