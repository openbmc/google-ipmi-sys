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

    MOCK_CONST_METHOD0(getEthDetails, std::tuple<std::uint8_t, std::string>());
    MOCK_CONST_METHOD1(getRxPackets, std::int64_t(const std::string&));
    MOCK_CONST_METHOD1(getCpldVersion,
                       std::tuple<std::uint8_t, std::uint8_t, std::uint8_t,
                                  std::uint8_t>(unsigned int));
    MOCK_CONST_METHOD1(psuResetDelay, void(std::uint32_t));
    MOCK_CONST_METHOD0(psuResetOnShutdown, void());
    MOCK_METHOD0(getFlashSize, uint32_t());
    MOCK_METHOD2(getEntityName, std::string(std::uint8_t, std::uint8_t));
    MOCK_METHOD0(getMachineName, std::string());
    MOCK_METHOD0(buildI2cPcieMapping, void());
    MOCK_CONST_METHOD0(getI2cPcieMappingSize, size_t());
    MOCK_CONST_METHOD1(getI2cEntry,
                       std::tuple<std::uint32_t, std::string>(unsigned int));
    MOCK_CONST_METHOD1(hostS5PowerOffDelay, void(std::uint32_t));
};

} // namespace ipmi
} // namespace google
