/*
 * Copyright 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "entity_name.hpp"

#include "main.hpp"

#include <cstdint>
#include <cstring>
#include <experimental/filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>
#include <xyz/openbmc_project/Common/error.hpp>

namespace google
{
namespace ipmi
{
namespace fs = std::experimental::filesystem;

namespace
{

// TODO (jaghu) : Add a call to get getChannelMaxTransferSize.
#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

using Json = nlohmann::json;

using namespace phosphor::logging;
using InternalFailure =
    sdbusplus::xyz::openbmc_project::Common::Error::InternalFailure;

static constexpr auto configFile =
    "/usr/share/ipmi-entity-association/entity_association_map.json";

static const std::map<uint8_t, std::string> entityIdToName{
    {0x03, "cpu"},
    {0x04, "storage_device"},
    {0x0B, "add_in_card"},
    {0x20, "memory_module"}};

Json parse_config()
{
    std::ifstream jsonFile(configFile);
    if (!jsonFile.is_open())
    {
        log<level::ERR>("Entity association JSON file not found");
        elog<InternalFailure>();
    }

    auto data = Json::parse(jsonFile, nullptr, false);
    if (data.is_discarded())
    {
        log<level::ERR>("Entity association JSON parser failure");
        elog<InternalFailure>();
    }

    return data;
}

std::string read(const std::string& type, uint8_t instance, const Json& config)
{
    static const std::vector<Json> empty{};
    std::vector<Json> readings = config.value(type, empty);
    std::string name = "";
    for (const auto& j : readings)
    {
        uint8_t instanceNum = j.value("instance", 0);
        // Not the instance we're interested in
        if (instanceNum != instance)
        {
            continue;
        }

        // Found the instance we're interested in
        name = j.value("name", "");

        break;
    }
    return name;
}

} // namespace

struct GetEntityNameRequest
{
    uint8_t subcommand;
    uint8_t entity_id;
    uint8_t entity_instance;
} __attribute__((packed));

struct GetEntityNameReply
{
    uint8_t subcommand;
    uint8_t entity_name_len;
    uint8_t entity_name[0];
} __attribute__((packed));

ipmi_ret_t GetEntityName(const uint8_t* reqBuf, uint8_t* replyBuf,
                         size_t* dataLen)
{
    struct GetEntityNameRequest request;

    if ((*dataLen) < sizeof(request))
    {
        std::fprintf(stderr, "Invalid command length: %u\n",
                     static_cast<uint32_t>(*dataLen));
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    std::memcpy(&request, &reqBuf[0], sizeof(request));

    // Check if we support this Entity ID.
    auto it = entityIdToName.find(request.entity_id);
    if (it == entityIdToName.end())
    {
        log<level::ERR>("Unknown Entity ID",
                        entry("ENTITY_ID=%d", request.entity_id));
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    static Json config{};
    static bool parsed = false;

    std::string entityName;
    try
    {
        // Parse the JSON config file.
        if (!parsed)
        {
            config = parse_config();
            parsed = true;
        }

        // Find the "entity id:entity instance" mapping to entity name.
        entityName = read(it->second, request.entity_instance, config);
        if (entityName.empty())
            return IPMI_CC_INVALID_FIELD_REQUEST;
    }
    catch (InternalFailure& e)
    {
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    int length = sizeof(struct GetEntityNameReply) + entityName.length();

    // TODO (jaghu) : Add a call to get getChannelMaxTransferSize.
    if (length > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr, "Response would overflow response buffer\n");
        return IPMI_CC_INVALID;
    }

    auto reply = reinterpret_cast<struct GetEntityNameReply*>(&replyBuf[0]);
    reply->subcommand = SysEntityName;
    reply->entity_name_len = entityName.length();
    std::memcpy(reply->entity_name, entityName.c_str(), entityName.length());

    (*dataLen) = length;
    return IPMI_CC_OK;
}
} // namespace ipmi
} // namespace google
