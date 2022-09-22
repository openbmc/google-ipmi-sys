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

#include "flash_size.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <cstdint>
#include <cstring>
#include <ipmid/api-types.hpp>
#include <span>
#include <string>
#include <vector>

namespace google
{
namespace ipmi
{

Resp getFlashSize(std::span<const uint8_t>, HandlerInterface* handler)
{
    uint32_t flashSize;
    try
    {
        flashSize = handler->getFlashSize();
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }

    flashSize = htole32(flashSize);
    return ::ipmi::responseSuccess(
        SysOEMCommands::SysGetFlashSize,
        std::vector<std::uint8_t>((std::uint8_t*)&(flashSize),
                                  (std::uint8_t*)&(flashSize) +
                                      sizeof(std::uint32_t)));
}
} // namespace ipmi
} // namespace google
