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
};

class Handler : public HandlerInterface
{
  public:
    Handler() = default;
    ~Handler() = default;

    std::tuple<std::uint8_t, std::string> getEthDetails() const override;
};

extern Handler handlerImpl;

} // namespace ipmi
} // namespace google
