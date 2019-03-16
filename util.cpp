#include "util.hpp"

#include <cstdio>
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

std::string readPropertyFile(const std::string& fileName)
{
    std::ifstream ifs(fileName);
    std::string contents;

    if (!ifs.is_open())
    {
        std::fprintf(stderr, "Unable to open file %s.\n", fileName.c_str());
    }
    else
    {
        if (ifs >> contents)
        {
            // If the last character is a null terminator; remove it.
            if (!contents.empty())
            {
                char const& back = contents.back();
                if (back == '\0')
                    contents.pop_back();
            }

            return contents;
        }
        else
        {
            std::fprintf(stderr, "Unable to read file %s.\n", fileName.c_str());
        }
    }

    return "";
}

} // namespace ipmi
} // namespace google
