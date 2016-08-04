/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnSqlTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include "DgnSqlTestDomain.h"
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_SQLITE

using namespace DgnSqlTestNamespace;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct SqlFunctionsTest : public ::testing::Test
{   
    static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;
    ScopedDgnHost m_host;
    DgnDbPtr      m_db;
    DgnModelId    m_defaultModelId;
    DgnCategoryId m_defaultCategoryId;
    BeSQLite::Db::OpenMode m_openMode;

    static void SetUpTestCase();
    void SetUp() override;
    void TearDown() override;
    
    void SetupProject(WCharCP newFileName, BeSQLite::Db::OpenMode);
    void InsertElement(PhysicalElementR pelem);
    DgnModelR GetDefaultModel() {return *m_db->Models().GetModel(m_defaultModelId);}
    SpatialModelP GetDefaultSpatialModel() {return dynamic_cast<SpatialModelP>(&GetDefaultModel());}
    DgnCode CreateCode(Utf8StringCR value) const { return NamespaceAuthority::CreateCode("SqlFunctionsTest", value, *m_db); }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlFunctionsTest::SetUp()
    {
    // Must register my domain whenever I initialize a host
    DgnDomains::RegisterDomain(DgnSqlTestDomain::GetDomain());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlFunctionsTest::TearDown()
    {
    if (m_db.IsValid() && BeSQLite::Db::OpenMode::ReadWrite == m_openMode)
        m_db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlFunctionsTest::SetUpTestCase()
    {
    ScopedDgnHost tempHost;
    //  Request a root seed file.
    DgnDbTestUtils::SeedDbInfo seedFileInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, false));

    // Customize for my tests
    auto db = DgnDbTestUtils::OpenSeedDbCopy(seedFileInfo.fileName, L"SqlFunctionsTest/seed.bim");
    ASSERT_TRUE(db.IsValid());

    DgnSqlTestDomain::ImportSchemaFromPath(*db, T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());

    auto& hdlr = dgn_AuthorityHandler::Namespace::GetHandler();
    DgnAuthority::CreateParams params(*db, db->Domains().GetClassId(hdlr), "SqlFunctionsTest");
    DgnAuthorityPtr auth = hdlr.Create(params);
    if (auth.IsValid())
        auth->Insert();

    db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlFunctionsTest::SetupProject(WCharCP newFileName, BeSQLite::Db::OpenMode mode)
    {
    m_openMode = mode;
    if (BeSQLite::Db::OpenMode::Readonly == mode)
        m_db = DgnDbTestUtils::OpenSeedDb(L"SqlFunctionsTest/seed.bim");
    else
        m_db = DgnDbTestUtils::OpenSeedDbCopy(L"SqlFunctionsTest/seed.bim", newFileName);
    ASSERT_TRUE (m_db.IsValid());

    m_defaultModelId = m_db->Models().QueryFirstModelId();

    DgnModelPtr defaultModel = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(defaultModel.IsValid());
    GetDefaultModel().FillModel();
    
    m_defaultCategoryId = DgnCategory::QueryFirstCategoryId(*m_db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlFunctionsTest::InsertElement(PhysicalElementR pelem)
    {
    ASSERT_TRUE( m_db->Elements().Insert(pelem).IsValid() );
    ASSERT_TRUE( pelem.GetElementId().IsValid() ) << L"Insert is supposed to set the ElementId";
    }
#define EABB_FROM_GEOM "DGN_bbox(BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z)"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, placement_areaxy)
    {
    SetupProject(L"placement_areaxy.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    double o1y = 5.0;
    double o2x = 5.0;
    DPoint3d o1origin = DPoint3d::From(0,o1y,0);
    DPoint3d o2origin = DPoint3d::From(o2x,0,0);

    ObstacleElementPtr obstacleAt0 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o1origin, 0.0, CreateCode("obstacleAt0"));
    InsertElement(*obstacleAt0);
    obstacleAt0->SetSomeProperty(*m_db, "B");

    ObstacleElementPtr obstacle2At90 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o2origin, 90.0, CreateCode("obstacle2At90"));
    InsertElement(*obstacle2At90);
    obstacle2At90->SetSomeProperty(*m_db, "A");

    // Example of passing the wrong object to a SQL function
    if (true)
        {
        //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_bbox_areaxy_error.sampleCode
        // This is an example of passing the wrong type of object to DGN_bbox_areaxy and getting an error.
        // This statement is wrong, because DGN_placement_angles returns a DGN_angles object, while DGN_bbox_areaxy expects a DGN_bbox object.
        // Note that the error is detected when you try to step the statement, not when you prepare it.
        Statement stmt;
        stmt.Prepare(*m_db, "SELECT DGN_bbox_areaxy(DGN_angles(Yaw,Pitch,Roll)) FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d));
        DbResult rc = stmt.Step(); // rc will be BE_SQLITE_ERROR, and m_db->GetLastError() will return "Illegal input to DGN_bbox_areaxy"
        //__PUBLISH_EXTRACT_END__
        ASSERT_EQ( BE_SQLITE_ERROR , rc );
        BeTest::Log("SqlFunctionsTest", BeTest::PRIORITY_INFO, Utf8PrintfString("SQLite error: %s\n", m_db->GetLastError().c_str()).c_str()); // displays "SQLite error: Illegal input to DGN_bbox_areaxy"
        }

    //  The X-Y area is width (X) time depth (Y)
    double obstacleXyArea = obstacleAt0->GetPlacement().GetElementBox().GetWidth() * obstacleAt0->GetPlacement().GetElementBox().GetDepth();

    m_db->SaveChanges();

    //  Get the areas of the obstacles individually and sum them up 
    double totalAreaXy = 0.0;
        {
        Statement stmt;
        stmt.Prepare(*m_db, "SELECT DGN_bbox_areaxy(" EABB_FROM_GEOM ") FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d));

        DbResult rc;
        while (BE_SQLITE_ROW == (rc = stmt.Step()))
            {
            double areaxy = stmt.GetValueDouble(0);
            ASSERT_EQ( obstacleXyArea , areaxy );
            totalAreaXy += areaxy;
            }

        ASSERT_EQ( BE_SQLITE_DONE , rc ) << Utf8PrintfString("SQLite error: %s", m_db->GetLastError().c_str()).c_str();
        EXPECT_DOUBLE_EQ( 2*obstacleXyArea , totalAreaXy );
        }

    //  Compute the sum of the areas using SUM -- should get the same result
    if (true)
        {
        //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_bbox_areaxy_sum.sampleCode
        // This is an example of using DGN_placement_eabb to sum up element areas. Note that we must to use 
        // element-aligned bounding boxes in this query, rather than axis-aligned bounding boxes.
        Statement stmt;
        stmt.Prepare(*m_db, "SELECT SUM(DGN_bbox_areaxy(" EABB_FROM_GEOM ")) FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d));
        //__PUBLISH_EXTRACT_END__

        ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
        ASSERT_EQ( totalAreaXy , stmt.GetValueDouble(0) );
        }

    //  Do the same with a sub-selection
    if (true)
        {
        Statement stmt;
        stmt.Prepare(*m_db, "SELECT SUM(area) FROM (SELECT DGN_bbox_areaxy(" EABB_FROM_GEOM ") AS area FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d) ")");

        ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
        ASSERT_EQ( totalAreaXy , stmt.GetValueDouble(0) );
        }

    }

#define ANGLES_FROM_PLACEMENT "DGN_angles(g.Yaw,g.Pitch,g.Roll)"
#define EC_ANGLES_FROM_PLACEMENT "DGN_angles(g.Yaw,g.Pitch,g.Roll)"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, placement_angles)
    {
    SetupProject(L"placement_angles.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    double o1y = 5.0;
    double o2x = 5.0;
    DPoint3d o1origin = DPoint3d::From(0,o1y,0);
    DPoint3d o2origin = DPoint3d::From(o2x,0,0);

    //  Create an element @ 0 degrees
    ObstacleElementPtr elemAt0 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o1origin, 0.0, CreateCode("elemAt0"));
    InsertElement(*elemAt0);
    elemAt0->SetSomeProperty(*m_db, "B");

    //  Create an element @ 90 degrees
    ObstacleElementPtr elem1At90 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o2origin, 90.0, CreateCode("elem1At90"));
    InsertElement(*elem1At90);
    elem1At90->SetSomeProperty(*m_db, "A");

    //  Verify that only one is found with a placement angle of 90
    Statement stmt;
    stmt.Prepare(*m_db, "SELECT g.ElementId FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d) " AS g WHERE DGN_angles_maxdiff(" ANGLES_FROM_PLACEMENT ",DGN_Angles(90,0,0)) < 1.0");

    ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
    ASSERT_EQ( elem1At90->GetElementId() , stmt.GetValueId<DgnElementId>(0) );
    ASSERT_EQ( BE_SQLITE_DONE, stmt.Step() );

    if (true)
        {
        //  Do the same by checking the yaw angle directly
        Statement stmt2;
        //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_angles_value.sampleCode
        // This query uses DGN_angles_value to extract the Yaw angle of an element's placement, in order to compare it with 90.
        stmt2.Prepare(*m_db, "SELECT g.ElementId FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d) " AS g WHERE ABS(g.Yaw - 90) < 1.0");
        //__PUBLISH_EXTRACT_END__

        ASSERT_EQ( BE_SQLITE_ROW , stmt2.Step() );
        ASSERT_EQ( elem1At90->GetElementId() , stmt2.GetValueId<DgnElementId>(0) );
        ASSERT_EQ( BE_SQLITE_DONE, stmt2.Step() );
        }
    
    //  Create anoter element @ 90 degrees
    ObstacleElementPtr elem2At90 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o2origin, 90.0, CreateCode("elem2At90"));
    InsertElement(*elem2At90);
    elem2At90->SetSomeProperty(*m_db, "B");

    //  Verify that 2 are now found @ 90
    if (true)
        {
        DgnElementIdSet ids;
        stmt.Reset();
        ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
        ids.insert(stmt.GetValueId<DgnElementId>(0));
        ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
        ids.insert(stmt.GetValueId<DgnElementId>(0));
        ASSERT_EQ( BE_SQLITE_DONE, stmt.Step() );
        ASSERT_TRUE( ids.Contains(elem1At90->GetElementId())  );
        ASSERT_TRUE( ids.Contains(elem2At90->GetElementId())  );
        }

    //  Only one should be found with a placement angle of 0
    stmt.Finalize();
    stmt.Prepare(*m_db, "SELECT g.ElementId FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d) " AS g WHERE DGN_angles_maxdiff(" ANGLES_FROM_PLACEMENT ",DGN_Angles(0,0,0)) < 1.0");

    ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
    ASSERT_EQ( elemAt0->GetElementId() , stmt.GetValueId<DgnElementId>(0) );
    ASSERT_EQ( BE_SQLITE_DONE, stmt.Step() );

    //  Now add an additional where clause, so that we find only elem2At90
    //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_angles_maxdiff.sampleCode
    // This query looks for "Obstacles" that are oriented at 90 degrees in the X-Y plane.
    // It is an example of using DGN_angles_maxdiff to look for elements with a specific placement angle. 
    // This example also shows how to combine tests on geometry and business properties in a single query. 
    // This example uses ECSql.
    ECSqlStatement estmt;
    estmt.Prepare(*m_db, "SELECT o.ECInstanceId FROM " 
                            BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " AS g,"
                            DGN_SQL_TEST_SCHEMA_NAME "." DGN_SQL_TEST_OBSTACLE_CLASS " AS o"
                            " WHERE (o.ECInstanceId=g.ECInstanceId) AND (o.SomeProperty = 'B') AND (DGN_angles_maxdiff(" EC_ANGLES_FROM_PLACEMENT ",DGN_Angles(90.0,0,0)) < 1.0)");
    //__PUBLISH_EXTRACT_END__
    ASSERT_EQ(BE_SQLITE_ROW , estmt.Step());
    ASSERT_EQ( elem2At90->GetElementId() , estmt.GetValueId<DgnElementId>(0) );
    ASSERT_EQ(BE_SQLITE_DONE, estmt.Step());
    }

#define ORIGIN_FROM_PLACEMENT "DGN_point(g.Origin_X,g.Origin_Y,g.Origin_Z)"
#define BBOX_FROM_PLACEMENT "DGN_bbox(g.BBoxLow_X,g.BBoxLow_Y,g.BBoxLow_Z,g.BBoxHigh_X,g.BBoxHigh_Y,g.BBoxHigh_Z)"
#define PLACEMENT_FROM_GEOM "DGN_placement(" ORIGIN_FROM_PLACEMENT "," ANGLES_FROM_PLACEMENT "," BBOX_FROM_PLACEMENT ")"
#define AABB_FROM_PLACEMENT "DGN_placement_aabb(" PLACEMENT_FROM_GEOM ")"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, DGN_point_min_distance_to_bbox)
    {
    SetupProject(L"DGN_point_min_distance_to_bbox.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    double o1y = 2.0;
    double o2x = 5.0;
    DPoint3d o1origin = DPoint3d::From(0,o1y,0);
    DPoint3d o2origin = DPoint3d::From(o2x,0,0);

    DgnElementId r1, o1, o1a, o2, o2a;
        {
        RobotElementPtr robot1 = RobotElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, DPoint3d::From(0,0,0), 0.0, CreateCode("Robot1"));
        InsertElement(*robot1);

        ObstacleElementPtr obstacle1 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o1origin, 0.0, CreateCode("Obstacle1"));
        InsertElement(*obstacle1);
        obstacle1->SetTestUniqueAspect(*m_db, "SomeKindOfObstacle");

        ObstacleElementPtr obstacle1a = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o1origin, 0.0, CreateCode("Obstacle1a"));
        InsertElement(*obstacle1a);
        obstacle1a->SetTestUniqueAspect(*m_db, "SomeOtherKindOfObstacle");

        ObstacleElementPtr obstacle2 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o2origin, 90.0, CreateCode("Obstacle2"));
        InsertElement(*obstacle2);
        obstacle2->SetTestUniqueAspect(*m_db, "SomeKindOfObstacle");

        ObstacleElementPtr obstacle2a = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o2origin, 90.0, CreateCode("Obstacle2a"));
        InsertElement(*obstacle2a);
        obstacle2a->SetTestUniqueAspect(*m_db, "SomeOtherKindOfObstacle");

        // Double-check that we created the robots and obstacles, as expected
        ASSERT_EQ( RobotElement::QueryClassId(*m_db)    , robot1->GetElementClassId() );
        ASSERT_EQ( ObstacleElement::QueryClassId(*m_db) , obstacle1->GetElementClassId() );

        m_db->SaveChanges();

        r1 = robot1->GetElementId();
        o1 = obstacle1->GetElementId();
        o1a = obstacle1a->GetElementId();
        o2 = obstacle2->GetElementId();
        o2a = obstacle2a->GetElementId();
        }

    RobotElementCPtr robot1 = m_db->Elements().Get<RobotElement>(r1);
    ASSERT_TRUE( robot1.IsValid() );
        
    Statement stmt;
    stmt.Prepare(*m_db,        // aspect.sc01 == aspect.TestUniqueAspectProperty
        "SELECT aspect.ElementId, aspect.sc01 FROM " BIS_TABLE(BIS_CLASS_Element) " e, " BIS_TABLE(BIS_CLASS_ElementUniqueAspect) " aspect," BIS_TABLE(BIS_CLASS_GeometricElement3d) " g, " DGN_VTABLE_SpatialIndex " rt WHERE"
             " rt.ElementId MATCH DGN_spatial_overlap_aabb(:bbox)" // FROM R-Tree
             " AND g.ElementId=rt.ElementId"
             " AND DGN_point_min_distance_to_bbox(:testPoint, " AABB_FROM_PLACEMENT ") <= :maxDistance"  // select geoms that are within some distance of a specified point
             " AND e.Id=g.ElementId"
             " AND e.ECClassId=:ecClass"       //  select only Obstacles
             " AND aspect.ElementId=e.Id AND aspect.sc01=:propertyValue"       // ... with certain items
        );

    //  Initial placement
    //
    //  |                           ^
    //  |                           |
    //  |<---Obstacle1------->      Obstacle2
    //  |                           |
    //  |R1                         V
    //  +-- -- -- -- -- -- --
    if (true)
        {
        // Search for Obstacles that are 2 meters from the center of Robot1. We should find only Obstacle1
        stmt.Reset();
        stmt.ClearBindings();
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();
        AxisAlignedBox3d xbox = r1aabb;
        DPoint3d r1center = DPoint3d::FromInterpolate(r1aabb.low, 0.5, r1aabb.high);
        xbox.low.Add(DPoint3d::From(-3,-3,-3));
        xbox.high.Add(DPoint3d::From(3,3,3));
        stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &xbox, sizeof(xbox), Statement::MakeCopy::No);
        stmt.BindBlob(stmt.GetParameterIndex(":testPoint"), &r1center, sizeof(r1center), Statement::MakeCopy::No);
        stmt.BindDouble(stmt.GetParameterIndex(":maxDistance"), 2.0);
        stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "SomeKindOfObstacle", Statement::MakeCopy::No);

        DbResult rc=stmt.Step();
        ASSERT_EQ( BE_SQLITE_ROW , rc);

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1 , ofoundId );
        ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";

        // Expand the search to a distance of 5 meters, and we should find both obstacles
        stmt.Reset();
        stmt.ClearBindings();

        xbox.low.Add(DPoint3d::From(-3,-3,-3));
        xbox.high.Add(DPoint3d::From(3,3,3));
        stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &xbox, sizeof(xbox), Statement::MakeCopy::No);
        stmt.BindBlob(stmt.GetParameterIndex(":testPoint"), &r1center, sizeof(r1center), Statement::MakeCopy::No);
        stmt.BindDouble(stmt.GetParameterIndex(":maxDistance"), 5.0);
        stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "SomeKindOfObstacle", Statement::MakeCopy::No);
        
        DgnElementIdSet hits;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            hits.insert(stmt.GetValueId<DgnElementId>(0));
            ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );
            }
        ASSERT_EQ( 2 , hits.size() );
        ASSERT_TRUE( hits.Contains(o1) );
        ASSERT_TRUE( hits.Contains(o2) );

        // Narrow the search to 1 meter, and we should get no hits.
        stmt.Reset();
        stmt.ClearBindings();

        stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &xbox, sizeof(xbox), Statement::MakeCopy::No);
        stmt.BindBlob(stmt.GetParameterIndex(":testPoint"), &r1center, sizeof(r1center), Statement::MakeCopy::No);
        stmt.BindDouble(stmt.GetParameterIndex(":maxDistance"), 1.0);
        stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "SomeKindOfObstacle", Statement::MakeCopy::No);
        
        ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() );
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, spatialQueryECSql)
    {
    SetupProject(L"spatialQueryECSql.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    double o1y = 5.0;
    double o2x = 5.0;
    DPoint3d o1origin = DPoint3d::From(0,o1y,0);
    DPoint3d o2origin = DPoint3d::From(o2x,0,0);

    DgnElementId r1, o1, o1a, o2, o2a;
        {
        RobotElementPtr robot1 = RobotElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, DPoint3d::From(0,0,0), 0.0, CreateCode("Robot1"));
        InsertElement(*robot1);

        ObstacleElementPtr obstacle1 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o1origin, 0.0, CreateCode("Obstacle1"));
        InsertElement(*obstacle1);
        obstacle1->SetTestUniqueAspect(*m_db, "SomeKindOfObstacle");

        ObstacleElementPtr obstacle1a = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o1origin, 0.0, CreateCode("Obstacle1a"));
        InsertElement(*obstacle1a);
        obstacle1a->SetTestUniqueAspect(*m_db, "SomeOtherKindOfObstacle");

        ObstacleElementPtr obstacle2 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o2origin, 90.0, CreateCode("Obstacle2"));
        InsertElement(*obstacle2);
        obstacle2->SetTestUniqueAspect(*m_db, "SomeKindOfObstacle");

        ObstacleElementPtr obstacle2a = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o2origin, 90.0, CreateCode("Obstacle2a"));
        InsertElement(*obstacle2a);
        obstacle2a->SetTestUniqueAspect(*m_db, "SomeOtherKindOfObstacle");

        // Double-check that we created the robots and obstacles, as expected
        ASSERT_EQ( RobotElement::QueryClassId(*m_db)    , robot1->GetElementClassId() );
        ASSERT_EQ( ObstacleElement::QueryClassId(*m_db) , obstacle1->GetElementClassId() );

        m_db->SaveChanges();

        r1 = robot1->GetElementId();
        o1 = obstacle1->GetElementId();
        o1a = obstacle1a->GetElementId();
        o2 = obstacle2->GetElementId();
        o2a = obstacle2a->GetElementId();
        }

        RobotElementCPtr robot1 = m_db->Elements().Get<RobotElement>(r1);

    ECSqlStatement stmt;
    stmt.Prepare(*m_db, 
        "SELECT TestUniqueAspect.ElementId,TestUniqueAspect.TestUniqueAspectProperty FROM " BIS_SCHEMA(BIS_CLASS_SpatialIndex) " rt,DgnPlatformTest.Obstacle,DgnPlatformTest.TestUniqueAspect"
            " WHERE rt.ECInstanceId MATCH DGN_spatial_overlap_aabb(:bbox)"
            " AND Obstacle.ECInstanceId=rt.ECInstanceId"
            " AND TestUniqueAspect.ElementId=rt.ECInstanceId AND TestUniqueAspect.TestUniqueAspectProperty = :propertyValue"
        );

    //  Make sure that the range tree index is used (first) and that other tables are indexed (after)
    Utf8CP sql = stmt.GetNativeSql();
    Utf8String qplan = m_db->ExplainQuery(sql);
    auto scanRt = qplan.find("SCAN TABLE " DGN_VTABLE_SpatialIndex " VIRTUAL TABLE INDEX");
    //auto searchItem = qplan.find("SEARCH TABLE dptest_TestUniqueAspect USING INTEGER PRIMARY KEY");
    //auto searchElement = qplan.find("SEARCH TABLE dgn_Element USING COVERING INDEX"); WIP: removing ElementItem has changed the query plan, but not convinced that is an actual problem
    ASSERT_NE(Utf8String::npos, scanRt) << "Unexpected query plan for SQL " << sql << ". Actual query plan: " << qplan.c_str();
    //ASSERT_NE(Utf8String::npos , searchItem ) << "Unexpected query plan for SQL " << sql << ". Actual query plan: " << qplan.c_str();
    //ASSERT_NE(Utf8String::npos , searchElement ) << "Unexpected query plan for SQL " << sql << ". Actual query plan: " << qplan.c_str();
    //ASSERT_LT(scanRt , searchItem) << "Unexpected query plan for SQL " << sql << ". Actual query plan: " << qplan.c_str();
    //ASSERT_LT(scanRt , searchElement) << "Unexpected query plan for SQL " << sql << ". Actual query plan: " << qplan.c_str();

    //  Initial placement: Robot1 is not touching any obstacle
    //
    //  |
    //  |<---Obstacle1------->
    //  |                   ^
    //  |                   |
    //  |                   Obstacle2
    //  |                   |
    //  |R1                 V
    //  +-- -- -- -- -- -- --
    if (true)
        {
        stmt.Reset();
        stmt.ClearBindings();
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();
        stmt.BindBinary(stmt.GetParameterIndex("bbox"), &r1aabb, sizeof(r1aabb), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        stmt.BindText(stmt.GetParameterIndex("propertyValue"), "SomeKindOfObstacle", BeSQLite::EC::IECSqlBinder::MakeCopy::No);

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "\r\nTranslated SQL: " << sql;
        }

    //  Move Robot1 up, so that it touches Obstacle1 but not Obstacle2
    //
    //  |
    //  |<---Obstacle1------->
    //  |R1                 ^
    //  |                   |
    //  |                   Obstacle2
    //  |                   |
    //  |                   V
    //  +-- -- -- -- -- -- --
    if (true)
        {
        ObstacleElementCPtr obstacle1 = m_db->Elements().Get<ObstacleElement>(o1);

        RobotElementPtr robot1CC = m_db->Elements().GetForEdit<RobotElement>(r1);
        ASSERT_TRUE( robot1CC.IsValid() );
        robot1CC->Translate(DVec3d::FromStartEnd(robot1CC->GetPlacement().GetOrigin(), obstacle1->GetPlacement().GetOrigin()));
        ASSERT_TRUE( m_db->Elements().Update(*robot1CC).IsValid() );

        stmt.Reset();
        stmt.ClearBindings();
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBinary(stmt.GetParameterIndex("bbox"), &r1aabb, sizeof(r1aabb), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        stmt.BindText(stmt.GetParameterIndex("propertyValue"), "SomeKindOfObstacle", BeSQLite::EC::IECSqlBinder::MakeCopy::No);

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step() );

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1 , ofoundId );
        ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ(BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";

        //  Query for the other kind of obstacle
        stmt.Reset();
        stmt.ClearBindings();
        r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBinary(stmt.GetParameterIndex("bbox"), &r1aabb, sizeof(r1aabb), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        stmt.BindText(stmt.GetParameterIndex("propertyValue"), "SomeOtherKindOfObstacle", BeSQLite::EC::IECSqlBinder::MakeCopy::No);

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step() );

        ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1a , ofoundId );
        ASSERT_STREQ( "SomeOtherKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ(BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";

        //  Query should fail if I specify an item that is not found on any kind of obstacle
        stmt.Reset();
        stmt.ClearBindings();
        r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBinary(stmt.GetParameterIndex("bbox"), &r1aabb, sizeof(r1aabb), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        stmt.BindText(stmt.GetParameterIndex("propertyValue"), "no kind of obstacle", BeSQLite::EC::IECSqlBinder::MakeCopy::No);

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

    //  Move Robot1 over, so that it touches Obstacle2 but not Obstacle1
    //
    //  |
    //  |<---Obstacle1------->
    //  |                   ^
    //  |                   |
    //  |                   Obstacle2
    //  |                   |
    //  |                R1 V
    //  +-- -- -- -- -- -- --
    if (true)
        {
        ObstacleElementCPtr obstacle2 = m_db->Elements().Get<ObstacleElement>(o2);

        RobotElementPtr robot1CC = m_db->Elements().GetForEdit<RobotElement>(r1);
        ASSERT_TRUE( robot1CC.IsValid() );
        robot1CC->Translate(DVec3d::FromStartEnd(robot1CC->GetPlacement().GetOrigin(), obstacle2->GetPlacement().GetOrigin()));
        ASSERT_TRUE( m_db->Elements().Update(*robot1CC).IsValid() );

        stmt.Reset();
        stmt.ClearBindings();
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBinary(stmt.GetParameterIndex("bbox"), &r1aabb, sizeof(r1aabb), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        stmt.BindText(stmt.GetParameterIndex("propertyValue"), "SomeKindOfObstacle", BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step() );

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o2 , ofoundId );
        ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ(BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";
        }

    //  Move Robot1 so that it touches both Obstacle1 and Obstacle1
    //
    //  |
    //  |<---Obstacle1------->
    //  |                R1 ^
    //  |                   |
    //  |                   Obstacle2
    //  |                   |
    //  |                   V
    //  +-- -- -- -- -- -- --
    if (true)
        {
        RobotElementPtr robot1CC = m_db->Elements().GetForEdit<RobotElement>(r1);
        ASSERT_TRUE( robot1CC.IsValid() );
        robot1CC->Translate(DVec3d::FromStartEnd(robot1CC->GetPlacement().GetOrigin(), DPoint3d::From(o2x,o1y,0)));
        ASSERT_TRUE( m_db->Elements().Update(*robot1CC).IsValid() );

        DgnElementIdSet overlaps;
        stmt.Reset();
        stmt.ClearBindings();
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBinary(stmt.GetParameterIndex("bbox"), &r1aabb, sizeof(r1aabb), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        stmt.BindText(stmt.GetParameterIndex("propertyValue"), "SomeKindOfObstacle", BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        while (BE_SQLITE_ROW == stmt.Step())
            {
            overlaps.insert(stmt.GetValueId<DgnElementId>(0));
            ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );
            }
        ASSERT_EQ( 2 , overlaps.size() );
        ASSERT_TRUE( overlaps.Contains(o1) );
        ASSERT_TRUE( overlaps.Contains(o2) );
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, spatialQuery)
    {
    SetupProject(L"spatialQuery.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    double o1y = 5.0;
    double o2x = 5.0;
    DPoint3d o1origin = DPoint3d::From(0,o1y,0);
    DPoint3d o2origin = DPoint3d::From(o2x,0,0);

    DgnElementId r1, o1, o1a, o2, o2a;
        {
        RobotElementPtr robot1 = RobotElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, DPoint3d::From(0,0,0), 0.0, CreateCode("Robot1"));
        InsertElement(*robot1);

        ObstacleElementPtr obstacle1 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o1origin, 0.0, CreateCode("Obstacle1"));
        InsertElement(*obstacle1);
        obstacle1->SetTestUniqueAspect(*m_db, "SomeKindOfObstacle");

        ObstacleElementPtr obstacle1a = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o1origin, 0.0, CreateCode("Obstacle1a"));
        InsertElement(*obstacle1a);
        obstacle1a->SetTestUniqueAspect(*m_db, "SomeOtherKindOfObstacle");

        ObstacleElementPtr obstacle2 = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o2origin, 90.0, CreateCode("Obstacle2"));
        InsertElement(*obstacle2);
        obstacle2->SetTestUniqueAspect(*m_db, "SomeKindOfObstacle");

        ObstacleElementPtr obstacle2a = ObstacleElement::Create(*GetDefaultSpatialModel(), m_defaultCategoryId, o2origin, 90.0, CreateCode("Obstacle2a"));
        InsertElement(*obstacle2a);
        obstacle2a->SetTestUniqueAspect(*m_db, "SomeOtherKindOfObstacle");

        // Double-check that we created the robots and obstacles, as expected
        ASSERT_EQ( RobotElement::QueryClassId(*m_db)    , robot1->GetElementClassId() );
        ASSERT_EQ( ObstacleElement::QueryClassId(*m_db) , obstacle1->GetElementClassId() );

        m_db->SaveChanges();

        r1 = robot1->GetElementId();
        o1 = obstacle1->GetElementId();
        o1a = obstacle1a->GetElementId();
        o2 = obstacle2->GetElementId();
        o2a = obstacle2a->GetElementId();
        }

    //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_spatial_overlap_aabb.sampleCode
    // This query uses DGN_spatial_overlap_aabb to find elements whose range overlaps the argument :bbox and are of class :ecClass and have
    // item property = :propertyValue.
    Statement stmt;
    stmt.Prepare(*m_db,       // aspect.sc01 == aspect.TestUniqueAspectProperty
        "SELECT aspect.ElementId,aspect.sc01 FROM " DGN_VTABLE_SpatialIndex " rt," BIS_TABLE(BIS_CLASS_Element) " e," BIS_TABLE(BIS_CLASS_ElementUniqueAspect) " aspect WHERE"
           " rt.ElementId MATCH DGN_spatial_overlap_aabb(:bbox)"      // select elements whose range overlaps box
           " AND e.Id=rt.ElementId AND e.ECClassId=:ecClass"        // and are of a specific ecClass 
           " AND aspect.ElementId=e.Id AND aspect.sc01=:propertyValue"   // ... with certain item value
        );

    RobotElementCPtr robot1 = m_db->Elements().Get<RobotElement>(r1);
    AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();
    stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &r1aabb, sizeof(r1aabb), Statement::MakeCopy::No);
    stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
    stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "SomeKindOfObstacle", Statement::MakeCopy::No);
    //__PUBLISH_EXTRACT_END__
    
    //  Initial placement
    //
    //  |
    //  |<---Obstacle1------->
    //  |                   ^
    //  |                   |
    //  |                   Obstacle2
    //  |                   |
    //  |R1                 V
    //  +-- -- -- -- -- -- --
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    //  Move Robot1 up, so that it touches Obstacle1 but not Obstacle2
    //
    //  |
    //  |<---Obstacle1------->
    //  |R1                 ^
    //  |                   |
    //  |                   Obstacle2
    //  |                   |
    //  |                   V
    //  +-- -- -- -- -- -- --
    if (true)
        {
        ObstacleElementCPtr obstacle1 = m_db->Elements().Get<ObstacleElement>(o1);

        RobotElementPtr robot1CC = m_db->Elements().GetForEdit<RobotElement>(r1);
        ASSERT_TRUE( robot1CC.IsValid() );
        robot1CC->Translate(DVec3d::FromStartEnd(robot1CC->GetPlacement().GetOrigin(), obstacle1->GetPlacement().GetOrigin()));
        ASSERT_TRUE( m_db->Elements().Update(*robot1CC).IsValid() );

        stmt.Reset();
        stmt.ClearBindings();
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &r1aabb, sizeof(r1aabb), Statement::MakeCopy::No);
        stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "SomeKindOfObstacle", Statement::MakeCopy::No);

        ASSERT_EQ( BE_SQLITE_ROW, stmt.Step() );

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1 , ofoundId );
        ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";

        //  Query for the other kind of obstacle
        stmt.Reset();
        stmt.ClearBindings();
        r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &r1aabb, sizeof(r1aabb), Statement::MakeCopy::No);
        stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "SomeOtherKindOfObstacle", Statement::MakeCopy::No);

        ASSERT_EQ( BE_SQLITE_ROW, stmt.Step() );

        ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1a , ofoundId );
        ASSERT_STREQ( "SomeOtherKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";

        //  Query should fail if I specify an item that is not found on any kind of obstacle
        stmt.Reset();
        stmt.ClearBindings();
        r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &r1aabb, sizeof(r1aabb), Statement::MakeCopy::No);
        stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "no kind of obstacle", Statement::MakeCopy::No);

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

    //  Move Robot1 over, so that it touches Obstacle2 but not Obstacle1
    //
    //  |
    //  |<---Obstacle1------->
    //  |                   ^
    //  |                   |
    //  |                   Obstacle2
    //  |                   |
    //  |                R1 V
    //  +-- -- -- -- -- -- --
    if (true)
        {
        ObstacleElementCPtr obstacle2 = m_db->Elements().Get<ObstacleElement>(o2);

        RobotElementPtr robot1CC = m_db->Elements().GetForEdit<RobotElement>(r1);
        ASSERT_TRUE( robot1CC.IsValid() );
        robot1CC->Translate(DVec3d::FromStartEnd(robot1CC->GetPlacement().GetOrigin(), obstacle2->GetPlacement().GetOrigin()));
        ASSERT_TRUE( m_db->Elements().Update(*robot1CC).IsValid() );

        stmt.Reset();
        stmt.ClearBindings();
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &r1aabb, sizeof(r1aabb), Statement::MakeCopy::No);
        stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "SomeKindOfObstacle", Statement::MakeCopy::No);
        ASSERT_EQ( BE_SQLITE_ROW, stmt.Step() );

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o2 , ofoundId );
        ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";
        }

    //  Move Robot1 so that it touches both Obstacle1 and Obstacle1
    //
    //  |
    //  |<---Obstacle1------->
    //  |                R1 ^
    //  |                   |
    //  |                   Obstacle2
    //  |                   |
    //  |                   V
    //  +-- -- -- -- -- -- --
    if (true)
        {
        RobotElementPtr robot1CC = m_db->Elements().GetForEdit<RobotElement>(r1);
        ASSERT_TRUE( robot1CC.IsValid() );
        robot1CC->Translate(DVec3d::FromStartEnd(robot1CC->GetPlacement().GetOrigin(), DPoint3d::From(o2x,o1y,0)));
        ASSERT_TRUE( m_db->Elements().Update(*robot1CC).IsValid() );

        DgnElementIdSet overlaps;
        stmt.Reset();
        stmt.ClearBindings();
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &r1aabb, sizeof(r1aabb), Statement::MakeCopy::No);
        stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "SomeKindOfObstacle", Statement::MakeCopy::No);
        while (BE_SQLITE_ROW == stmt.Step())
            {
            overlaps.insert(stmt.GetValueId<DgnElementId>(0));
            ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );
            }
        ASSERT_EQ( 2 , overlaps.size() );
        ASSERT_TRUE( overlaps.Contains(o1) );
        ASSERT_TRUE( overlaps.Contains(o2) );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, bbox_union)
    {
    DgnDbTestDgnManager tdm(L"04_Plant.i.ibim", __FILE__, Db::OpenMode::Readonly, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnDbP dgndb = tdm.GetDgnProjectP();

    Statement stmt;
    //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_bbox_union.sampleCode
    // This is an example of accumlating the union of bounding boxes.
    // Note that when computing a union, it only makes sense to use axis-aligned bounding boxes, not element-aligned bounding boxes.
    stmt.Prepare(*dgndb, "SELECT DGN_bbox_union(" AABB_FROM_PLACEMENT ") FROM " BIS_TABLE(BIS_CLASS_Element) " AS e," BIS_TABLE(BIS_CLASS_GeometricElement3d) 
                    " AS g WHERE e.ModelId=65 AND e.id=g.ElementId");
    //__PUBLISH_EXTRACT_END__
    auto rc = stmt.Step();
    ASSERT_EQ(BE_SQLITE_ROW, rc);

    int resultSize = stmt.GetColumnBytes(0);
    ASSERT_EQ(sizeof(DRange3d), resultSize);
    BoundingBox3dCR result = *(BoundingBox3dCP)stmt.GetValueBlob(0);
    ASSERT_TRUE(result.IsValid());

    stmt.Finalize();
#ifdef NEEDSWORK_PLACEMENT_STRUCT
    //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_angles.sampleCode
    // An example of constructing a DGN_Angles object in order to test the placement angles of elements in the Db.
    Utf8CP anglesSql = "SELECT count(*) FROM (SELECT Placement FROM "
        BIS_TABLE(BIS_CLASS_GeometricElement3d)
        " WHERE Placement IS NOT NULL) WHERE DGN_angles_maxdiff(DGN_placement_angles(Placement),DGN_Angles(0,0,90)) < 1.0";

    stmt.Prepare(*dgndb, anglesSql);
    //__PUBLISH_EXTRACT_END__

    rc = stmt.Step();
    ASSERT_EQ(BE_SQLITE_ROW, rc);
    int count = stmt.GetValueInt(0);
    ASSERT_NE(count, 0);

    stmt.Finalize();
#else
    int count = 0;
#endif
    stmt.Prepare(*dgndb, "SELECT count(*) FROM "
                         BIS_TABLE(BIS_CLASS_GeometricElement3d) " AS g WHERE g.Roll < 90");

    rc = stmt.Step();
    ASSERT_EQ(BE_SQLITE_ROW, rc);
    count = stmt.GetValueInt(0);
    ASSERT_NE(count, 0);
    }

