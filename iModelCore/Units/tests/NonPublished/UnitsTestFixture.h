/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/UnitsTestFixture.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "UnitsTests.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Basanta.Kharel     12/2015
//=======================================================================================    
struct UnitsTestFixture : public ::testing::Test
    {
    public:
        UnitsTestFixture() : ::testing::Test() {}
        virtual ~UnitsTestFixture() {}
        virtual void SetUp() override;
        virtual void TearDown() override {}

        static Utf8String GetConversionDataPath(WCharCP dataFile);
        static Utf8String GetOutputDataPath(WCharCP dataFile);
    };

END_UNITS_UNITTESTS_NAMESPACE