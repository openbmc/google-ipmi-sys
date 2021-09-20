#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <span>

namespace google
{
namespace ipmi
{

//  Handle the Accel OOB device count command
Resp accelOobDeviceCount(std::span<const uint8_t> data,
                         HandlerInterface* handler);

Resp accelOobDeviceName(std::span<const uint8_t> data,
                        HandlerInterface* handler);

Resp accelOobRead(std::span<const uint8_t> data, HandlerInterface* handler);

Resp accelOobWrite(std::span<const uint8_t> data, HandlerInterface* handler);

} // namespace ipmi
} // namespace google
