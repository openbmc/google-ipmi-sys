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

#pragma once

#include <exception>
#include <string>

namespace google
{
namespace ipmi
{

/**
 * This can be used by the Handler object to throw an exception and suggest an
 * IPMI return code to use for the error.
 */
class IpmiException : public std::exception
{
  public:
    explicit IpmiException(int ipmicc) :
        _message("IPMI Code Received: " + std::to_string(ipmicc)),
        _ipmicc(ipmicc)
    {
    }

    virtual const char* what() const noexcept override
    {
        return _message.c_str();
    }

    int getIpmiError() const
    {
        return _ipmicc;
    }

  private:
    std::string _message;
    int _ipmicc;
};

} // namespace ipmi
} // namespace google
