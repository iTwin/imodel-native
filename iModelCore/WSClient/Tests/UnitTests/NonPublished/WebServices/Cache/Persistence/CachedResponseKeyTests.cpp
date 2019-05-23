/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "CachedResponseKeyTests.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachedResponseKeyTests, IsValid_DefaultCtor_False)
    {
    EXPECT_FALSE(CachedResponseKey().IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachedResponseKeyTests, IsValid_ValidParentAndEmptyName_True)
    {
    EXPECT_TRUE(CachedResponseKey(StubECInstanceKey(), "").IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachedResponseKeyTests, IsValid_ValidParentAndNonEmptyName_True)
    {
    EXPECT_TRUE(CachedResponseKey(StubECInstanceKey(), "Foo").IsValid());
    }
