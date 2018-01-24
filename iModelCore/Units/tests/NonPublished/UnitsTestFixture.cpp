/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/UnitsTestFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UnitsTestFixture.h"
#include <BeSQLite/L10N.h>

BEGIN_UNITS_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void UnitsTestFixture::SetUp()
    {
    BeFileName sqlangFile;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(sqlangFile);
    sqlangFile.AppendToPath(L"sqlang");
    sqlangFile.AppendToPath(L"Units_en.sqlang.db3");

    BeFileName temporaryDirectory;
    BeTest::GetHost().GetTempDir(temporaryDirectory);

    BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory, BeSQLite::BeSQLiteLib::LogErrors::Yes);
    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(sqlangFile));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Bill Steinbock 12/17
//----------------------------------------------------------------------------------------
void UnitsTestFixture::TearDown()
    {
    BeSQLite::L10N::Shutdown();
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