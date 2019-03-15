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
};

} // namespace ipmi
} // namespace google
