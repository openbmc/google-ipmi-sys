#pragma once

#include "handler.hpp"

#include <gmock/gmock.h>

namespace google
{
namespace ipmi
{

class HandlerMock : public HandlerInterface
{

  public:
    ~HandlerMock() = default;

    MOCK_CONST_METHOD0(getEthDetails, std::tuple<std::uint8_t, std::string>());
    MOCK_CONST_METHOD1(getRxPackets, std::int64_t(const std::string&));
    MOCK_CONST_METHOD1(getCpldVersion,
                       std::tuple<std::uint8_t, std::uint8_t, std::uint8_t,
                                  std::uint8_t>(unsigned int));
    MOCK_CONST_METHOD1(psuResetDelay, void(std::uint32_t));
};

} // namespace ipmi
} // namespace google
