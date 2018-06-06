/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/BeDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"
#include "ProfileManager.h"

struct BeDbCompatibilityTestFixture : CompatibilityTestFixture 
    {
protected:
    Profile& Profile() const { return ProfileManager::Get().GetProfile(ProfileType::BeDb); }
    };

