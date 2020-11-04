#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace google
{
namespace ipmi
{

using VersionTuple =
    std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>;

class HandlerInterface
{
  public:
    virtual ~HandlerInterface() = default;

    /**
     * Return ethernet details (hard-coded).
     *
     * @return tuple of ethernet details (channel, if name).
     */
    virtual std::tuple<std::uint8_t, std::string> getEthDetails() const = 0;

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
};

} // namespace ipmi
} // namespace google
