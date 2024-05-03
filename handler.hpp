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

#include <ipmid/api-types.hpp>
#include <ipmid/message.hpp>

#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace google
{
namespace ipmi
{

using Resp = ::ipmi::RspType<std::uint8_t, std::vector<uint8_t>>;

using VersionTuple =
    std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>;

class HandlerInterface
{
  public:
    virtual ~HandlerInterface() = default;

    /**
     * Return the operation mode of BMC
     *
     * @return the BMC operation mode
     */
    virtual uint8_t getBmcMode() = 0;

    /**
     * Return ethernet details (hard-coded).
     *
     * @return tuple of ethernet details (channel, if name).
     */
    virtual std::tuple<std::uint8_t, std::string>
        getEthDetails(std::string intf) const = 0;

    /**
     * Return the value of rx_packets, given a if_name.
     *
     * @param[in] name, the interface name.
     * @return the number of packets received.
     * @throw IpmiException on failure.
     */
    virtual std::int64_t getRxPackets(const std::string& name) const = 0;

    /**
     * Return the values from a cpld version file.
     *
     * @param[in] id - the cpld id number.
     * @return the quad of numbers as a tuple (maj,min,pt,subpt)
     * @throw IpmiException on failure.
     */
    virtual VersionTuple getCpldVersion(unsigned int id) const = 0;

    /**
     * Set the PSU Reset delay.
     *
     * @param[in] delay - delay in seconds.
     * @throw IpmiException on failure.
     */
    virtual void psuResetDelay(std::uint32_t delay) const = 0;

    /**
     * Arm for PSU reset on host shutdown.
     *
     * @throw IpmiException on failure.
     */
    virtual void psuResetOnShutdown() const = 0;

    /**
     * Return the entity name.
     * On the first call to this method it'll build the list of entities.
     * @todo Consider moving the list building to construction time (and ignore
     * failures).
     *
     * @param[in] id - the entity id value
     * @param[in] instance - the entity instance
     * @return the entity's name
     * @throw IpmiException on failure.
     */
    virtual std::string getEntityName(std::uint8_t id,
                                      std::uint8_t instance) = 0;

    /**
     * Return the flash size of bmc chip.
     *
     * @return the flash size of bmc chip
     * @throw IpmiException on failure.
     */
    virtual uint32_t getFlashSize() = 0;

    /**
     * Return the name of the machine, parsed from release information.
     *
     * @return the machine name
     * @throw IpmiException on failure.
     */
    virtual std::string getMachineName() = 0;

    /**
     * Populate the i2c-pcie mapping vector.
     */
    virtual void buildI2cPcieMapping() = 0;

    /**
     * Return the size of the i2c-pcie mapping vector.
     *
     * @return the size of the vector holding the i2c-pcie mapping tuples.
     */
    virtual size_t getI2cPcieMappingSize() const = 0;

    /**
     * Return a copy of the entry in the vector.
     *
     * @param[in] entry - the index into the vector.
     * @return the tuple at that index.
     */
    virtual std::tuple<std::uint32_t, std::string>
        getI2cEntry(unsigned int entry) const = 0;

    /**
     * Set the Host Power Off delay.
     *
     * @param[in] delay - delay in seconds.
     * @throw IpmiException on failure.
     */
    virtual void hostPowerOffDelay(std::uint32_t delay) const = 0;

    /**
     * Return the number of devices from the CustomAccel service.
     *
     * @return the number of devices.
     * @throw IpmiException on failure.
     */
    virtual uint32_t accelOobDeviceCount() const = 0;

    /**
     * Return the name of a single device from the CustomAccel service.
     *
     * Valid indexes start at 0 and go up to (but don't include) the number of
     * devices. The number of devices can be queried with accelOobDeviceCount.
     *
     * @param[in] index - the index of the device, starting at 0.
     * @return the name of the device.
     * @throw IpmiException on failure.
     */
    virtual std::string accelOobDeviceName(size_t index) const = 0;

    /**
     * Read from a single CustomAccel service device.
     *
     * Valid device names can be queried with accelOobDeviceName.
     * If num_bytes < 8, all unused MSBs are padded with 0s.
     *
     * @param[in] name - the name of the device (from DeviceName).
     * @param[in] address - the address to read from.
     * @param[in] num_bytes - the size of the read, in bytes.
     * @return the data read, with 0s padding any unused MSBs.
     * @throw IpmiException on failure.
     */
    virtual uint64_t accelOobRead(std::string_view name, uint64_t address,
                                  uint8_t num_bytes) const = 0;

    /**
     * Write to a single CustomAccel service device.
     *
     * Valid device names can be queried with accelOobDeviceName.
     * If num_bytes < 8, all unused MSBs are ignored.
     *
     * @param[in] name - the name of the device (from DeviceName).
     * @param[in] address - the address to read from.
     * @param[in] num_bytes - the size of the read, in bytes.
     * @param[in] data - the data to write.
     * @throw IpmiException on failure.
     */
    virtual void accelOobWrite(std::string_view name, uint64_t address,
                               uint8_t num_bytes, uint64_t data) const = 0;

    /**
     * Parse the I2C tree to get the highest level of bifurcation in target bus.
     *
     * @param[in] index    - PCIe Slot Index
     * @return list of lanes taken by each device.
     */
    virtual std::vector<uint8_t> pcieBifurcation(uint8_t index) = 0;

    /**
     * Prepare for OS boot.
     *
     * If in bare metal mode, the BMC will disable IPMI, to protect against an
     * untrusted OS.
     */
    virtual void linuxBootDone() const = 0;

    /**
     * Update the VR settings for the given settings_id
     *
     * @param[in] chip_id    - Accel Device#
     * @param[in] settings_id  - ID of the setting to update
     * @param[in] value  - Value of the setting
     */
    virtual void accelSetVrSettings(::ipmi::Context::ptr ctx, uint8_t chip_id,
                                    uint8_t settings_id,
                                    uint16_t value) const = 0;

    /**
     * Read current VR settings value for the given settings_id
     *
     * @param[in] chip_id    - Accel Device#
     * @param[in] settings_id  - ID of the setting to read
     */
    virtual uint16_t accelGetVrSettings(::ipmi::Context::ptr ctx,
                                        uint8_t chip_id,
                                        uint8_t settings_id) const = 0;

    /**
     * Get the BM instance property from /run/<propertyType>
     *
     * @param[in] propertyType  - BM instance property type
     * @return - string of the requested BM instance property
     */
    virtual std::string getBMInstanceProperty(uint8_t propertyType) const = 0;
};

} // namespace ipmi
} // namespace google
