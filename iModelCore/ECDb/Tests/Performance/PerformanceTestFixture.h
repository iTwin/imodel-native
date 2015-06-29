/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Performance/PerformanceTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>
#include <BeSQLite/BeSQLite.h>

#include "PerformanceTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      09/2012
//=======================================================================================    
struct PerformanceTestFixture : public ::testing::Test
    {
private:
    BeFileName m_testDbPath;

protected:
    PerformanceTestFixture();
    virtual ~PerformanceTestFixture () {};

    virtual void InitializeTestDb () = 0;

    void CreateEmptyDb (BeSQLite::EC::ECDbR db, BeFileNameCR dbPath);
    void SetTestDbPath (BeFileNameCR dbPath);

    void OpenTestDb (BeSQLite::EC::ECDbR testDb, BeSQLite::Db::OpenMode openMode = BeSQLite::Db::OpenMode::Readonly, BeSQLite::DefaultTxn defaultTransactionMode = BeSQLite::DefaultTxn::Yes) const;

    static void ImportSchema (BeSQLite::EC::ECDbR testDb, ECN::ECSchemaPtr schema, ECN::ECSchemaReadContextPtr ecSchemaReadContext);
    static void InsertTestData (BeSQLite::EC::ECDbR db, ECN::ECClassCP ecClass, int instanceCount);
    static void InsertTestInstance (ECN::ECClassCR ecClass, ECN::StandaloneECEnablerR instanceEnabler, ECInstanceInserter const& inserter);
    static void AssignRandomECValue (ECN::ECValue& ecValue, ECN::ECPropertyCR ecProp);

    static void ReadECSchemaFromFile (ECN::ECSchemaPtr& ecSchema, ECN::ECSchemaReadContextPtr& ecSchemaReadContext, WCharCP ecSchemaFileName);

    static void LogResultsToFile(bmap<Utf8String, double> results);

public:
    virtual void SetUp () override;
    virtual void TearDown () override;
    };
//=======================================================================================    
//! @bsiclass                                                Adeel.Shoukat    07/2014
//=======================================================================================
struct PerformanceTestingFrameWork
{
private:
    Db m_Db;
    ECSqlStatement stmt;
    DbResult dbOpenStat;
    BeFileName dir;

public:
    PerformanceTestingFrameWork()
    {
    }
    void openDb();
    bool writeTodb(StopWatch &timerCount, Utf8String testName, Utf8String testDescription);
    bool writeTodb(double timeInSeconds, Utf8String testName, Utf8String testDescription);
};

END_ECDBUNITTESTS_NAMESPACE
