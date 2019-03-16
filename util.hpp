#pragma once

#include <nlohmann/json.hpp>
#include <string>

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

} // namespace ipmi
} // namespace google
