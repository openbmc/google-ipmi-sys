// Copyright 2023 Google LLC
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

#include "file_system_wrapper.hpp"

namespace google
{
namespace ipmi
{
namespace fs = std::filesystem;

class FileSystemWrapper : public FileSystemInterface
{
  public:
    bool exists(const fs::path& path, std::error_code& ec) const;
    void rename(const fs::path& oldPath, const fs::path& newPath,
                std::error_code& ec) const;
    void create(const char* path) const;
};

} // namespace ipmi
} // namespace google
