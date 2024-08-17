// Copyright 2024 Google LLC
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

#include "bios_setting.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <ipmid/api-types.hpp>
#include <stdplus/fd/create.hpp>
#include <stdplus/fd/managed.hpp>
#include <stdplus/fd/ops.hpp>
#include <stdplus/print.hpp>

#include <filesystem>
#include <fstream>
#include <span>
#include <vector>

namespace google
{
namespace ipmi
{

std::vector<uint8_t> readBiosSettings()
{
    std::vector<uint8_t> biosSettings;
    try
    {
        stdplus::ManagedFd managedFd = stdplus::fd::open(
            biosSettingPath,
            stdplus::fd::OpenFlags(stdplus::fd::OpenAccess::ReadOnly));
        biosSettings = stdplus::fd::readAll<std::vector<uint8_t>>(managedFd);
    }
    catch (const std::exception& e)
    {
        stdplus::print(stderr, "Read unsuccessful");
        return {};
    }
    return biosSettings;
}

Resp readBiosSetting(std::span<const uint8_t>, HandlerInterface*)
{
    struct ReadBiosSettingResponse
    {
        uint8_t length;
        uint8_t buffer[MAX_PAYLOAD_SIZE];
    } __attribute__((packed));

    std::vector<uint8_t> biosSettings = readBiosSettings();
    size_t settingsLength = biosSettings.size();
    if (settingsLength == 0)
    {
        return ::ipmi::responseRetBytesUnavailable();
    }

    std::vector<std::uint8_t> reply;
    reply.reserve(1 + settingsLength);
    reply.emplace_back(static_cast<uint8_t>(settingsLength));
    reply.insert(reply.end(), biosSettings.begin(), biosSettings.end());

    return ::ipmi::responseSuccess(SysOEMCommands::SysReadBiosSetting, reply);
}

} // namespace ipmi
} // namespace google
