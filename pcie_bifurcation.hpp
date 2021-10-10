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

#include <span>

namespace google
{
namespace ipmi
{

struct PcieBifurcationReply
{
    uint8_t bifurcationLength;
} __attribute__((packed));

Resp pcieBifurcation(std::span<const uint8_t> data, HandlerInterface* handler,
                     bool dynamic);

} // namespace ipmi
} // namespace google
