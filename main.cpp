/*
 * Copyright 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "handler_impl.hpp"
#include "ipmi.hpp"

#include <ipmid/api.h>

#include <cstdint>
#include <cstdio>
#include <functional>
#include <ipmid/iana.hpp>
#include <ipmid/oemrouter.hpp>

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

Handler handlerImpl;

void setupGoogleOemSysCommands() __attribute__((constructor));

void setupGoogleOemSysCommands()
{
    oem::Router* oemRouter = oem::mutableRouter();

    std::fprintf(stderr,
                 "Registering OEM:[%#08X], Cmd:[%#04X] for Sys Commands\n",
                 oem::googOemNumber, oem::google::sysCmd);

    using namespace std::placeholders;
    oemRouter->registerHandler(
        oem::googOemNumber, oem::google::sysCmd,
        std::bind(handleSysCommand, &handlerImpl, _1, _2, _3, _4));
}

} // namespace ipmi
} // namespace google
