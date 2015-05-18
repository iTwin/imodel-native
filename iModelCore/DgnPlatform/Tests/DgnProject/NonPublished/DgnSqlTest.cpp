/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnSqlTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include "DgnSqlTestDomain.h"

USING_NAMESPACE_BENTLEY_SQLITE 
using namespace DgnSqlTestNamespace;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct SqlFunctionsTest : public ::testing::Test
{
public:
    ScopedDgnHost m_host;
    DgnDbPtr      m_db;
    DgnModelId    m_defaultModelId;
    DgnCategoryId m_defaultCategoryId;

    
    void SetUp() override;
    void TearDown() override;

    void SetupProject (WCharCP dgnDbFileName, WCharCP inFileName, BeSQLite::Db::OpenMode);
    void InsertElement(PhysicalElementR pelem);
    DgnModelR GetDefaultModel() {return *m_db->Models().GetModel(m_defaultModelId);}
    PhysicalModelP GetDefaultPhysicalModel() {return dynamic_cast<PhysicalModelP>(&GetDefaultModel());}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlFunctionsTest::SetUp()
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlFunctionsTest::TearDown()
    {
    if (m_db.IsValid())
        m_db->GetTxnManager().Deactivate(); // finalizes TxnManager's prepared statements
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlFunctionsTest::SetupProject (WCharCP dgnDbFileName, WCharCP inFileName, BeSQLite::Db::OpenMode mode)
    {
    BeFileName fullDgnDbFileName;
    ASSERT_EQ (SUCCESS, DgnDbTestDgnManager::GetTestDataOut (fullDgnDbFileName, dgnDbFileName, inFileName, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb (&result, fullDgnDbFileName, DgnDb::OpenParams(mode));
    ASSERT_TRUE (m_db.IsValid());
    ASSERT_TRUE( result == BE_SQLITE_OK);

    DgnSqlTestDomain::RegisterDomainAndImportSchema(*m_db, T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelP defaultModel = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_NE( nullptr , defaultModel );
    GetDefaultModel().FillModel();
    
    m_defaultCategoryId = m_db->Categories().MakeIterator().begin().GetCategoryId();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlFunctionsTest::InsertElement(PhysicalElementR pelem)
    {
    ASSERT_TRUE( m_db->Elements().Insert(pelem).IsValid() );
    ASSERT_TRUE( pelem.GetElementId().IsValid() ) << L"Insert is supposed to set the ElementId";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, placement_areaxy)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"placement_areaxy.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    double o1y = 5.0;
    double o2x = 5.0;
    DPoint3d o1origin = DPoint3d::From(0,o1y,0);
    DPoint3d o2origin = DPoint3d::From(o2x,0,0);

    ObstacleElementPtr obstacleAt0 = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o1origin, 0.0, "obstacleAt0");
    InsertElement(*obstacleAt0);
    obstacleAt0->SetSomeProperty(*m_db, "B");

    ObstacleElementPtr obstacle2At90 = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o2origin, 90.0, "obstacle2At90");
    InsertElement(*obstacle2At90);
    obstacle2At90->SetSomeProperty(*m_db, "A");

    // Example of passing the wrong object to a SQL function
    if (true)
        {
        //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_bbox_areaxy_error.sampleCode
        //  This statement is wrong, because DGN_placement_angles returns a DGN_angles object, while DGN_bbox_areaxy expects a DGN_bbox object.
        Statement stmt;
        stmt.Prepare(*m_db, "SELECT DGN_bbox_areaxy(DGN_placement_angles(Placement)) FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom));
        DbResult rc = stmt.Step();
        ASSERT_EQ( BE_SQLITE_ERROR , rc );
        printf ("SQLite error: %s", m_db->GetLastError()); // displays "SQLite error: Illegal input to DGN_bbox_areaxy"
        //__PUBLISH_EXTRACT_END__
        }

    //  The X-Y area is width (X) time depth (Y)
    double obstacleXyArea = obstacleAt0->GetPlacement().GetElementBox().GetWidth() * obstacleAt0->GetPlacement().GetElementBox().GetDepth();

    m_db->SaveChanges();

    //  Get the areas of the obstacles individually and sum them up 
    double totalAreaXy = 0.0;
        {
        Statement stmt;
        stmt.Prepare(*m_db, "SELECT DGN_bbox_areaxy(DGN_placement_eabb(Placement)) FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom));

        DbResult rc;
        while (BE_SQLITE_ROW == (rc = stmt.Step()))
            {
            double areaxy = stmt.GetValueDouble(0);
            ASSERT_EQ( obstacleXyArea , areaxy );
            totalAreaXy += areaxy;
            }

        ASSERT_EQ( BE_SQLITE_DONE , rc ) << (Utf8CP)Utf8PrintfString("SQLite error: %s", m_db->GetLastError());
        EXPECT_DOUBLE_EQ( 2*obstacleXyArea , totalAreaXy );
        }

    //  Compute the sum of the areas using SUM -- should get the same result
    if (true)
        {
        //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_bbox_areaxy_sum.sampleCode
        Statement stmt;
        stmt.Prepare(*m_db, "SELECT SUM(DGN_bbox_areaxy(DGN_placement_eabb(Placement))) FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom));
        //__PUBLISH_EXTRACT_END__

        ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
        ASSERT_EQ( totalAreaXy , stmt.GetValueDouble(0) );
        }

    //  Do the same with a sub-selection
    if (true)
        {
        Statement stmt;
        stmt.Prepare(*m_db, "SELECT SUM(area) FROM (SELECT DGN_bbox_areaxy(DGN_placement_eabb(Placement)) AS area FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) ")");

        ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
        ASSERT_EQ( totalAreaXy , stmt.GetValueDouble(0) );
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, placement_angles)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"placement_angles.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    double o1y = 5.0;
    double o2x = 5.0;
    DPoint3d o1origin = DPoint3d::From(0,o1y,0);
    DPoint3d o2origin = DPoint3d::From(o2x,0,0);

    //  Create an element @ 0 degrees
    ObstacleElementPtr elemAt0 = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o1origin, 0.0, "elemAt0");
    InsertElement(*elemAt0);
    elemAt0->SetSomeProperty(*m_db, "B");

    //  Create an element @ 90 degrees
    ObstacleElementPtr elem1At90 = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o2origin, 90.0, "elem1At90");
    InsertElement(*elem1At90);
    elem1At90->SetSomeProperty(*m_db, "A");

    //  Verify that only one is found with a placement angle of 90
    Statement stmt;
    stmt.Prepare(*m_db, "SELECT g.ElementId FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " AS g WHERE DGN_angles_maxdiff(DGN_placement_angles(g.Placement),DGN_Angles(90,0,0)) < 1.0");

    ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
    ASSERT_EQ( elem1At90->GetElementId() , stmt.GetValueId<DgnElementId>(0) );
    ASSERT_EQ( BE_SQLITE_DONE, stmt.Step() );

    if (true)
        {
        //  Do the same by checking the yaw angle directly
        Statement stmt2;
        //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_angles_value.sampleCode
        stmt2.Prepare(*m_db, "SELECT g.ElementId FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " AS g WHERE ABS(DGN_angles_value(DGN_placement_angles(g.Placement), 0) - 90) < 1.0");
        //__PUBLISH_EXTRACT_END__

        ASSERT_EQ( BE_SQLITE_ROW , stmt2.Step() );
        ASSERT_EQ( elem1At90->GetElementId() , stmt2.GetValueId<DgnElementId>(0) );
        ASSERT_EQ( BE_SQLITE_DONE, stmt2.Step() );
        }
    
    //  Create anoter element @ 90 degrees
    ObstacleElementPtr elem2At90 = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o2origin, 90.0, "elem2At90");
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
    stmt.Prepare(*m_db, "SELECT g.ElementId FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " AS g WHERE DGN_angles_maxdiff(DGN_placement_angles(g.Placement),DGN_Angles(0,0,0)) < 1.0");

    ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
    ASSERT_EQ( elemAt0->GetElementId() , stmt.GetValueId<DgnElementId>(0) );
    ASSERT_EQ( BE_SQLITE_DONE, stmt.Step() );

    //  Now add an additional where clause, so that we find only elem2At90
#ifndef DONT_USE_ECSQL
    if (true)
        {
        //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_angles_maxdiff.sampleCode
        ECSqlStatement estmt;
        estmt.Prepare(*m_db, "SELECT o.ECInstanceId FROM " 
                                DGN_SCHEMA(DGN_CLASSNAME_ElementGeom) " AS g,"
                                DGN_SQL_TEST_SCHEMA_NAME "." DGN_SQL_TEST_OBSTACLE_CLASS " AS o"
                             " WHERE (o.ECInstanceId=g.ECInstanceId) AND (o.SomeProperty = 'B') AND (DGN_angles_maxdiff(DGN_placement_angles(g.Placement),DGN_Angles(90.0,0,0)) < 1.0)");
        //__PUBLISH_EXTRACT_END__
        ASSERT_EQ( EC::ECSqlStepStatus::HasRow , estmt.Step() );
        ASSERT_EQ( elem2At90->GetElementId() , estmt.GetValueId<DgnElementId>(0) );
        ASSERT_EQ( EC::ECSqlStepStatus::Done, estmt.Step() );
        }
#else
    if (true)
        {
        Statement estmt;
        estmt.Prepare(*m_db, "SELECT o.Id FROM " 
                                DGN_TABLE(DGN_CLASSNAME_ElementGeom) " AS g,"
                                DGN_TABLE(DGN_CLASSNAME_Element) " AS o"
                             " WHERE (o.Id=g.ElementId) AND (o.SomeProperty = 'B') AND (DGN_angles_maxdiff(DGN_placement_angles(g.Placement),DGN_Angles(90.0,0,0)) < 1.0)");
        ASSERT_EQ( BE_SQLITE_ROW , estmt.Step() );
        ASSERT_EQ( elem2At90->GetElementId() , estmt.GetValueId<DgnElementId>(0) );
        ASSERT_EQ( BE_SQLITE_DONE, estmt.Step() );
        }
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, spatialQuery)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"spatialQuery.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    double o1y = 5.0;
    double o2x = 5.0;
    DPoint3d o1origin = DPoint3d::From(0,o1y,0);
    DPoint3d o2origin = DPoint3d::From(o2x,0,0);

    DgnElementId r1, o1, o1a, o2, o2a;
        {
        RobotElementPtr robot1 = RobotElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, DPoint3d::From(0,0,0), 0.0, "Robot1");
        InsertElement(*robot1);

        ObstacleElementPtr obstacle1 = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o1origin, 0.0, "Obstacle1");
        InsertElement(*obstacle1);
        obstacle1->SetTestItem(*m_db, "SomeKindOfObstacle");

        ObstacleElementPtr obstacle1a = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o1origin, 0.0, "Obstacle1a");
        InsertElement(*obstacle1a);
        obstacle1a->SetTestItem(*m_db, "SomeOtherKindOfObstacle");

        ObstacleElementPtr obstacle2 = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o2origin, 90.0, "Obstacle2");
        InsertElement(*obstacle2);
        obstacle2->SetTestItem(*m_db, "SomeKindOfObstacle");

        ObstacleElementPtr obstacle2a = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o2origin, 90.0, "Obstacle2a");
        InsertElement(*obstacle2a);
        obstacle2a->SetTestItem(*m_db, "SomeOtherKindOfObstacle");

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


#ifdef CANT_USE_ECSQL // 1. ?n bindings are not supported, 2. dgn_PrjRTree is not in the ecschema
    ECSqlStatement stmt;
    stmt.Prepare(*m_db, 
        "SELECT item.ECInstanceId, item.TestItemProperty FROM"
        "  (SELECT g.ECInstanceId FROM"
        "     (SELECT rt.ElementId FROM dgn_PrjRTree rt "                           // FROM R-Tree
        "        WHERE rt.MaxX >= ?1 AND rt.MinX <= ?4"                             //  select Elements in nodes that overlap bbox
        "          AND rt.MaxY >= ?2 AND rt.MinY <= ?5"
        "          AND rt.MaxZ >= ?3 AND rt.MinZ <= ?6) rt_res"                     //  (call this result set "rt_res")
        "     ,"
        "      dgn.ElementGeom g WHERE g.ECInstanceId=rt_res.ECInstanceId"      // JOIN ElementGeom
        "          AND DGN_bbox_overlaps(DGN_placement_aabb(g.Placement), DGN_bbox(?1, ?2, ?3, ?4, ?5, ?6))"  // select geoms that overlap bbox
        "  ) geom_res"                                                          //   (call this result set "geom_res")
        "  ,"
        "  DgnPlatformTest.Obstacle obstacle,"                                  // JOIN Obstacle
        "  DgnPlatformTest.TestItem item "                                      // JOIN TestItem
        "       WHERE obstacle.ECInstanceId = geom_res.ECInstanceId"            //      only Obstacles
        "        AND item.Elementid = obstacle.ECInstanceId AND item.TestItemProperty = ?7" //    ... with certain items
        );
#else
    Utf8CP sql = 
        "SELECT item.ElementId, item.x01 FROM"
        "  (SELECT g.ElementId FROM"
        "     (SELECT rt.ElementId FROM dgn_PrjRTree rt "                       //          FROM R-Tree
        "        WHERE rt.MaxX >= ?1 AND rt.MinX <= ?4"                         //           select Elements in nodes that overlap bbox
        "          AND rt.MaxY >= ?2 AND rt.MinY <= ?5"
        "          AND rt.MaxZ >= ?3 AND rt.MinZ <= ?6) rt_res"                 //           (call this result set "rt_res")
        "     ,"
        "      dgn_ElementGeom g WHERE g.ElementId=rt_res.ElementId"            //      JOIN ElementGeom
        "          AND DGN_bbox_overlaps(DGN_placement_aabb(g.Placement), DGN_bbox(?1, ?2, ?3, ?4, ?5, ?6))"  // select geoms that overlap bbox
        "  ) geom_res"                                                          //      (call this result set "geom_res")
        "  , dgn_Element e"                                                     // JOIN Element
        "  , dgn_ElementItem item "                                             // JOIN ElementItem
        "       WHERE e.Id = geom_res.ElementId AND e.ECClassId=?7"             //  select only Obstacles
        "        AND item.ElementId = e.Id AND item.x01 = ?8"                  //                     ... with certain items
    ;

    //printf ("%s\n", m_db->ExplainQueryPlan(nullptr, sql));

    Statement stmt;
    stmt.Prepare(*m_db, sql);

#endif

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
    if (true)
        {
        stmt.Reset();
        stmt.ClearBindings();
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();
        stmt.BindDouble(1, r1aabb.low.x);
        stmt.BindDouble(2, r1aabb.low.y);
        stmt.BindDouble(3, r1aabb.low.z);
        stmt.BindDouble(4, r1aabb.high.x);
        stmt.BindDouble(5, r1aabb.high.y);
        stmt.BindDouble(6, r1aabb.high.z);
        stmt.BindId(7, ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(8, "SomeKindOfObstacle", Statement::MakeCopy::No);

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
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
        stmt.BindDouble(1, r1aabb.low.x);
        stmt.BindDouble(2, r1aabb.low.y);
        stmt.BindDouble(3, r1aabb.low.z);
        stmt.BindDouble(4, r1aabb.high.x);
        stmt.BindDouble(5, r1aabb.high.y);
        stmt.BindDouble(6, r1aabb.high.z);
        stmt.BindId(7, ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(8, "SomeKindOfObstacle", Statement::MakeCopy::No);

        ASSERT_EQ( BE_SQLITE_ROW, stmt.Step() );

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1 , ofoundId );
        ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";

        //  Query for the other kind of obstacle
        stmt.Reset();
        stmt.ClearBindings();
        r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindDouble(1, r1aabb.low.x);
        stmt.BindDouble(2, r1aabb.low.y);
        stmt.BindDouble(3, r1aabb.low.z);
        stmt.BindDouble(4, r1aabb.high.x);
        stmt.BindDouble(5, r1aabb.high.y);
        stmt.BindDouble(6, r1aabb.high.z);
        stmt.BindId(7, ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(8, "SomeOtherKindOfObstacle", Statement::MakeCopy::No);

        ASSERT_EQ( BE_SQLITE_ROW, stmt.Step() );

        ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1a , ofoundId );
        ASSERT_STREQ( "SomeOtherKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";

        //  Query should fail if I specify an item that is not found on any kind of obstacle
        stmt.Reset();
        stmt.ClearBindings();
        r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindDouble(1, r1aabb.low.x);
        stmt.BindDouble(2, r1aabb.low.y);
        stmt.BindDouble(3, r1aabb.low.z);
        stmt.BindDouble(4, r1aabb.high.x);
        stmt.BindDouble(5, r1aabb.high.y);
        stmt.BindDouble(6, r1aabb.high.z);
        stmt.BindId(7, ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(8, "no kind of obstacle", Statement::MakeCopy::No);

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
        stmt.BindDouble(1, r1aabb.low.x);
        stmt.BindDouble(2, r1aabb.low.y);
        stmt.BindDouble(3, r1aabb.low.z);
        stmt.BindDouble(4, r1aabb.high.x);
        stmt.BindDouble(5, r1aabb.high.y);
        stmt.BindDouble(6, r1aabb.high.z);
        stmt.BindId(7, ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(8, "SomeKindOfObstacle", Statement::MakeCopy::No);
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
        stmt.BindDouble(1, r1aabb.low.x);
        stmt.BindDouble(2, r1aabb.low.y);
        stmt.BindDouble(3, r1aabb.low.z);
        stmt.BindDouble(4, r1aabb.high.x);
        stmt.BindDouble(5, r1aabb.high.y);
        stmt.BindDouble(6, r1aabb.high.z);
        stmt.BindId(7, ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(8, "SomeKindOfObstacle", Statement::MakeCopy::No);
        while (BE_SQLITE_ROW == stmt.Step())
            {
            overlaps.insert (stmt.GetValueId<DgnElementId>(0));
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
TEST_F(SqlFunctionsTest, bbox_overlaps)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"bbox_overlaps.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    double o1y = 5.0;
    double o2x = 5.0;
    DPoint3d o1origin = DPoint3d::From(0,o1y,0);
    DPoint3d o2origin = DPoint3d::From(o2x,0,0);

    DgnElementId r1, o1, o2;
        {
        RobotElementPtr robot1 = RobotElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, DPoint3d::From(0,0,0), 0.0, "Robot1");
        InsertElement(*robot1);

        ObstacleElementPtr obstacle1 = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o1origin, 0.0, "Obstacle1");
        InsertElement(*obstacle1);

        ObstacleElementPtr obstacle2 = ObstacleElement::Create(*GetDefaultPhysicalModel(), m_defaultCategoryId, o2origin, 90.0, "Obstacle2");
        InsertElement(*obstacle2);

        // Double-check that we created the robots and obstacles, as expected
        ASSERT_EQ( RobotElement::QueryClassId(*m_db)    , robot1->GetElementClassId() );
        ASSERT_EQ( ObstacleElement::QueryClassId(*m_db) , obstacle1->GetElementClassId() );

        m_db->SaveChanges();

        r1 = robot1->GetElementId();
        o1 = obstacle1->GetElementId();
        o2 = obstacle2->GetElementId();
        }

    //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_bbox_overlaps.sampleCode
    //  Check if Robot1 has run into any obstacles
    Statement stmt;
    stmt.Prepare(*m_db, 
        "SELECT o.id, o.placement "
        " FROM "    //  r1 - the id and placement of a particular robot
        "       (SELECT e.Id as id, g.Placement as placement" 
        "        FROM dgn_Element e, dgn_ElementGeom g WHERE (e.Id = ? AND e.Id = g.ElementId)) r1" 
        "     ,"    //  o  - the id and placement of all obstacles
        "       (SELECT e.Id as id, g.Placement as placement"                                       
        "        FROM dgn_Element e, dgn_ElementGeom g WHERE (e.ECClassId=? AND e.Id = g.ElementId)) o"  
        " WHERE DGN_bbox_overlaps(DGN_placement_aabb(r1.Placement), DGN_placement_aabb(o.Placement))");
    //__PUBLISH_EXTRACT_END__
    stmt.BindId(1, r1);
    stmt.BindId(2, ObstacleElement::QueryClassId(*m_db));

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
        ASSERT_EQ( BE_SQLITE_ROW, stmt.Step() );

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1 , ofoundId );
        int resultSize = stmt.GetColumnBytes(1);
        ASSERT_EQ(sizeof(Placement3d), resultSize);
        Placement3dCR foundPlacement = *(Placement3dCP)stmt.GetValueBlob(1);
        ASSERT_TRUE( foundPlacement.GetOrigin().IsEqual( obstacle1->GetPlacement().GetOrigin() ) );

        ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() ) << L"Expected only 1 overlap";
        }

    if (true)   
        {
        // do the same thing, using the r-tree to speed up the query
        // Note: DGN_bbox_value indices are: XLow=0, YLow=1, Zlow=2, XHigh=3, YHigh=4, ZHigh=5

        //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_bbox_overlaps_rtree.sampleCode
        // Note: DGN_bbox_value indices are: XLow=0, YLow=1, Zlow=2, XHigh=3, YHigh=4, ZHigh=5
        //          In this query, we bind the parameters in the same order, but starting at 1:
        //                                   XLow=1, YLow=2, Zlow=3, XHigh=4, YHigh=5, ZHigh=6
        Statement stmt2;
        stmt2.Prepare(*m_db, 
            " SELECT e.Id as id, g.Placement as placement"                                       
            " FROM dgn_Element e, dgn_ElementGeom g, dgn_PrjRTree rt"
            " WHERE rt.MaxX >= ?1 AND rt.MinX <= ?4"        // find the rt nodes the overlap a specified bounding box
            "   AND rt.MaxY >= ?2 AND rt.MinY <= ?5"
            "   AND rt.MaxZ >= ?3 AND rt.MinZ <= ?6"
            "   AND rt.ElementId=e.Id"                      //  for all elements in those nodes
            "   AND e.Id = g.ElementId"                     //      choose the elements with placement bbs that overlap the specified bounding box
            "   AND DGN_bbox_overlaps(DGN_placement_aabb(g.Placement), DGN_bbox(?1, ?2, ?3, ?4, ?5, ?6))"
            "   AND e.ECClassId=?7"                         //                          and that are of a specific class
            );
        RobotElementCPtr robot1 = m_db->Elements().Get<RobotElement>(r1);
        AxisAlignedBox3d r1aabb = robot1->GetPlacement().CalculateRange();

        stmt2.BindDouble(1, r1aabb.low.x);
        stmt2.BindDouble(2, r1aabb.low.y);
        stmt2.BindDouble(3, r1aabb.low.z);
        stmt2.BindDouble(4, r1aabb.high.x);
        stmt2.BindDouble(5, r1aabb.high.y);
        stmt2.BindDouble(6, r1aabb.high.z);
        stmt2.BindId(7, ObstacleElement::QueryClassId(*m_db));
        //__PUBLISH_EXTRACT_END__
        ASSERT_EQ( BE_SQLITE_ROW, stmt2.Step() );

        DgnElementId ofoundId = stmt2.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1 , ofoundId );

        ASSERT_EQ( BE_SQLITE_DONE , stmt2.Step() ) << L"Expected only 1 overlap";
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
        ASSERT_EQ( BE_SQLITE_ROW, stmt.Step() );

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o2 , ofoundId );

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
        while (BE_SQLITE_ROW == stmt.Step())
            overlaps.insert (stmt.GetValueId<DgnElementId>(0));

        ASSERT_EQ( 2 , overlaps.size() );
        ASSERT_TRUE( overlaps.Contains(o1) );
        ASSERT_TRUE( overlaps.Contains(o2) );
        }

    //  Note that o1 and o2 also overlap each other. Verify that.
    //
    //  |
    //  |<---Obstacle1------->
    //  |                   ^
    //  |                   |
    //  |                   Obstacle2
    //  |                   |
    //  |                   V
    //  +-- -- -- -- -- -- --
    if (true)
        {
        Statement stmt2;
        stmt2.Prepare(*m_db, 
                        "SELECT allObstacles.id, allObstacles.placement "
                        " FROM "    //  obstacle1 - the id and placement of Obstacle1
                        "       (SELECT e.Id as id, g.Placement as placement" 
                        "        FROM dgn_Element e, dgn_ElementGeom g WHERE (e.Id = ? AND e.Id = g.ElementId)) obstacle1" 
                        "     ,"    //  allObstacles - the id and placement of all obstacles
                        "       (SELECT e.Id as id, g.Placement as placement"                                       
                        "        FROM dgn_Element e, dgn_ElementGeom g WHERE (e.ECClassId=? AND e.Id = g.ElementId)) allObstacles"  
                        " WHERE (allObstacles.id != obstacle1.id) AND DGN_bbox_overlaps(DGN_placement_aabb(obstacle1.Placement), DGN_placement_aabb(allObstacles.Placement))");
        stmt2.BindId(1, o1);
        stmt2.BindId(2, ObstacleElement::QueryClassId(*m_db));

        ASSERT_EQ( BE_SQLITE_ROW , stmt2.Step() );
        ASSERT_EQ( o2 , stmt2.GetValueId<DgnElementId>(0) );
        ASSERT_EQ( BE_SQLITE_DONE , stmt2.Step() );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SqlFunctionsTest, bbox_union)
    {
    DgnDbTestDgnManager tdm (L"04_Plant.i.idgndb", __FILE__, Db::OPEN_Readonly, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnDbP dgndb = tdm.GetDgnProjectP();

    Statement stmt;
    //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_bbox_union.sampleCode
    stmt.Prepare(*dgndb, "SELECT DGN_bbox_union(DGN_placement_aabb(g.Placement)) FROM " DGN_TABLE(DGN_CLASSNAME_Element) " AS e," DGN_TABLE(DGN_CLASSNAME_ElementGeom) 
                    " AS g WHERE e.ModelId=2 AND e.id=g.ElementId");
    //__PUBLISH_EXTRACT_END__
    auto rc = stmt.Step();
    ASSERT_EQ(BE_SQLITE_ROW, rc);

    int resultSize = stmt.GetColumnBytes(0);
    ASSERT_EQ(sizeof(DRange3d), resultSize);
    BoundingBox3dCR result = *(BoundingBox3dCP)stmt.GetValueBlob(0);
    ASSERT_TRUE(result.IsValid());

    stmt.Finalize();
    //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_angles.sampleCode
    stmt.Prepare(*dgndb, "SELECT count(*) FROM "
                         DGN_TABLE(DGN_CLASSNAME_ElementGeom) " AS g WHERE DGN_angles_maxdiff(DGN_placement_angles(g.Placement),DGN_Angles(0,0,90)) < 1.0");
    //__PUBLISH_EXTRACT_END__

    rc = stmt.Step();
    ASSERT_EQ(BE_SQLITE_ROW, rc);
    int count = stmt.GetValueInt(0);
    ASSERT_NE(count, 0);

    stmt.Finalize();
    stmt.Prepare(*dgndb, "SELECT count(*) FROM "
                         DGN_TABLE(DGN_CLASSNAME_ElementGeom) " AS g WHERE DGN_angles_value(DGN_placement_angles(g.Placement),2) < 90");

    rc = stmt.Step();
    ASSERT_EQ(BE_SQLITE_ROW, rc);
    count = stmt.GetValueInt(0);
    ASSERT_NE(count, 0);
    }

