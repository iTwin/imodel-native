/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/TestFixture/UnitsTestFixture.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../UnitsTestsPch.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Basanta.Kharel     12/2015
//=======================================================================================    
struct UnitsTestFixture : public ::testing::Test
    {
    public:
        static Utf8String GetConversionDataPath(WCharCP dataFile);
        static Utf8String GetOutputDataPath(WCharCP dataFile);
    };

END_UNITS_UNITTESTS_NAMESPACE