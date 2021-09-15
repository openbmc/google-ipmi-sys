#include "commands.hpp"
#include "entity_name.hpp"
#include "handler_mock.hpp"
#include "helper.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#define MAX_IPMI_BUFFER 64

using ::testing::Return;

namespace google
{
namespace ipmi
{

TEST(EntityNameCommandTest, InvalidCommandLength)
{
    // GetEntityNameRequest is three bytes, let's send 2.
    std::vector<std::uint8_t> request = {0x01};
    HandlerMock hMock;

    EXPECT_EQ(::ipmi::responseReqDataLenInvalid(),
              getEntityName(request, &hMock));
}

TEST(EntityNameCommandTest, ValidRequest)
{
    std::uint8_t entityId = 3;
    std::uint8_t entityInstance = 5;
    std::vector<std::uint8_t> request = {entityId, entityInstance};
    std::string entityName = "asdf";

    HandlerMock hMock;
    EXPECT_CALL(hMock, getEntityName(entityId, entityInstance))
        .WillOnce(Return(entityName));

    auto reply = getEntityName(request, &hMock);
    auto result = ValidateReply(reply);
    auto& data = result.second;

    EXPECT_EQ(sizeof(GetEntityNameReply) + entityName.size(), data.size());
    EXPECT_EQ(SysOEMCommands::SysEntityName, result.first);
    EXPECT_EQ(entityName.length(), data[0]);
    EXPECT_EQ(entityName.data(),
              std::string(data.begin() + sizeof(struct GetEntityNameReply),
                          data.end()));
}

} // namespace ipmi
} // namespace google
