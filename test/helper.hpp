#pragma once

#include "handler_mock.hpp"

#include <ipmid/api-types.hpp>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

std::pair<std::uint8_t, std::vector<std::uint8_t>> ValidateReply(
    ::ipmi::RspType<std::uint8_t, std::optional<std::vector<uint8_t>>> reply,
    bool hasData = true);

} // namespace ipmi
} // namespace google
