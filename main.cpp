// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "handler_impl.hpp"
#include "ipmi.hpp"

#include <ipmid/api.h>

#include <cstdint>
#include <cstdio>
#include <functional>
#include <ipmid/api-types.hpp>
#include <ipmid/handler.hpp>
#include <ipmid/iana.hpp>
#include <span>

namespace oem
{
namespace google
{
constexpr int sysCmd = 50;
} // namespace google
} // namespace oem

namespace google
{
namespace ipmi
{

void setupGoogleOemSysCommands() __attribute__((constructor));

void setupGoogleOemSysCommands()
{
    static Handler handlerImpl;

    std::fprintf(stderr,
                 "Registering OEM:[%#08X], Cmd:[%#04X] for Sys Commands\n",
                 oem::googOemNumber, oem::google::sysCmd);

    ::ipmi::registerOemHandler(::ipmi::prioOemBase, oem::googOemNumber,
                               oem::google::sysCmd, ::ipmi::Privilege::User,
                               [](::ipmi::Context::ptr ctx, uint8_t cmd,
                                  std::span<const uint8_t> data) {
                                   return handleSysCommand(&handlerImpl, ctx,
                                                           cmd, data);
                               });
}

} // namespace ipmi
} // namespace google
