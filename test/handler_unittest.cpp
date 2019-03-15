#include "errors.hpp"
#include "handler.hpp"

#include <string>
#include <tuple>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

TEST(HandlerTest, EthCheckValidHappy)
{
    // The code returns compiled-in information, and therefore cannot really
    // fail.
    Handler h;
    std::tuple<std::uint8_t, std::string> result = h.getEthDetails();
    EXPECT_EQ(1, std::get<0>(result));
    EXPECT_STREQ("eth0", std::get<1>(result).c_str());
}

TEST(HandlerTest, CableCheckIllegalPath)
{
    Handler h;
    EXPECT_THROW(h.getRxPackets("eth0/../../"), IpmiException);
}

// TODO: Add checks for other functions of handler.

} // namespace ipmi
} // namespace google
