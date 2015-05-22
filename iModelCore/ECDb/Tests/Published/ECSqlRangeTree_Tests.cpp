/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlRangeTree_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AddRowToRtree(ECDbR ecdb, ECInstanceId const& id, DRange2dCR boundingBox)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "INSERT INTO demo_rtree VALUES(?,?,?,?,?)"))
        return ERROR;

    stmt.BindId(1, id);
    stmt.BindDouble(2, boundingBox.low.x);
    stmt.BindDouble(3, boundingBox.high.x);
    stmt.BindDouble(4, boundingBox.low.y);
    stmt.BindDouble(5, boundingBox.high.y);

    return stmt.Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CreateRangeTreeTestProject(Utf8StringR ecdbPath)
    {
    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"RangeTreeTest\" nameSpacePrefix=\"rt\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference version='01.12' prefix='bsca' name ='Bentley_Standard_CustomAttributes' />"
        "  <ECClass typeName=\"DemoRTree\" isDomainClass='True'>"
        "       <ECCustomAttributes>"
        "           <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.12'>"
        "               <MapStrategy>MapToExistingTable</MapStrategy>"
        "               <TableName>demo_rtree</TableName>"
        "           </ECDbClassHint>"
        "       </ECCustomAttributes>"
        "       <ECProperty typeName='double' propertyName='MinX' />"
        "       <ECProperty typeName='double' propertyName='MaxX' />"
        "       <ECProperty typeName='double' propertyName='MinY' />"
        "       <ECProperty typeName='double' propertyName='MaxY' />"
        "  </ECClass>"
        "  <ECClass typeName='DemoData' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='string' />"
        "  </ECClass>"
        "</ECSchema>";

    ECDbTestProject testProject;
    ECDbR ecdb = testProject.Create("rtreetest.ecdb");
    ecdbPath = ecdb.GetDbFileName();

    //create rtree before importing the schema
    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE VIRTUAL TABLE demo_rtree USING rtree(ECInstanceId,minX,maxX,minY,maxY);"))
        return ERROR;

    auto schemaCache = ECDbTestUtility::ReadECSchemaFromString(testSchemaXml);
    if (SUCCESS != ecdb.Schemas().ImportECSchemas(*schemaCache))
        return ERROR;

    ecdb.SaveChanges();

    //populate with data
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, "INSERT INTO rt.DemoData (Name) VALUES(?)"))
        return ERROR;

    //first entry
    stmt.BindText(1, "SQLite headquarters", IECSqlBinder::MakeCopy::Yes);
    ECInstanceKey newKey;
    if (ECSqlStepStatus::Done != stmt.Step(newKey))
        return ERROR;

    if (SUCCESS != AddRowToRtree(ecdb, newKey.GetECInstanceId(), DRange2d::From(-80.7749, 35.3776, -80.7747, 35.3778)))
        return ERROR;

    stmt.Reset();
    stmt.ClearBindings();

    //second entry
    stmt.BindText(1, "NC 12th Congressional District in 2010", IECSqlBinder::MakeCopy::Yes);
    if (ECSqlStepStatus::Done != stmt.Step(newKey))
        return ERROR;

    if (SUCCESS != AddRowToRtree(ecdb, newKey.GetECInstanceId(), DRange2d::From(-81.0, 35.0, -79.6, 36.2)))
        return ERROR;

    ecdb.SaveChanges();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlRangeTreeTests, SimpleQuery)
    {
    Utf8String ecdbPath;
    ASSERT_EQ(SUCCESS, CreateRangeTreeTestProject(ecdbPath));

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(ECDb::OpenMode::OPEN_Readonly)));

    Utf8CP ecsql = "SELECT d.Name FROM rt.DemoData d, rt.DemoRTree rt "
        "WHERE d.ECInstanceId = rt.ECInstanceId AND "
        "rt.MinX>=-81.08 AND rt.MaxX <=-80.58 AND rt.MinY>=35.00 AND rt.MaxY<=35.44";

    ECSqlStatement stmt;
    ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, ecsql)) << stmt.GetLastStatusMessage();

    int rowCount = 0;
    while (stmt.Step() == ECSqlStepStatus::HasRow)
        {
        rowCount++;
        ASSERT_STREQ("SQLite headquarters", stmt.GetValueText(0)) << "ECSQL " << ecsql << " is expected to only return SQLite headquarters entry";
        }

    ASSERT_EQ(1, rowCount) << "ECSQL " << ecsql << " is expected to only return SQLite headquarters entry";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlRangeTreeTests, MatchQuery)
    {
    Utf8String ecdbPath;
    ASSERT_EQ(SUCCESS, CreateRangeTreeTestProject(ecdbPath));

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(ECDb::OpenMode::OPEN_Readonly)));

    Utf8CP ecsql = "SELECT d.Name FROM rt.DemoData d, rt.DemoRTree rt "
        "WHERE d.ECInstanceId = rt.ECInstanceId AND "
        "ECInstanceId MATCH BBOX2D(-81.08,35.00,-80.58,35.44)";

    ECSqlStatement stmt;
    ASSERT_EQ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare(ecdb, ecsql)) << stmt.GetLastStatusMessage();

    /* defining a geometry function not yet possible in the BeSQLite API. Once this is available
       this test will leverage that
    int rowCount = 0;
    while (stmt.Step() == ECSqlStepStatus::HasRow)
        {
        ASSERT_STREQ("SQLite headquarters", stmt.GetValueText(0));
        }

    ASSERT_EQ(1, rowCount) << "ECSQL " << ecsql << " is expected to only return SQLite headquarters entry";
    */
    }

END_ECDBUNITTESTS_NAMESPACE
