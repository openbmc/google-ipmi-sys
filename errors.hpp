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
        message("IPMI Code Received: " + std::to_string(ipmicc)),
        _ipmicc(ipmicc)
    {
    }

    virtual const char* what() const noexcept override
    {
        return message.c_str();
    }

    int getIpmiError() const
    {
        return _ipmicc;
    }

  private:
    std::string message;
    int _ipmicc;
};

} // namespace ipmi
} // namespace google
