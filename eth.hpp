#pragma once

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

// Handle the eth query command.
// Sys can query the if_name and IPMI channel of the BMC's NCSI ethernet
// device.
ipmi_ret_t GetEthDevice(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen);

} // namespace ipmi
} // namespace google
