#pragma once

#include "handler.hpp"

#include <ipmid/api.h>

namespace google
{
namespace ipmi
{

struct CableReply
{
    uint8_t subcommand;
    uint8_t value;
} __attribute__((packed));

//
// Handle the cablecheck.  Sys must supply which ethernet device they're
// interested in.
ipmi_ret_t cableCheck(const uint8_t* reqBuf, uint8_t* replyBuf, size_t* dataLen,
                      const HandlerInterface* handler = &handlerImpl);

} // namespace ipmi
} // namespace google
