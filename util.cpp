#include "util.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <string>
#include <xyz/openbmc_project/Common/error.hpp>

namespace google
{
namespace ipmi
{

using namespace phosphor::logging;
using InternalFailure =
    sdbusplus::xyz::openbmc_project::Common::Error::InternalFailure;

nlohmann::json parseConfig(const std::string& file)
{
    std::ifstream jsonFile(file);
    if (!jsonFile.is_open())
    {
        log<level::ERR>("Entity association JSON file not found");
        elog<InternalFailure>();
    }

    auto data = nlohmann::json::parse(jsonFile, nullptr, false);
    if (data.is_discarded())
    {
        log<level::ERR>("Entity association JSON parser failure");
        elog<InternalFailure>();
    }

    return data;
}

} // namespace ipmi
} // namespace google
