/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/BeDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

struct BeDbCompatibilityTestFixture : CompatibilityTestFixture 
    {
protected:
    Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::BeDb); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeDbCompatibilityTestFixture, OpenAllVersionInReadWriteMode)
    {
    }

