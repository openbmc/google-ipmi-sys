#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace google
{
namespace ipmi
{

/**
 * Parse a file and return the json object.
 *
 * @param[in] file - the path to the file to parse.
 * @return the json object if valid.
 * @throw elog<InternalFailure> on failures.
 */
nlohmann::json parseConfig(const std::string& file);

/**
 * Read a dts property file and return the contents.
 *
 * @param[in] file - the path to the file to parse.
 * @return the property value or an empty string on failure.
 */
std::string readPropertyFile(const std::string& fileName);

/**
 * Build a map of the i2c bus numbers to their PCIe slot names.
 *
 * @return list of pairs of i2c bus with their corresponding slot names.
 */
std::vector<std::tuple<std::uint32_t, std::string>> buildPcieMap();

} // namespace ipmi
} // namespace google
