/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Persistence/CacheEnvironmentTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CacheEnvironmentTests.h"

#include <WebServices/Cache/Persistence/CacheEnvironment.h>

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(CacheEnvironmentTests, Ctor_Default_EmptyPaths)
    {
    CacheEnvironment env;
    EXPECT_EQ(L"", env.persistentFileCacheDir);
    EXPECT_EQ(L"", env.temporaryFileCacheDir);
    EXPECT_EQ(L"", env.externalFileCacheDir);
    }

TEST_F(CacheEnvironmentTests, Ctor_PersistentAndTemporaryPaths_SetsPaths)
    {
    CacheEnvironment env(BeFileName(L"A"), BeFileName(L"B"));
    EXPECT_EQ(L"A", env.persistentFileCacheDir);
    EXPECT_EQ(L"B", env.temporaryFileCacheDir);
    EXPECT_EQ(L"", env.externalFileCacheDir);
    }

TEST_F(CacheEnvironmentTests, GetPath_UnknownLocationPassed_Empty)
    {
    CacheEnvironment env;
    env.persistentFileCacheDir = BeFileName(L"A");
    env.temporaryFileCacheDir = BeFileName(L"B");
    env.externalFileCacheDir = BeFileName(L"C");
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(L"", env.GetPath((FileCache) 666));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(CacheEnvironmentTests, GetPath_FileCacheLocationPassed_ReturnsApproapriatePath)
    {
    CacheEnvironment env;
    env.persistentFileCacheDir = BeFileName(L"A");
    env.temporaryFileCacheDir = BeFileName(L"B");
    env.externalFileCacheDir = BeFileName(L"C");
    EXPECT_EQ(L"A", env.GetPath(FileCache::Persistent));
    EXPECT_EQ(L"B", env.GetPath(FileCache::Temporary));
    EXPECT_EQ(L"C", env.GetPath(FileCache::External));
    }

TEST_F(CacheEnvironmentTests, GetRootFolderId_UnknownLocationPassed_Negative)
    {
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(-1, CacheEnvironment::GetRootFolderId((FileCache) 666));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(CacheEnvironmentTests, GetRootFolderId_FileCacheLocationPassed_ReturnsValid)
    {
    EXPECT_NE(-1, CacheEnvironment::GetRootFolderId(FileCache::Persistent));
    EXPECT_NE(-1, CacheEnvironment::GetRootFolderId(FileCache::Temporary));
    EXPECT_NE(-1, CacheEnvironment::GetRootFolderId(FileCache::External));
    }
