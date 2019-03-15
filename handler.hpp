#pragma once

#include <cstdint>
#include <string>
#include <tuple>

namespace google
{
namespace ipmi
{

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
};

class Handler : public HandlerInterface
{
  public:
    Handler() = default;
    ~Handler() = default;

    std::tuple<std::uint8_t, std::string> getEthDetails() const override;
    std::int64_t getRxPackets(const std::string& name) const override;
};

extern Handler handlerImpl;

} // namespace ipmi
} // namespace google
