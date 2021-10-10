#pragma once

#include "handler.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>

#include <gmock/gmock.h>

namespace google
{
namespace ipmi
{

class HandlerMock : public HandlerInterface
{

  public:
    ~HandlerMock() = default;

    MOCK_METHOD((std::tuple<std::uint8_t, std::string>), getEthDetails,
                (std::string), (const, override));
    MOCK_METHOD(std::int64_t, getRxPackets, (const std::string&),
                (const, override));
    MOCK_METHOD(
        (std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>),
        getCpldVersion, (unsigned int), (const, override));

    MOCK_METHOD(void, psuResetDelay, (std::uint32_t), (const, override));
    MOCK_METHOD(void, psuResetOnShutdown, (), (const, override));
    MOCK_METHOD(std::uint32_t, getFlashSize, (), (override));
    MOCK_METHOD(std::string, getEntityName, (std::uint8_t, std::uint8_t), (override));
    MOCK_METHOD(std::string, getMachineName, (), (override));
    MOCK_METHOD(void, buildI2cPcieMapping, (), (override));
    MOCK_METHOD(size_t, getI2cPcieMappingSize, (), (const, override));
    MOCK_METHOD((std::tuple<std::uint32_t, std::string>), getI2cEntry, (unsigned int), (const, override));
    MOCK_METHOD(void, hostPowerOffDelay, (std::uint32_t), (const, override));
};

} // namespace ipmi
} // namespace google
