// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/file_system_provider/fileapi/file_stream_writer.h"

#include <string>
#include <vector>

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/run_loop.h"
#include "chrome/browser/chromeos/file_system_provider/fake_provided_file_system.h"
#include "chrome/browser/chromeos/file_system_provider/service.h"
#include "chrome/browser/chromeos/file_system_provider/service_factory.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "content/public/test/test_file_system_context.h"
#include "extensions/browser/extension_registry.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "webkit/browser/fileapi/async_file_util.h"
#include "webkit/browser/fileapi/external_mount_points.h"
#include "webkit/browser/fileapi/file_system_url.h"

namespace chromeos {
namespace file_system_provider {
namespace {

const char kExtensionId[] = "mbflcebpggnecokmikipoihdbecnjfoj";
const char kFileSystemId[] = "testing-file-system";
const char kTextToWrite[] = "This is a test of FileStreamWriter.";

// Pushes a value to the passed log vector.
void LogValue(std::vector<int>* log, int value) {
  log->push_back(value);
}

// Creates a cracked FileSystemURL for tests.
fileapi::FileSystemURL CreateFileSystemURL(const std::string& mount_point_name,
                                           const base::FilePath& file_path) {
  const std::string origin = std::string("chrome-extension://") + kExtensionId;
  const fileapi::ExternalMountPoints* const mount_points =
      fileapi::ExternalMountPoints::GetSystemInstance();
  return mount_points->CreateCrackedFileSystemURL(
      GURL(origin),
      fileapi::kFileSystemTypeExternal,
      base::FilePath::FromUTF8Unsafe(mount_point_name).Append(file_path));
}

// Creates a Service instance. Used to be able to destroy the service in
// TearDown().
KeyedService* CreateService(content::BrowserContext* context) {
  return new Service(Profile::FromBrowserContext(context),
                     extensions::ExtensionRegistry::Get(context));
}

}  // namespace

class FileSystemProviderFileStreamWriter : public testing::Test {
 protected:
  FileSystemProviderFileStreamWriter() {}
  virtual ~FileSystemProviderFileStreamWriter() {}

  virtual void SetUp() OVERRIDE {
    ASSERT_TRUE(data_dir_.CreateUniqueTempDir());
    profile_manager_.reset(
        new TestingProfileManager(TestingBrowserProcess::GetGlobal()));
    ASSERT_TRUE(profile_manager_->SetUp());
    profile_ = profile_manager_->CreateTestingProfile("testing-profile");

    ServiceFactory::GetInstance()->SetTestingFactory(profile_, &CreateService);
    Service* service = Service::Get(profile_);  // Owned by its factory.
    service->SetFileSystemFactoryForTesting(
        base::Bind(&FakeProvidedFileSystem::Create));

    const bool result = service->MountFileSystem(kExtensionId,
                                                 kFileSystemId,
                                                 "Testing File System",
                                                 false /* writable */);
    ASSERT_TRUE(result);
    provided_file_system_ = static_cast<FakeProvidedFileSystem*>(
        service->GetProvidedFileSystem(kExtensionId, kFileSystemId));
    ASSERT_TRUE(provided_file_system_);
    const ProvidedFileSystemInfo& file_system_info =
        provided_file_system_->GetFileSystemInfo();
    const std::string mount_point_name =
        file_system_info.mount_path().BaseName().AsUTF8Unsafe();

    file_url_ = CreateFileSystemURL(
        mount_point_name, base::FilePath::FromUTF8Unsafe(kFakeFilePath + 1));
    ASSERT_TRUE(file_url_.is_valid());
    wrong_file_url_ = CreateFileSystemURL(
        mount_point_name, base::FilePath::FromUTF8Unsafe("im-not-here.txt"));
    ASSERT_TRUE(wrong_file_url_.is_valid());
  }

  virtual void TearDown() OVERRIDE {
    // Setting the testing factory to NULL will destroy the created service
    // associated with the testing profile.
    ServiceFactory::GetInstance()->SetTestingFactory(profile_, NULL);
  }

  content::TestBrowserThreadBundle thread_bundle_;
  base::ScopedTempDir data_dir_;
  scoped_ptr<TestingProfileManager> profile_manager_;
  TestingProfile* profile_;  // Owned by TestingProfileManager.
  FakeProvidedFileSystem* provided_file_system_;  // Owned by Service.
  fileapi::FileSystemURL file_url_;
  fileapi::FileSystemURL wrong_file_url_;
};

TEST_F(FileSystemProviderFileStreamWriter, Write) {
  std::vector<int> write_log;

  const int64 initial_offset = 0;
  FileStreamWriter writer(file_url_, initial_offset);
  scoped_refptr<net::IOBuffer> io_buffer(new net::StringIOBuffer(kTextToWrite));

  {
    const int result = writer.Write(io_buffer.get(),
                                    sizeof(kTextToWrite) - 1,
                                    base::Bind(&LogValue, &write_log));
    EXPECT_EQ(net::ERR_IO_PENDING, result);
    base::RunLoop().RunUntilIdle();

    ASSERT_EQ(1u, write_log.size());
    EXPECT_LT(0, write_log[0]);
    EXPECT_EQ(sizeof(kTextToWrite) - 1, static_cast<size_t>(write_log[0]));

    FakeEntry entry;
    ASSERT_TRUE(provided_file_system_->GetEntry(
        base::FilePath::FromUTF8Unsafe(kFakeFilePath), &entry));
    EXPECT_EQ(kTextToWrite, entry.contents.substr(0, sizeof(kTextToWrite) - 1));
  }

  // Write additional data to be sure, that the writer's offset is shifted
  // properly.
  {
    const int result = writer.Write(io_buffer.get(),
                                    sizeof(kTextToWrite) - 1,
                                    base::Bind(&LogValue, &write_log));
    EXPECT_EQ(net::ERR_IO_PENDING, result);
    base::RunLoop().RunUntilIdle();

    ASSERT_EQ(2u, write_log.size());
    EXPECT_LT(0, write_log[0]);
    EXPECT_EQ(sizeof(kTextToWrite) - 1, static_cast<size_t>(write_log[0]));

    FakeEntry entry;
    ASSERT_TRUE(provided_file_system_->GetEntry(
        base::FilePath::FromUTF8Unsafe(kFakeFilePath), &entry));

    // The testing text is written twice.
    const std::string expected_contents =
        std::string(kTextToWrite) + kTextToWrite;
    EXPECT_EQ(expected_contents,
              entry.contents.substr(0, expected_contents.size()));
  }
}

TEST_F(FileSystemProviderFileStreamWriter, Write_WrongFile) {
  std::vector<int> write_log;

  const int64 initial_offset = 0;
  FileStreamWriter writer(wrong_file_url_, initial_offset);
  scoped_refptr<net::IOBuffer> io_buffer(new net::StringIOBuffer(kTextToWrite));

  const int result = writer.Write(io_buffer.get(),
                                  sizeof(kTextToWrite) - 1,
                                  base::Bind(&LogValue, &write_log));
  EXPECT_EQ(net::ERR_IO_PENDING, result);
  base::RunLoop().RunUntilIdle();

  ASSERT_EQ(1u, write_log.size());
  EXPECT_EQ(net::ERR_FILE_NOT_FOUND, write_log[0]);
}

TEST_F(FileSystemProviderFileStreamWriter, Write_Append) {
  std::vector<int> write_log;

  FakeEntry entry_before;
  ASSERT_TRUE(provided_file_system_->GetEntry(
      base::FilePath::FromUTF8Unsafe(kFakeFilePath), &entry_before));

  const int64 initial_offset = entry_before.metadata.size;
  ASSERT_LT(0, initial_offset);

  FileStreamWriter writer(file_url_, initial_offset);
  scoped_refptr<net::IOBuffer> io_buffer(new net::StringIOBuffer(kTextToWrite));

  const int result = writer.Write(io_buffer.get(),
                                  sizeof(kTextToWrite) - 1,
                                  base::Bind(&LogValue, &write_log));
  EXPECT_EQ(net::ERR_IO_PENDING, result);
  base::RunLoop().RunUntilIdle();

  ASSERT_EQ(1u, write_log.size());
  EXPECT_EQ(sizeof(kTextToWrite) - 1, static_cast<size_t>(write_log[0]));

  FakeEntry entry_after;
  ASSERT_TRUE(provided_file_system_->GetEntry(
      base::FilePath::FromUTF8Unsafe(kFakeFilePath), &entry_after));
  const std::string expected_contents = entry_before.contents + kTextToWrite;
  EXPECT_EQ(expected_contents, entry_after.contents);
}

}  // namespace file_system_provider
}  // namespace chromeos
