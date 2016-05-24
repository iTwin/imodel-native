/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/UnitsTestFixture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UnitsTestFixture.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void UnitsTestFixture::SetUp()
    {
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnitsTestFixture::GetConversionDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testData);
    testData.AppendToPath(L"ConversionData");
    testData.AppendToPath(dataFile);
    return Utf8String(testData);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnitsTestFixture::GetOutputDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetOutputRoot(testData);
    testData.AppendToPath(dataFile);
    return Utf8String(testData);
    }
END_UNITS_UNITTESTS_NAMESPACE