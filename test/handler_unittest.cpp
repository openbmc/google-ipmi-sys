#include "errors.hpp"
#include "handler.hpp"
#include "handler_impl.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
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

TEST(HandlerTest, readNameFromConfigInstanceVariety)
{
    // Make sure it handles the failures and successes as we expect.
    struct testCase
    {
        std::string type;
        std::uint8_t instance;
        std::string expectedName;
    };

    std::vector<testCase> tests = {
        {"cpu", 5, ""},
        {"cpu", 3, "CPU2"},
    };

    auto j2 = R"(
      {
        "cpu": [
          {"instance": 1, "name": "CPU0"},
          {"instance": 2, "name": "CPU1"},
          {"instance": 3, "name": "CPU2"},
          {"instance": 4, "name": "CPU3"}
        ]
      }
    )"_json;

    for (const auto& test : tests)
    {
        EXPECT_STREQ(test.expectedName.c_str(),
                     readNameFromConfig(test.type, test.instance, j2).c_str());
    }
}

// TODO: If we can test with phosphor-logging in the future, there are more
// failure cases.

TEST(HandlerTest, getEntityNameWithNameNotFoundExcepts)
{
    const char* testFilename = "test.json";
    std::string contents = R"({"cpu": [{"instance": 1, "name": "CPU0"}]})";
    std::ofstream outputJson(testFilename);
    outputJson << contents;
    outputJson.flush();
    outputJson.close();

    Handler h(testFilename);
    EXPECT_THROW(h.getEntityName(0x03, 2), IpmiException);
    (void)std::remove(testFilename);
}

TEST(HandlerTest, getEntityNameWithNameFoundReturnsIt)
{
    const char* testFilename = "test.json";
    std::string contents = R"({"cpu": [{"instance": 1, "name": "CPU0"}]})";
    std::ofstream outputJson(testFilename);
    outputJson << contents;
    outputJson.flush();
    outputJson.close();

    Handler h(testFilename);
    EXPECT_STREQ("CPU0", h.getEntityName(0x03, 1).c_str());
    (void)std::remove(testFilename);
}

// TODO: Add checks for other functions of handler.

} // namespace ipmi
} // namespace google
