#pragma once

#include <cstdint>
#include <string>
#include <tuple>

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
};

class Handler : public HandlerInterface
{
  public:
    Handler() = default;
    ~Handler() = default;

    std::tuple<std::uint8_t, std::string> getEthDetails() const override;
    std::int64_t getRxPackets(const std::string& name) const override;
    VersionTuple getCpldVersion(unsigned int id) const override;
    void psuResetDelay(std::uint32_t delay) const override;
    std::string getEntityName(std::uint8_t id, std::uint8_t instance) override;
};

extern Handler handlerImpl;

} // namespace ipmi
} // namespace google
