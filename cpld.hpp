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
#include <span>

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
Resp cpldVersion(std::span<const uint8_t> data,
                 const HandlerInterface* handler);

} // namespace ipmi
} // namespace google
