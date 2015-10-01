/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Performance/PerformanceTestingFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTestingFixture.h"

//---------------------------------------------------------------------------------------
// Creating a new Db for the test
// @bsimethod                                    Majd.Uddin                   06/12
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName PerformanceTestFixtureBase::getDbFilePath(WCharCP dbName)
{
    BeFileName dbFileName;
    BeTest::GetHost().GetOutputRoot(dbFileName);
    dbFileName.AppendToPath(dbName);
    return dbFileName;
}

//---------------------------------------------------------------------------------------
// Creating a new Db for the test
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PerformanceTestFixtureBase::SetupDb(Db& db, WCharCP dbName)
{
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    BeFileName dbFullName = getDbFilePath(dbName);
    if (BeFileName::DoesPathExist(dbFullName))
        BeFileName::BeDeleteFile(dbFullName);
    DbResult result = db.CreateNewDb(dbFullName.GetNameUtf8().c_str());
    EXPECT_EQ(BE_SQLITE_OK, result) << "Db Creation failed";
    return result;
}



