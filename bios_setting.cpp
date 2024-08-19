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
#include <stdplus/numeric/endian.hpp>
#include <stdplus/print.hpp>
#include <stdplus/raw.hpp>

#include <filesystem>
#include <fstream>
#include <span>
#include <vector>

namespace google
{
namespace ipmi
{

std::vector<uint8_t> readBiosSettingFromFile(const std::string& biosSettingPath)
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
        stdplus::print(stderr, "Read unsuccessful: {}\n", e.what());
        return {};
    }
    return biosSettings;
}

Resp readBiosSetting(std::span<const uint8_t>, HandlerInterface*,
                     const std::string& biosSettingPath)
{
    std::vector<uint8_t> biosSettings =
        readBiosSettingFromFile(biosSettingPath);
    size_t settingsLength = biosSettings.size();
    if (settingsLength == 0)
    {
        return ::ipmi::responseRetBytesUnavailable();
    }

    // Reply format is: Length of the payload (1 byte) + payload
    std::vector<std::uint8_t> reply;
    reply.reserve(1 + settingsLength);
    reply.emplace_back(static_cast<uint8_t>(settingsLength));
    reply.insert(reply.end(), biosSettings.begin(), biosSettings.end());

    return ::ipmi::responseSuccess(SysOEMCommands::SysReadBiosSetting, reply);
}

Resp writeBiosSetting(std::span<const uint8_t> data, HandlerInterface*,
                      const std::string& biosSettingPath)
{
    std::uint8_t payloadSize;
    try
    {
        // This subspans the data automatically
        payloadSize = stdplus::raw::extract<
            stdplus::EndianPacked<decltype(payloadSize), std::endian::little>>(
            data);
    }
    catch (const std::exception& e)
    {
        stdplus::print(stderr, "Extracting payload failed: {}\n", e.what());
        return ::ipmi::responseReqDataLenInvalid();
    }

    if (data.size() != payloadSize)
    {
        stdplus::print(stderr, "Invalid command length {} vs. payloadSize {}\n",
                       static_cast<uint32_t>(data.size()),
                       static_cast<uint32_t>(payloadSize));
        return ::ipmi::responseReqDataLenInvalid();
    }

    // Write the setting
    try
    {
        stdplus::ManagedFd managedFd = stdplus::fd::open(
            biosSettingPath,
            stdplus::fd::OpenFlags(stdplus::fd::OpenAccess::WriteOnly)
                .set(stdplus::fd::OpenFlag::Trunc)
                .set(stdplus::fd::OpenFlag::Create));
        stdplus::fd::writeExact(managedFd, data);
    }
    catch (const std::exception& e)
    {
        stdplus::print(stderr, "Write unsuccessful: {}\n", e.what());
        return ::ipmi::responseRetBytesUnavailable();
    }

    // Reply format is: Length of the payload written
    std::vector<std::uint8_t> reply;
    reply.reserve(1);
    reply.emplace_back(static_cast<uint8_t>(payloadSize));

    return ::ipmi::responseSuccess(SysOEMCommands::SysWriteBiosSetting, reply);
}

} // namespace ipmi
} // namespace google
