/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/ECDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

struct ECDbCompatibilityTestFixture : CompatibilityTestFixture 
    {
    protected:
        Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::ECDb); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, OpenAllVersionInReadWriteMode)
    {
    }

