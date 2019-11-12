/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <BeSQLite/RTreeMatch.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlRangeTreeTests : ECDbTestFixture
    {
    //---------------------------------------------------------------------------------------
    // @bsimethod                                     Krischan.Eberle                  05/15
    //+---------------+---------------+---------------+---------------+---------------+------
    BentleyStatus AddRowToRtree(ECDbCR ecdb, ECInstanceId id, DRange2dCR boundingBox)
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
    void SetupTestFile()
        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("rtreetest.ecdb"));
        //create rtree before importing the schema
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE demo_rtree USING rtree(ECInstanceId,minX,maxX,minY,maxY);"));

        ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
            "<ECSchema schemaName='RangeTreeTest' alias='rt' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "  <ECEntityClass typeName='DemoRTree' modifier='Sealed'>"
            "       <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.02.00'>"
            "                <MapStrategy>ExistingTable</MapStrategy>"
            "               <TableName>demo_rtree</TableName>"
            "                <ECInstanceIdColumn>ECInstanceId</ECInstanceIdColumn>"
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
            "</ECSchema>")));

        //populate with data (sample data taken from https://www.sqlite.org/rtree.html)
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO rt.DemoData (Name) VALUES(?)"));

        //first entry
        stmt.BindText(1, "SQLite headquarters", IECSqlBinder::MakeCopy::Yes);
        ECInstanceKey newKey;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));

        ASSERT_EQ(SUCCESS, AddRowToRtree(m_ecdb, newKey.GetInstanceId(), DRange2d::From(-80.7749, 35.3776, -80.7747, 35.3778)));

        stmt.Reset();
        stmt.ClearBindings();

        //second entry
        stmt.BindText(1, "NC 12th Congressional District in 2010", IECSqlBinder::MakeCopy::Yes);
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
        stmt.Finalize();

        ASSERT_EQ(SUCCESS, AddRowToRtree(m_ecdb, newKey.GetInstanceId(), DRange2d::From(-81.0, 35.0, -79.6, 36.2)));

        m_ecdb.SaveChanges();
        Utf8String filePath(m_ecdb.GetDbFileName());
        m_ecdb.CloseDb();
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly)));
        }
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlRangeTreeTests, SimpleQuery)
    {
    SetupTestFile();
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    //sample data taken from https ://www.sqlite.org/rtree.html
    Utf8CP ecsql = "SELECT d.Name FROM rt.DemoData d, rt.DemoRTree rt "
        "WHERE d.ECInstanceId = rt.ECInstanceId AND "
        "rt.MinX>=-81.08 AND rt.MaxX <=-80.58 AND rt.MinY>=35.00 AND rt.MaxY<=35.44";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));

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
    int _TestRange(QueryInfo const& info) override
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
TEST_F(ECSqlRangeTreeTests, MatchQuery)
    {
    SetupTestFile();
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    BBox2DMatchFunction bbox2dFunc;
    ASSERT_EQ(0, m_ecdb.AddRTreeMatchFunction(bbox2dFunc));

    //sample data taken from https ://www.sqlite.org/rtree.html
    Utf8CP ecsql = "SELECT d.Name FROM rt.DemoData d, rt.DemoRTree rt "
        "WHERE d.ECInstanceId = rt.ECInstanceId AND "
        "rt.ECInstanceId MATCH BBOX2D(-81.08,35.00,-80.58,35.44)";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));

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
TEST_F(ECSqlRangeTreeTests, MatchQueryWithParameters)
    {
    SetupTestFile();
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    BBox2DMatchFunction bbox2dFunc;
    ASSERT_EQ(0, m_ecdb.AddRTreeMatchFunction(bbox2dFunc));

    //sample data taken from https ://www.sqlite.org/rtree.html
    Utf8CP ecsql = "SELECT d.Name FROM rt.DemoData d, rt.DemoRTree rt "
        "WHERE d.ECInstanceId = rt.ECInstanceId AND "
        "rt.ECInstanceId MATCH BBOX2D(?,?,?,?)";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));

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
