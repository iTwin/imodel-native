/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "UrlSafeBase64UtilitiesTests.h"

#include <Licensing/Utils/UrlSafeBase64Utilities.h>

USING_NAMESPACE_BENTLEY_LICENSING

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

TEST_F(UrlSafeBase64UtilitiesTests, ToBase64_AlreadyBase64_ReturnsSameString)
    {
    EXPECT_EQ("c2FkZmFzZGZh", UrlSafeBase64Utilities::ToBase64("c2FkZmFzZGZh"));
    }

TEST_F(UrlSafeBase64UtilitiesTests, ToBase64_ReplaceSymbolsCorrectly)
    {
    EXPECT_EQ ("Y+N/ZmFz+mR//2+h", UrlSafeBase64Utilities::ToBase64("Y-N_ZmFz-mR__2-h"));
    }

TEST_F(UrlSafeBase64UtilitiesTests, ToBase64_AppendsMissingEqSymbols)
    {
    EXPECT_EQ("c2FkZmFzZGY=", UrlSafeBase64Utilities::ToBase64("c2FkZmFzZGY"));
    EXPECT_EQ("c2FkZmFzZG==", UrlSafeBase64Utilities::ToBase64("c2FkZmFzZG"));
    EXPECT_EQ("c2FkZmFzZ===", UrlSafeBase64Utilities::ToBase64("c2FkZmFzZ"));
    EXPECT_EQ("c2FkZmFz",     UrlSafeBase64Utilities::ToBase64("c2FkZmFz"));
    }
