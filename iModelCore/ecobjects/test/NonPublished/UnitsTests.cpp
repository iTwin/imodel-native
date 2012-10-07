/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/UnitsTests.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "StopWatch.h"
#include "TestFixture.h"
#include <ECUnits/Units.h>

using namespace Bentley::EC;

BEGIN_BENTLEY_EC_UNITS_NAMESPACE

struct UnitsTest : ECTestFixture
    {
    void            LocateStandardSystem (WCharCP name, StandardUnitSystem id)
        {
        EXPECT_TRUE (0 == wcscmp (UnitSystem::GetStandard (id).GetName(), name));
        UnitSystemCP us = UnitSystem::GetByName (name);
        EXPECT_TRUE (NULL != us);
        EXPECT_TRUE (0 == wcscmp (us->GetName(), name));
        EXPECT_TRUE (us == &UnitSystem::GetStandard (id));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitsTest, InitializeStandards)
    {
    LocateStandardSystem (L"none", StandardUnitSystem_None);
    LocateStandardSystem (L"usCustomary", StandardUnitSystem_USCustomary);
    LocateStandardSystem (L"si", StandardUnitSystem_SI);
    LocateStandardSystem (L"both", StandardUnitSystem_Both);
    }

END_BENTLEY_EC_UNITS_NAMESPACE
