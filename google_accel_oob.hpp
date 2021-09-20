#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

//  Handle the Accel OOB device count command
ipmi_ret_t accelOobDeviceCount(const uint8_t* reqBuf, uint8_t* replyBuf,
                               size_t* dataLen, HandlerInterface* handler);

ipmi_ret_t accelOobDeviceName(const uint8_t* reqBuf, uint8_t* replyBuf,
                              size_t* dataLen, HandlerInterface* handler);

ipmi_ret_t accelOobRead(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, HandlerInterface* handler);

ipmi_ret_t accelOobWrite(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen, HandlerInterface* handler);

} // namespace ipmi
} // namespace google
