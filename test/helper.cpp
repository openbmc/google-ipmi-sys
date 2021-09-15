#include "helper.hpp"

#include <ipmid/api-types.hpp>
#include <optional>
#include <span>
#include <utility>

namespace google
{
namespace ipmi
{

std::pair<std::uint8_t, std::vector<std::uint8_t>>
    ValidateReply(::ipmi::RspType<std::uint8_t, std::vector<uint8_t>> reply,
                  bool hasData)
{
    // Reply is in the form of
    // std::tuple<ipmi::Cc, std::optional<std::tuple<RetTypes...>>>
    EXPECT_EQ(::ipmi::ccSuccess, std::get<0>(reply));

    auto actualReply = std::get<1>(reply);
    EXPECT_TRUE(actualReply.has_value());

    auto subcommand = std::get<0>(*actualReply);
    auto data = std::get<1>(*actualReply);
    EXPECT_EQ(hasData, !data.empty());

    return std::make_pair(subcommand, hasData ? data : std::vector<uint8_t>{});
}

} // namespace ipmi
} // namespace google
