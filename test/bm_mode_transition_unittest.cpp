// Copyright 2022 Google LLC
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

#include "bmc_mode_enum.hpp"
#include "errors.hpp"
#include "file_system_mock.hpp"
#include "handler.hpp"
#include "handler_impl.hpp"

#include <charconv>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <tuple>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{

using testing::_;
using testing::Return;

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::ContainerEq;
using ::testing::DoAll;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsNull;
using ::testing::MatcherCast;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::SafeMatcherCast;
using ::testing::SetArgPointee;
using ::testing::StrEq;
using ::testing::StrictMock;
using ::testing::StrNe;
using ::testing::WithArg;

namespace fs = std::filesystem;

static constexpr auto bmDriveCleaningFlagPath = "/run/bm-drive-cleaning.flag";
static constexpr auto bmDriveCleaningDoneFlagPath =
    "/run/bm-drive-cleaning-done.flag";
static constexpr auto bmDriveCleaningDoneAckFlagPath =
    "/run/bm-drive-cleaning-done-ack.flag";
static constexpr auto BM_SIGNAL_PATH = "/run/bm-ready.flag";

class MockFsHandler : public Handler
{
  public:
    MockFsHandler(std::shared_ptr<FileSystemMock> mock,
                  const std::string& config = "") :
        Handler(config)
    {
        fsPtr = mock;
    }
};

TEST(BmcModeTransitionTest, DriveCleaningDoneAckFlag)
{
    auto fsMockPtr = std::make_shared<FileSystemMock>();
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .WillOnce(Return(true));

    MockFsHandler h(fsMockPtr);
    EXPECT_EQ(h.getBmcMode(), static_cast<uint8_t>(BmcMode::BM_MODE));
}

TEST(BmcModeTransitionTest, DriveCleaningDoneFlag)
{
    auto fsMockPtr = std::make_shared<FileSystemMock>();
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneFlagPath), _))
        .WillOnce(Return(true));
    EXPECT_CALL(*fsMockPtr, rename(fs::path(bmDriveCleaningDoneFlagPath),
                                   fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .Times(1);

    MockFsHandler h(fsMockPtr);
    EXPECT_EQ(h.getBmcMode(), static_cast<uint8_t>(BmcMode::BM_MODE));
}

TEST(BmcModeTransitionTest, FirstCleaningFlag)
{
    auto fsMockPtr = std::make_shared<FileSystemMock>();
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(BM_SIGNAL_PATH), _))
        .WillOnce(Return(true));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, create(bmDriveCleaningFlagPath)).Times(1);

    MockFsHandler h(fsMockPtr);
    EXPECT_EQ(h.getBmcMode(), static_cast<uint8_t>(BmcMode::BM_CLEANING_MODE));
}

TEST(BmcModeTransitionTest, NonBmMode)
{
    auto fsMockPtr = std::make_shared<FileSystemMock>();
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(BM_SIGNAL_PATH), _))
        .WillOnce(Return(false));
    MockFsHandler h(fsMockPtr);
    EXPECT_EQ(h.getBmcMode(), static_cast<uint8_t>(BmcMode::NON_BM_MODE));
}

} // namespace ipmi
} // namespace google
