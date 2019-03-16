#include "util.hpp"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <regex>
#include <string>
#include <tuple>
#include <vector>
#include <xyz/openbmc_project/Common/error.hpp>

namespace google
{
namespace ipmi
{
namespace fs = std::filesystem;
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

std::vector<std::tuple<std::uint32_t, std::string>> buildPcieMap()
{
    std::vector<std::tuple<std::uint32_t, std::string>> pcie_i2c_map;

    // Build a vector with i2c bus to pcie slot mapping.
    // Iterate through all the devices under "/sys/bus/i2c/devices".
    for (const auto& i2c_dev : fs::directory_iterator("/sys/bus/i2c/devices"))
    {
        std::string i2c_dev_path = i2c_dev.path();
        std::smatch i2c_dev_string_number;
        std::regex e("(i2c-)(\\d+)");

        // Check if the device has "i2c-" in its path.
        if (std::regex_search(i2c_dev_path, i2c_dev_string_number, e))
        {
            // Check if the i2c device has "pcie-slot" file under "of-node" dir.
            std::string pcie_slot_path = i2c_dev_path + "/of_node/pcie-slot";
            std::string pcie_slot;

            // Read the "pcie-slot" name from the "pcie-slot" file.
            pcie_slot = readPropertyFile(pcie_slot_path);
            if (pcie_slot.empty())
            {
                continue;
            }
            std::string pcie_slot_name;
            std::string pcie_slot_full_path;

            // Append the "pcie-slot" name to dts base.
            pcie_slot_full_path.append("/proc/device-tree");
            pcie_slot_full_path.append(pcie_slot);

            // Read the "label" which contains the pcie slot name.
            pcie_slot_full_path.append("/label");
            pcie_slot_name = readPropertyFile(pcie_slot_full_path);

            if (pcie_slot_name.empty())
            {
                continue;
            }

            // Get the i2c bus number from the i2c device path.
            std::uint32_t i2c_bus_number =
                i2c_dev_string_number[2].matched
                    ? std::stoi(i2c_dev_string_number[2])
                    : 0;
            // Store the i2c bus number and the pcie slot name in the vector.
            pcie_i2c_map.push_back(
                std::make_tuple(i2c_bus_number, pcie_slot_name));
        }
    }

    return pcie_i2c_map;
}

} // namespace ipmi
} // namespace google
