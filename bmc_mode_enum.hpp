// Copyright 2022 Google LLC
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

#pragma once

#include <cstdint>

namespace google
{
namespace ipmi
{
inline constexpr auto bmDriveCleaningFlagPath = "/run/bm-drive-cleaning.flag";
inline constexpr auto bmDriveCleaningDoneFlagPath =
    "/run/bm-drive-cleaning-done.flag";
inline constexpr auto bmDriveCleaningDoneAckFlagPath =
    "/run/bm-drive-cleaning-done-ack.flag";

enum class BmcMode : uint8_t
{
    NON_BM_MODE = 0,
    BM_MODE,
    BM_CLEANING_MODE
};

} // namespace ipmi
} // namespace google
