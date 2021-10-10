#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

#include <ipmid/api-types.hpp>
#include <vector>

namespace google
{
namespace ipmi
{

struct PcieBifuricationReply
{
    uint8_t bifurcationLength;
} __attribute__((packed));

Resp pcieBifurcation(const std::vector<std::uint8_t>& data,
                      HandlerInterface* handler);

} // namespace ipmi
} // namespace google
