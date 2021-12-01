/*
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstdint>
#include <ipmid/api-types.hpp>
#include <ipmid/message.hpp>
#include <ipmid/message/types.hpp>
#include <string>
#include <unordered_map>

/* @brief: Implement the Get Device ID IPMI command for gBMC
 *  @param[in] ctx - shared_ptr to an IPMI context struct
 *
 *  @returns IPMI completion code plus response data
 *   - Device ID (manufacturer defined)
 *   - Device revision[4 bits]; reserved[3 bits]; SDR support[1 bit]
 *   - FW revision major[7 bits] (binary encoded); available[1 bit]
 *   - FW Revision minor (BCD encoded)
 *   - IPMI version (0x02 for IPMI 2.0)
 *   - device support (bitfield of supported options)
 *   - MFG IANA ID (3 bytes)
 *   - product ID (2 bytes)
 *   - AUX info (4 bytes)
 */
::ipmi::RspType<uint8_t,  // Device ID
                uint8_t,  // Device Revision
                uint8_t,  // Firmware Revision Major
                uint8_t,  // Firmware Revision minor
                uint8_t,  // IPMI version
                uint8_t,  // Additional device support
                uint24_t, // MFG ID
                uint16_t, // Product ID
                uint32_t  // AUX info
                >
    ipmiAppGetGBMCDeviceId(::ipmi::Context::ptr ctx);

class VersionReader
{
  public:
    virtual ~VersionReader()
    {
    }
    virtual bool ReadVersion(int* major, int* minor, int* point,
                             int* subpoint) = 0;
    virtual std::string ReadVersion() = 0;
    virtual std::string ReadDistro() = 0;
};

// Abstracts access to the release file as key-value pairs, to allow mocking.
class ReleaseReader
{
  public:
    virtual ~ReleaseReader()
    {
    }
    virtual std::unordered_map<std::string, std::string> ReadReleaseFile() = 0;
};

class GoogleVersionReader : public VersionReader
{
  public:
    explicit GoogleVersionReader(ReleaseReader* release_reader);
    bool ReadVersion(int* major, int* minor, int* point,
                     int* subpoint) override;
    std::string ReadVersion() override;
    std::string ReadDistro() override;

  private:
    const std::unordered_map<std::string, std::string> os_release_items_;

    std::string GetOsReleaseValue(const std::string& key) const;
};

// Reader capable of parsing the format used in /etc/os-release.
class OsReleaseReader : public ReleaseReader
{
  public:
    // If no os_release_path is provided, default to the system file.
    explicit OsReleaseReader(
        const std::string& os_release_path = kOsReleaseDefaultPath);
    std::unordered_map<std::string, std::string> ReadReleaseFile() override;

    static std::string FindReleasePath(const std::string& override_path);

  private:
    static const char* const kOsReleaseDefaultPath;

    const std::string os_release_path;
};
