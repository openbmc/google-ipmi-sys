#pragma once

#include <host-ipmid/ipmid-api.h>

namespace google
{
namespace ipmi
{

// Given a cpld identifier, return a version if available.
ipmi_ret_t CpldVersion(const uint8_t* reqBuf, uint8_t* replyBuf,
                       size_t* dataLen);

} // namespace ipmi
} // namespace google
