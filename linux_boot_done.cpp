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

#include "linux_boot_done.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>

#include <span>
#include <vector>

namespace google
{
namespace ipmi
{

Resp linuxBootDone(std::span<const uint8_t>, HandlerInterface* handler)
{
    try
    {
        handler->linuxBootDone();
        return ::ipmi::responseSuccess(SysOEMCommands::SysLinuxBootDone,
                                       std::vector<std::uint8_t>{});
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }
}
} // namespace ipmi
} // namespace google
