/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECSqlTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle         09/15
//=======================================================================================
struct PerformanceECSqlTestFixture : public ECDbTestFixture
    {
    struct TestItem
        {
        Utf8String m_ecsql;
        Utf8String m_sql;
        Utf8String m_description;

        TestItem(Utf8CP ecsql, Utf8CP description = nullptr, Utf8CP sql = nullptr) : m_ecsql(ecsql), m_sql(sql), m_description(description) {}

        bool HasSql() const { return !m_sql.empty(); }
        };
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlTestFixture, WhereClauseWithPrimaryKey)
    {
    const int instanceCount = 100000;

    {
    SetupECDb("ecsqlperformance.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite), instanceCount);
    
    }

    TestItem testItem("UPDATE ecsql.TH3 SET S='S', S1='S1', S2='S2', S3='S3' WHERE ECInstanceId=?");

    //for (int )

    
    }

END_ECDBUNITTESTS_NAMESPACE