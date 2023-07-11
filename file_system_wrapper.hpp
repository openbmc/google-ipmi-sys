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

#include <filesystem>
#include <system_error>

namespace google
{
namespace ipmi
{
namespace fs = std::filesystem;

class FileSystemInterface
{
  public:
    virtual ~FileSystemInterface() = default;

    /**
     * Checks if the given file status or path corresponds to an existing file
     * or directory
     *
     * @param[in] path - path to examine.
     * @param[in] ec - out-parameter for error reporting in the non-throwing
     * overload
     * @return the name of the device.
     */
    virtual bool exists(const fs::path& path, std::error_code& ec) const = 0;

    /**
     * Moves or renames the filesystem object identified by old_p to new_p
     *
     * @param[in] old_p - path to move or rename.
     * @param[in] new_p - target path for the move/rename operation.
     * @param[in] ec - out-parameter for error reporting in the non-throwing
     * overload
     * @return the name of the device.
     */
    virtual void rename(const fs::path& old_p, const fs::path& new_p,
                        std::error_code& ec) const = 0;

    /**
     * Create an empty file at path specified
     *
     * @param[in] path - path to create the empty file.
     * @return the name of the device.
     */
    virtual void create(const char* path) const = 0;
};

} // namespace ipmi
} // namespace google
