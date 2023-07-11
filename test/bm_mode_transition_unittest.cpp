// Copyright 2023 Google LLC
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
#include "bm_config.h"

#include "bmc_mode_enum.hpp"
#include "errors.hpp"
#include "file_system_mock.hpp"
#include "handler.hpp"
#include "handler_impl.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>

#include <gtest/gtest.h>

namespace google
{
namespace ipmi
{
using ::testing::_;
using ::testing::Eq;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::StrEq;

namespace fs = std::filesystem;

class MockFsHandler : public Handler
{
  public:
    MockFsHandler(std::unique_ptr<FileSystemMock> mock,
                  const std::string& config = "") :
        Handler(config)
    {
        fsPtr_ = move(mock);
    }

  protected:
    const std::unique_ptr<FileSystemInterface>& getFs() const override
    {
        return fsPtr_;
    }

  private:
    std::unique_ptr<FileSystemInterface> fsPtr_;
};

TEST(BmcModeTransitionTest, DriveCleaningDoneAckFlag)
{
    auto fsMockPtr = std::make_unique<FileSystemMock>();
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .WillOnce(Return(true));

    MockFsHandler h(move(fsMockPtr));
    EXPECT_EQ(h.getBmcMode(), static_cast<uint8_t>(BmcMode::BM_MODE));
}

TEST(BmcModeTransitionTest, DriveCleaningDoneFlag)
{
    auto fsMockPtr = std::make_unique<FileSystemMock>();
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneFlagPath), _))
        .WillOnce(Return(true));
    EXPECT_CALL(*fsMockPtr, rename(fs::path(bmDriveCleaningDoneFlagPath),
                                   fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .Times(1);

    MockFsHandler h(move(fsMockPtr));
    EXPECT_EQ(h.getBmcMode(), static_cast<uint8_t>(BmcMode::BM_MODE));
}

TEST(BmcModeTransitionTest, FirstCleaningFlag)
{
    auto fsMockPtr = std::make_unique<FileSystemMock>();
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(BM_SIGNAL_PATH), _))
        .WillOnce(Return(true));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, create(StrEq(bmDriveCleaningFlagPath))).Times(1);

    MockFsHandler h(move(fsMockPtr));
    EXPECT_EQ(h.getBmcMode(), static_cast<uint8_t>(BmcMode::BM_CLEANING_MODE));
}

TEST(BmcModeTransitionTest, NonBmMode)
{
    auto fsMockPtr = std::make_unique<FileSystemMock>();
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneAckFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(bmDriveCleaningDoneFlagPath), _))
        .WillOnce(Return(false));
    EXPECT_CALL(*fsMockPtr, exists(fs::path(BM_SIGNAL_PATH), _))
        .WillOnce(Return(false));
    MockFsHandler h(move(fsMockPtr));
    EXPECT_EQ(h.getBmcMode(), static_cast<uint8_t>(BmcMode::NON_BM_MODE));
}

} // namespace ipmi
} // namespace google
