#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

ipmi_ret_t getFlashSize(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen, HandlerInterface* handler);

} // namespace ipmi
} // namespace google
