#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

// Set a time-delayed PSU hard reset.
ipmi_ret_t PsuHardReset(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen,
                        const HandlerInterface* handler = &handlerImpl);

} // namespace ipmi
} // namespace google
