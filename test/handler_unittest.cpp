// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "bifurcation_mock.hpp"
#include "errors.hpp"
#include "handler.hpp"
#include "handler_impl.hpp"

#include <fmt/format.h>

#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

using testing::_;
using testing::Return;

TEST(HandlerTest, EthCheckValidHappy)
{
    Handler h;
    std::tuple<std::uint8_t, std::string> result = h.getEthDetails("et");
    EXPECT_EQ(12, std::get<0>(result));
    EXPECT_STREQ("et", std::get<1>(result).c_str());
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

struct I2CTree
{
    std::string_view name;
    std::optional<std::vector<uint8_t>> bifurcation;
    std::optional<std::string_view> mux;
    std::vector<std::string_view> channels;
};

void createI2CTree(std::string_view path, std::vector<I2CTree> tree)
{
    std::filesystem::create_directory(path);

    for (const auto& device : tree)
    {
        std::filesystem::create_directory(
            fmt::format("{}/{}", path, device.name));
        if (device.mux)
        {
            auto muxPath =
                fmt::format("{}/{}/{}", path, device.name, *device.mux);
            std::filesystem::create_directory(muxPath);

            for (size_t i = 0; i < device.channels.size(); ++i)
            {
                std::filesystem::create_directory(
                    fmt::format("{}/channel-{}", muxPath, i));
                std::filesystem::create_directory(
                    fmt::format("{}/channel-{}/i2c-dev", muxPath, i));
                std::filesystem::create_directory(
                    fmt::format("{}/channel-{}/i2c-dev/{}", muxPath, i,
                                device.channels[i]));
            }
        }
    }
}

TEST(HandlerTest, PcieBifurcation)
{
    /*
      i2c-10 (16 lanes) PE0
      `-- 10-0077 (4-channel I2C MUX at 0x77)
          |-- i2c-30 (channel-0) (4 lanes)
          |-- i2c-40 (channel-1) (5 lanes)
          |-- i2c-50 (channel-2) (7 lanes)
          `-- i2c-11 (channel-3) (invalid bus)
      i2c-30 (4 lanes)
      `-- 30-0070 (2-channel I2C MUX at 0x70)
          |-- i2c-31 (channel-0) (2 lanes)
          |-- i2c-32 (channel-1) (2 lanes)
      i2c-40 (5 lanes)
      `-- 40-0070 (2-channel I2C MUX at 0x70)
          |-- i2c-41 (channel-0) (4 lanes)
          |-- i2c-42 (channel-1) (1 lanes)
      i2c-50 (7 lanes)
      `-- 0-0070 (4-channel I2C MUX at 0x70)
          |-- i2c-51 (channel-0) (3 lanes)
          |-- i52c-52 (channel-1) (4 lanes)
      i2c-51 (3 lanes)
      `-- 51-0070 (4-channel I2C MUX at 0x70)
          |-- i2c-53 (channel-0) (1 lanes)
          |-- i2c-54 (channel-1) (1 lanes)
          |-- i2c-55 (channel-2) (1 lanes)

      i2c-20 (16 lanes) PE1
      `-- 20-0077 (4-channel I2C MUX at 0x77)
          |-- i2c-60 (channel-0) (8 lanes)
          |-- i2c-70 (channel-1) (8 lanes)
          |-- i2c-21 (channel-2) (invalid bus)
          `-- i2c-22 (channel-3) (invalid bus)
      i2c-70 (8 lanes)
      `-- 70-0070 (2-channel I2C MUX at 0x70)
          |-- i2c-71 (channel-0) (4 lanes)
          |-- i2c-72 (channel-1) (4 lanes)
      i2c-71 (4 lanes)
      `-- 71-0070 (2-channel I2C MUX at 0x70)
          |-- i2c-73 (channel-0) (2 lanes)
          |-- i2c-74 (channel-1) (2 lanes)
    */

    std::vector<I2CTree> i2cTree = {
        {"i2c-10",
         std::vector<uint8_t>{4, 5, 7},
         "10-0077",
         {"i2c-30", "i2c-40", "i2c-50", "i2c-11"}},
        {"i2c-30", std::vector<uint8_t>{2, 2}, "30-0070", {"i2c-31", "i2c-32"}},
        {"i2c-40", std::vector<uint8_t>{4, 1}, "40-0070", {"i2c-41", "i2c-42"}},
        {"i2c-50", std::vector<uint8_t>{3, 4}, "50-0070", {"i2c-51", "i2c-52"}},
        {"i2c-51",
         std::vector<uint8_t>{1, 1, 1},
         "51-0070",
         {"i2c-53", "i2c-54", "i2c-55"}},
        {"i2c-20",
         std::vector<uint8_t>{8, 8},
         "20-0077",
         {"i2c-60", "i2c-70", "i2c-21", "i2c-22"}},
        {"i2c-70", std::vector<uint8_t>{4, 4}, "70-0070", {"i2c-71", "i2c-72"}},
        {"i2c-71", std::vector<uint8_t>{2, 2}, "71-0070", {"i2c-73", "i2c-74"}},
        // Empty devices
        {"i2c-11", std::nullopt, std::nullopt, {}},
        {"i2c-21", std::nullopt, std::nullopt, {}},
        {"i2c-22", std::nullopt, std::nullopt, {}},
        {"i2c-31", std::nullopt, std::nullopt, {}},
        {"i2c-32", std::nullopt, std::nullopt, {}},
        {"i2c-41", std::nullopt, std::nullopt, {}},
        {"i2c-42", std::nullopt, std::nullopt, {}},
        {"i2c-52", std::nullopt, std::nullopt, {}},
        {"i2c-53", std::nullopt, std::nullopt, {}},
        {"i2c-54", std::nullopt, std::nullopt, {}},
        {"i2c-55", std::nullopt, std::nullopt, {}},
        {"i2c-60", std::nullopt, std::nullopt, {}},
        {"i2c-72", std::nullopt, std::nullopt, {}},
        {"i2c-73", std::nullopt, std::nullopt, {}},
        {"i2c-74", std::nullopt, std::nullopt, {}},
    };

    BifurcationMock bifurcationHelper;

    for (const auto& tree : i2cTree)
    {
        uint8_t bus = 0;
        auto result = std::from_chars(tree.name.data() + 4,
                                      tree.name.data() + tree.name.size(), bus);
        if (result.ec == std::errc::invalid_argument)
        {
            continue;
        }

        EXPECT_CALL(bifurcationHelper, getBifurcation(bus))
            .WillOnce(Return(tree.bifurcation));
    }

    const std::string& testI2CPath = "./test/i2c/";
    createI2CTree(testI2CPath, i2cTree);

    const char* testFilename = "test.json";
    std::string contents = R"(
        {
            "add_in_card": [
                {"instance": 10, "name": "/PE0"},
                {"instance": 20, "name": "/PE1"}
            ]
        }
    )";
    std::ofstream outputJson(testFilename);
    outputJson << contents;
    outputJson.flush();
    outputJson.close();

    Handler h(testFilename);
    h.setBifurcationHelper(&bifurcationHelper);

    // 16
    // 8-8
    // 8-4-4
    // 8-2-2-4
    std::vector<uint8_t> expectedOutput = {8, 2, 2, 4};
    auto bifurcation = h.pcieBifurcation(1, testI2CPath);
    ASSERT_EQ(bifurcation.size(), expectedOutput.size());
    for (size_t i = 0; i < bifurcation.size(); ++i)
    {
        EXPECT_EQ(bifurcation[i], expectedOutput[i]);
    }

    // 16
    // 4-5-7
    // 2-2-5-7
    // 2-2-4-1-3-4
    // 2-2-4-1-1-1-1-4
    expectedOutput = {2, 2, 4, 1, 1, 1, 1, 4};
    bifurcation = h.pcieBifurcation(0, testI2CPath);

    ASSERT_EQ(bifurcation.size(), expectedOutput.size());
    for (size_t i = 0; i < bifurcation.size(); ++i)
    {
        EXPECT_EQ(bifurcation[i], expectedOutput[i]);
    }

    bifurcation = h.pcieBifurcation(3, testI2CPath);
    ASSERT_EQ(bifurcation.size(), 0);
    std::remove(testFilename);
    std::filesystem::remove_all(testI2CPath);
}

// TODO: Add checks for other functions of handler.

} // namespace ipmi
} // namespace google
