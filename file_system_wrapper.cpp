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

#include "file_system_wrapper_impl.hpp"

#include <fstream>
#include <system_error>

namespace google
{
namespace ipmi
{
namespace fs = std::filesystem;

bool FileSystemWrapper::exists(const fs::path& path, std::error_code& ec) const
{
    return fs::exists(path, ec);
}

void FileSystemWrapper::rename(const fs::path& oldPath, const fs::path& newPath,
                               std::error_code& ec) const
{
    fs::rename(oldPath, newPath, ec);
}

void FileSystemWrapper::create(const char* path) const
{
    std::ofstream ofs;
    ofs.open(path, std::ofstream::out);
    ofs.close();
}
} // namespace ipmi
} // namespace google
