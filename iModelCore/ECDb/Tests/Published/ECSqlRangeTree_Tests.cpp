/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlRangeTree_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h"
#include <BeSQLite/RTreeMatch.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AddRowToRtree(ECDbR ecdb, ECInstanceId id, DRange2dCR boundingBox)
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
        "<ECSchema schemaName=\"RangeTreeTest\" nameSpacePrefix=\"rt\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "  <ECSchemaReference version='01.12' prefix='bsca' name ='Bentley_Standard_CustomAttributes' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName=\"DemoRTree\" >"
        "       <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>ExistingTable</Strategy>"
        "                </MapStrategy>"
        "               <TableName>demo_rtree</TableName>"
        "            </ClassMap>"
        "       </ECCustomAttributes>"
        "       <ECProperty typeName='double' propertyName='MinX' />"
        "       <ECProperty typeName='double' propertyName='MaxX' />"
        "       <ECProperty typeName='double' propertyName='MinY' />"
        "       <ECProperty typeName='double' propertyName='MaxY' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='DemoData' >"
        "       <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
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

    //populate with data (sample data taken from https://www.sqlite.org/rtree.html)
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, "INSERT INTO rt.DemoData (Name) VALUES(?)"))
        return ERROR;

    //first entry
    stmt.BindText(1, "SQLite headquarters", IECSqlBinder::MakeCopy::Yes);
    ECInstanceKey newKey;
    if (BE_SQLITE_DONE != stmt.Step(newKey))
        return ERROR;

    if (SUCCESS != AddRowToRtree(ecdb, newKey.GetECInstanceId(), DRange2d::From(-80.7749, 35.3776, -80.7747, 35.3778)))
        return ERROR;

    stmt.Reset();
    stmt.ClearBindings();

    //second entry
    stmt.BindText(1, "NC 12th Congressional District in 2010", IECSqlBinder::MakeCopy::Yes);
    if (BE_SQLITE_DONE != stmt.Step(newKey))
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
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    //sample data taken from https ://www.sqlite.org/rtree.html
    Utf8CP ecsql = "SELECT d.Name FROM rt.DemoData d, rt.DemoRTree rt "
        "WHERE d.ECInstanceId = rt.ECInstanceId AND "
        "rt.MinX>=-81.08 AND rt.MaxX <=-80.58 AND rt.MinY>=35.00 AND rt.MaxY<=35.44";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql));

    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_STREQ("SQLite headquarters", stmt.GetValueText(0)) << "ECSQL " << ecsql << " is expected to only return SQLite headquarters entry";
        }

    ASSERT_EQ(1, rowCount) << "ECSQL " << ecsql << " is expected to only return SQLite headquarters entry";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
struct BBox2DMatchFunction : RTreeMatchFunction
    {
private:
    virtual int _TestRange(QueryInfo const& info) override
        {
        if (info.m_nParam != 4)
            return BE_SQLITE_ERROR;

        info.m_within = Within::Outside;
        RTree2dVal bbox2d (DRange2d::From(info.m_args[0].GetValueDouble(), info.m_args[1].GetValueDouble(), info.m_args[2].GetValueDouble(), info.m_args[3].GetValueDouble()));
        RTree2dValCP pt = (RTree2dValCP) info.m_coords;

        bool passedTest = (info.m_parentWithin == Within::Inside) ? true : bbox2d.Contains(*pt);
        if (!passedTest)
            return BE_SQLITE_OK;

        if (info.m_level > 0)
            {
            // For nodes, return 'level-score'.
            info.m_score = info.m_level;
            info.m_within = info.m_parentWithin == Within::Inside ? Within::Inside : bbox2d.Contains(*pt) ? Within::Inside : Within::Partly;
            }
        else
            {
            // For leaves (m_level==0), we return 0 so they are processed immediately (lowest score has highest priority).
            info.m_score = 0;
            info.m_within = Within::Partly;
            }

        return BE_SQLITE_OK;
        }

public:
    BBox2DMatchFunction() : RTreeMatchFunction("BBOX2D", 4) {}
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlRangeTreeTests, MatchQuery)
    {
    Utf8String ecdbPath;
    ASSERT_EQ(SUCCESS, CreateRangeTreeTestProject(ecdbPath));

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    BBox2DMatchFunction bbox2dFunc;
    ASSERT_EQ(0, ecdb.AddRTreeMatchFunction(bbox2dFunc));

    //sample data taken from https ://www.sqlite.org/rtree.html
    Utf8CP ecsql = "SELECT d.Name FROM rt.DemoData d, rt.DemoRTree rt "
        "WHERE d.ECInstanceId = rt.ECInstanceId AND "
        "rt.ECInstanceId MATCH BBOX2D(-81.08,35.00,-80.58,35.44)";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql));

    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_STREQ("SQLite headquarters", stmt.GetValueText(0));
        }

    ASSERT_EQ(1, rowCount) << "ECSQL " << ecsql << " is expected to only return SQLite headquarters entry";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlRangeTreeTests, MatchQueryWithParameters)
    {
    Utf8String ecdbPath;
    ASSERT_EQ(SUCCESS, CreateRangeTreeTestProject(ecdbPath));

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    BBox2DMatchFunction bbox2dFunc;
    ASSERT_EQ(0, ecdb.AddRTreeMatchFunction(bbox2dFunc));

    //sample data taken from https ://www.sqlite.org/rtree.html
    Utf8CP ecsql = "SELECT d.Name FROM rt.DemoData d, rt.DemoRTree rt "
        "WHERE d.ECInstanceId = rt.ECInstanceId AND "
        "rt.ECInstanceId MATCH BBOX2D(?,?,?,?)";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql));

    stmt.BindDouble(1, -81.08);
    stmt.BindDouble(2, 35.00);
    stmt.BindDouble(3, -80.58);
    stmt.BindDouble(4, 35.44);

    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_STREQ("SQLite headquarters", stmt.GetValueText(0));
        }

    ASSERT_EQ(1, rowCount) << "ECSQL " << ecsql << " is expected to only return SQLite headquarters entry";
    }

END_ECDBUNITTESTS_NAMESPACE
