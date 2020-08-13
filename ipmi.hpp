#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

// Handle the google-ipmi-sys IPMI OEM commands.
ipmi_ret_t handleSysCommand(HandlerInterface* handler, ipmi_cmd_t cmd,
                            const uint8_t* reqBuf, uint8_t* replyCmdBuf,
                            size_t* dataLen);

} // namespace ipmi
} // namespace google
