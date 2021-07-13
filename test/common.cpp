#include <cstdint>
#include <string>

namespace ipmi
{
std::uint8_t getChannelByName(const std::string& chName)
{
    return chName.size() + 10;
}
} // namespace ipmi
