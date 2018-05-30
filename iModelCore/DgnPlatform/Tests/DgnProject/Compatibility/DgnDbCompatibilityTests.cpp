/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/DgnDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

struct DgnDbCompatibilityTestFixture : CompatibilityTestFixture
    {
    ScopedDgnHost m_host;

    protected:
       Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::DgnDb); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnDbCompatibilityTestFixture, OpenAllVersionInReadWriteMode)
    {
    }

