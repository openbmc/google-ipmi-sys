// Copyright 2021 Google LLC
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

#include <ipmid/api-types.hpp>
#include <ipmid/message.hpp>

#include <optional>
#include <span>

namespace google
{
namespace ipmi
{

// Handle the google-ipmi-sys IPMI OEM commands.
Resp handleSysCommand(HandlerInterface* handler, ::ipmi::Context::ptr ctx,
                      uint8_t cmd, std::span<const uint8_t> data);

} // namespace ipmi
} // namespace google
