// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

// Handle the accel power setting command
Resp accelSetVrSettings(::ipmi::Context::ptr ctx, std::span<const uint8_t> data,
                        HandlerInterface* handler);
Resp accelGetVrSettings(::ipmi::Context::ptr ctx, std::span<const uint8_t> data,
                        HandlerInterface* handler);
} // namespace ipmi
} // namespace google
