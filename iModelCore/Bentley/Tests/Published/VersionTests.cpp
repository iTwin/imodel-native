/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/VersionTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "VersionTests.h"

USING_NAMESPACE_BENTLEY

TEST_F (VersionTests, Ctor_AllValuesPassed_SetsValues)
    {
    BeVersion version (1, 2, 3, 4);
    EXPECT_EQ (1, version.GetMajor());
    EXPECT_EQ (2, version.GetMinor());
    EXPECT_EQ (3, version.GetSub1());
    EXPECT_EQ (4, version.GetSub2());
    }

TEST_F (VersionTests, Ctor_MajorAndMinorValuesPassed_SetsValuesSubValuesToZero)
    {
    BeVersion version (1, 2);
    EXPECT_EQ (1, version.GetMajor());
    EXPECT_EQ (2, version.GetMinor());
    EXPECT_EQ (0, version.GetSub1());
    EXPECT_EQ (0, version.GetSub2());
    }

TEST_F (VersionTests, Ctor_StringWithDefaultFormat_SetsValues)
    {
    BeVersion version ("01.2.00003.04");
    EXPECT_EQ (1, version.GetMajor());
    EXPECT_EQ (2, version.GetMinor());
    EXPECT_EQ (3, version.GetSub1());
    EXPECT_EQ (4, version.GetSub2());
    }

TEST_F (VersionTests, Ctor_StringWithCustomFormat_SetsValues)
    {
    BeVersion version ("1:2,3-4", "%d:%d,%d-%d");
    EXPECT_EQ (1, version.GetMajor());
    EXPECT_EQ (2, version.GetMinor());
    EXPECT_EQ (3, version.GetSub1());
    EXPECT_EQ (4, version.GetSub2());
    }

TEST_F (VersionTests, Ctor_BadFormat_SetsValuesToZero)
    {
    BeVersion version ("not_version_format");
    EXPECT_EQ (0, version.GetMajor());
    EXPECT_EQ (0, version.GetMinor());
    EXPECT_EQ (0, version.GetSub1());
    EXPECT_EQ (0, version.GetSub2());
    }

TEST_F (VersionTests, IsEmpty_ValuesSet_False)
    {
    EXPECT_FALSE (BeVersion (0, 0, 0, 1).IsEmpty ());
    EXPECT_FALSE (BeVersion (0, 0, 1, 0).IsEmpty ());
    EXPECT_FALSE (BeVersion (0, 1, 0, 0).IsEmpty ());
    EXPECT_FALSE (BeVersion (1, 0, 0, 0).IsEmpty ());
    EXPECT_FALSE (BeVersion (1, 2, 3, 4).IsEmpty ());
    }

TEST_F (VersionTests, IsEmpty_ZeroValues_True)
    {
    EXPECT_TRUE (BeVersion (0, 0, 0, 0).IsEmpty ());
    EXPECT_TRUE (BeVersion ().IsEmpty ());
    }

TEST_F (VersionTests, ToString_ValuesSet_WritesValuesInDefaultFormat)
    {
    BeVersion version (1, 2, 0, 4);
    EXPECT_STREQ ("1.2.0.4", version.ToString ().c_str ());
    }

TEST_F (VersionTests, OperatorEqual_Equal_True)
    {
    EXPECT_TRUE (BeVersion (1, 1, 1, 1) == BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorEqual_Ineuqal_False)
    {
    EXPECT_FALSE (BeVersion (2, 1, 1, 1) == BeVersion (1, 1, 1, 1));
    EXPECT_FALSE (BeVersion (1, 2, 1, 1) == BeVersion (1, 1, 1, 1));
    EXPECT_FALSE (BeVersion (1, 1, 2, 1) == BeVersion (1, 1, 1, 1));
    EXPECT_FALSE (BeVersion (1, 1, 1, 2) == BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorInequal_Equal_False)
    {
    EXPECT_FALSE (BeVersion (1, 1, 1, 1) != BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorInequal_Ineuqal_True)
    {
    EXPECT_TRUE (BeVersion (2, 1, 1, 1) != BeVersion (1, 1, 1, 1));
    EXPECT_TRUE (BeVersion (1, 2, 1, 1) != BeVersion (1, 1, 1, 1));
    EXPECT_TRUE (BeVersion (1, 1, 2, 1) != BeVersion (1, 1, 1, 1));
    EXPECT_TRUE (BeVersion (1, 1, 1, 2) != BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorLess_Less_True)
    {
    EXPECT_TRUE (BeVersion (1, 2, 2, 2) < BeVersion (2, 1, 1, 1));
    EXPECT_TRUE (BeVersion (1, 1, 2, 2) < BeVersion (1, 2, 1, 1));
    EXPECT_TRUE (BeVersion (1, 1, 1, 2) < BeVersion (1, 1, 2, 1));
    EXPECT_TRUE (BeVersion (1, 1, 1, 1) < BeVersion (1, 1, 1, 2));
    }

TEST_F (VersionTests, OperatorLess_Equal_False)
    {
    EXPECT_FALSE (BeVersion (1, 1, 1, 1) < BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorLess_Greater_False)
    {
    EXPECT_FALSE (BeVersion (2, 1, 1, 1) < BeVersion (1, 2, 2, 2));
    EXPECT_FALSE (BeVersion (1, 2, 1, 1) < BeVersion (1, 1, 2, 2));
    EXPECT_FALSE (BeVersion (1, 1, 2, 1) < BeVersion (1, 1, 1, 2));
    EXPECT_FALSE (BeVersion (1, 1, 1, 2) < BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorGreater_Greater_True)
    {
    EXPECT_TRUE (BeVersion (2, 1, 1, 1) > BeVersion (1, 2, 2, 2));
    EXPECT_TRUE (BeVersion (1, 2, 1, 1) > BeVersion (1, 1, 2, 2));
    EXPECT_TRUE (BeVersion (1, 1, 2, 1) > BeVersion (1, 1, 1, 2));
    EXPECT_TRUE (BeVersion (1, 1, 1, 2) > BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorGreater_Equal_False)
    {
    EXPECT_FALSE (BeVersion (1, 1, 1, 1) > BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorGreater_Less_False)
    {
    EXPECT_FALSE (BeVersion (1, 2, 2, 2) > BeVersion (2, 1, 1, 1));
    EXPECT_FALSE (BeVersion (1, 1, 2, 2) > BeVersion (1, 2, 1, 1));
    EXPECT_FALSE (BeVersion (1, 1, 1, 2) > BeVersion (1, 1, 2, 1));
    EXPECT_FALSE (BeVersion (1, 1, 1, 1) > BeVersion (1, 1, 1, 2));
    }

TEST_F (VersionTests, OperatorLessOrEqual_Less_True)
    {
    EXPECT_TRUE (BeVersion (1, 2, 2, 2) <= BeVersion (2, 1, 1, 1));
    EXPECT_TRUE (BeVersion (1, 1, 2, 2) <= BeVersion (1, 2, 1, 1));
    EXPECT_TRUE (BeVersion (1, 1, 1, 2) <= BeVersion (1, 1, 2, 1));
    EXPECT_TRUE (BeVersion (1, 1, 1, 1) <= BeVersion (1, 1, 1, 2));
    }

TEST_F (VersionTests, OperatorLessOrEqual_Equal_True)
    {
    EXPECT_TRUE (BeVersion (1, 1, 1, 1) <= BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorLessOrEqual_Greater_False)
    {
    EXPECT_FALSE (BeVersion (2, 1, 1, 1) <= BeVersion (1, 2, 2, 2));
    EXPECT_FALSE (BeVersion (1, 2, 1, 1) <= BeVersion (1, 1, 2, 2));
    EXPECT_FALSE (BeVersion (1, 1, 2, 1) <= BeVersion (1, 1, 1, 2));
    EXPECT_FALSE (BeVersion (1, 1, 1, 2) <= BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorGreaterOrEqual_Greater_True)
    {
    EXPECT_TRUE (BeVersion (2, 1, 1, 1) >= BeVersion (1, 2, 2, 2));
    EXPECT_TRUE (BeVersion (1, 2, 1, 1) >= BeVersion (1, 1, 2, 2));
    EXPECT_TRUE (BeVersion (1, 1, 2, 1) >= BeVersion (1, 1, 1, 2));
    EXPECT_TRUE (BeVersion (1, 1, 1, 2) >= BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorGreaterOrEqual_Equal_True)
    {
    EXPECT_TRUE (BeVersion (1, 1, 1, 1) >= BeVersion (1, 1, 1, 1));
    }

TEST_F (VersionTests, OperatorGreaterOrEqual_Less_False)
    {
    EXPECT_FALSE (BeVersion (1, 2, 2, 2) >= BeVersion (2, 1, 1, 1));
    EXPECT_FALSE (BeVersion (1, 1, 2, 2) >= BeVersion (1, 2, 1, 1));
    EXPECT_FALSE (BeVersion (1, 1, 1, 2) >= BeVersion (1, 1, 2, 1));
    EXPECT_FALSE (BeVersion (1, 1, 1, 1) >= BeVersion (1, 1, 1, 2));
    }
