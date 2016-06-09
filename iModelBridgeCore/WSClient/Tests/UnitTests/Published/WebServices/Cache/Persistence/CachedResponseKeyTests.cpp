/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Persistence/CachedResponseKeyTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachedResponseKeyTests.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(CachedResponseKeyTests, IsValid_DefaultCtor_False)
    {
    EXPECT_FALSE(CachedResponseKey().IsValid());
    }

TEST_F(CachedResponseKeyTests, IsValid_ValidParentAndEmptyName_True)
    {
    EXPECT_TRUE(CachedResponseKey(StubECInstanceKey(), "").IsValid());
    }

TEST_F(CachedResponseKeyTests, IsValid_ValidParentAndNonEmptyName_True)
    {
    EXPECT_TRUE(CachedResponseKey(StubECInstanceKey(), "Foo").IsValid());
    }
