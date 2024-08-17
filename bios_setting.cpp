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
    // If the path doesn't exist, set the properties to an empty string
    if (!std::filesystem::exists(biosSettingPath.data()))
    {
        stdplus::print(stderr, "BIOS setting file could not be opened\n");
        return {};
    }

    std::ifstream ifs;
    ifs.exceptions(std::ifstream::failbit);
    try
    {
        ifs.open(biosSettingPath.data(), std::ios::binary);
        // Read the last character to see if it's '\n' - skip if it is
        ifs.seekg(-1, ifs.end);
        if (ifs.peek() != '\n')
        {
            ifs.seekg(0, ifs.end);
        }
        size_t length = ifs.tellg();
        ifs.seekg(0, ifs.beg);

        if (length > MAX_PAYLOAD_SIZE)
        {
            stdplus::print(stderr, "BIOS setting content was too big\n");
            return {};
        }

        char buffer[MAX_IPMI_BUFFER];
        ifs.seekg(0, ifs.beg);
        ifs.read(buffer, length);
        biosSettings.assign(buffer, buffer + length);
    }
    catch (std::ios_base::failure& fail)
    {
        stdplus::print(stderr, "Faild to read the BIOS setting file\n");
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
        throw IpmiException(::ipmi::ccRetBytesUnavailable);
    }

    std::vector<std::uint8_t> reply;
    reply.reserve(1 + settingsLength);
    reply.emplace_back(static_cast<uint8_t>(settingsLength));
    reply.insert(reply.end(), biosSettings.begin(), biosSettings.end());

    return ::ipmi::responseSuccess(SysOEMCommands::SysReadOemBiosSetting,
                                   reply);
}

} // namespace ipmi
} // namespace google
