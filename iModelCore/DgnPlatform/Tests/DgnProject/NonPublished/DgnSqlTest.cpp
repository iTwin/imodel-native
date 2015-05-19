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
        // This is an example of passing the wrong type of object to DGN_bbox_areaxy and getting an error.
        // This statement is wrong, because DGN_placement_angles returns a DGN_angles object, while DGN_bbox_areaxy expects a DGN_bbox object.
        // Note that the error is detected when you try to step the statement, not when you prepare it.
        Statement stmt;
        stmt.Prepare(*m_db, "SELECT DGN_bbox_areaxy(DGN_placement_angles(Placement)) FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom));
        DbResult rc = stmt.Step(); // rc will be BE_SQLITE_ERROR, and m_db->GetLastError() will return "Illegal input to DGN_bbox_areaxy"
        //__PUBLISH_EXTRACT_END__
        ASSERT_EQ( BE_SQLITE_ERROR , rc );
        BeTest::Log("SqlFunctionsTest", BeTest::PRIORITY_INFO, Utf8PrintfString("SQLite error: %s\n", m_db->GetLastError())); // displays "SQLite error: Illegal input to DGN_bbox_areaxy"
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
        // This is an example of using DGN_placement_eabb to sum up element areas. Note that we must to use 
        // element-aligned bounding boxes in this query, rather than axis-aligned bounding boxes.
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
        // This query uses DGN_angles_value to extract the Yaw angle of an element's placement, in order to compare it with 90.
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
        // This query looks for "Obstacles" that are oriented at 90 degrees in the X-Y plane.
        // It is an example of using DGN_angles_maxdiff to look for elements with a specific placement angle. 
        // This example also shows how to combine tests on geometry and business properties in a single query. 
        // This example uses ECSql.
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
TEST_F(SqlFunctionsTest, DGN_point_min_distance_to_bbox)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"DGN_point_min_distance_to_bbox.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    double o1y = 2.0;
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

	// Note:  Can't use ECSql here. It only allows ECClases, and dgn_PrjRTree is not in the ecschema
	
    Statement stmt;
    stmt.Prepare(*m_db, 
        "SELECT item.ElementId, item.x01 FROM"
        "  (SELECT g.ElementId FROM"
        "     (SELECT rt.ElementId FROM dgn_PrjRTree rt "                       //          FROM R-Tree
        "        WHERE rt.MaxX >= Dgn_bbox_value(:bbox,0) AND rt.MinX <= Dgn_bbox_value(:bbox,3)" // select Elements in nodes that overlap bbox
        "          AND rt.MaxY >= Dgn_bbox_value(:bbox,1) AND rt.MinY <= Dgn_bbox_value(:bbox,4)"
        "          AND rt.MaxZ >= Dgn_bbox_value(:bbox,2) AND rt.MinZ <= Dgn_bbox_value(:bbox,5)) rt_res" // (call this result set "rt_res")
        "     ,"
        "      dgn_ElementGeom g WHERE g.ElementId=rt_res.ElementId"            //      JOIN ElementGeom
        "          AND DGN_point_min_distance_to_bbox(:testPoint, DGN_placement_aabb(g.Placement)) <= :maxDistance"  // select geoms that are within some distance of a specified point
        "  ) geom_res"                                                          //      (call this result set "geom_res")
        "  , dgn_Element e"                                                     // JOIN Element
        "  , dgn_ElementItem item "                                             // JOIN ElementItem
        "       WHERE e.Id = geom_res.ElementId AND e.ECClassId=:ecClass"       //  select only Obstacles
        "        AND item.ElementId = e.Id AND item.x01 = :propertyValue"       //                     ... with certain items
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

        ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );

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
            hits.insert (stmt.GetValueId<DgnElementId>(0));
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
    SetupProject(L"3dMetricGeneral.idgndb", L"spatialQueryECSql.idgndb", BeSQLite::Db::OPEN_ReadWrite);

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

	// Note:  Can't use ECSql here. It only allows ECClases, and dgn_PrjRTree is not in the ecschema
	
    ECSqlStatement stmt;
    stmt.Prepare(*m_db, 
        "SELECT TestItem.ECInstanceId, TestItem.TestItemProperty FROM dgn.ElementRTree rt, DgnPlatformTest.Obstacle, DgnPlatformTest.TestItem"
        "     WHERE rt.MaxX >= Dgn_bbox_value(:bbox,0) AND rt.MinX <= Dgn_bbox_value(:bbox,3)"
        "       AND rt.MaxY >= Dgn_bbox_value(:bbox,1) AND rt.MinY <= Dgn_bbox_value(:bbox,4)"
        "       AND rt.MaxZ >= Dgn_bbox_value(:bbox,2) AND rt.MinZ <= Dgn_bbox_value(:bbox,5)"
        "       AND Obstacle.ECInstanceId = rt.ECInstanceId"
        "       AND TestItem.ECInstanceId = rt.ECInstanceId AND TestItem.TestItemProperty = :propertyValue"
        );

    //  Make sure that the range tree index is used (first) and that other tables are indexed (after)
    auto qplan = m_db->ExplainQueryPlan(nullptr, stmt.GetNativeSql());
    auto scanRt = qplan.find("SCAN TABLE dgn_PrjRTree VIRTUAL TABLE INDEX");
    auto searchItem = qplan.find("SEARCH TABLE dgn_ElementItem USING INTEGER PRIMARY KEY");
    auto searchElement = qplan.find ("SEARCH TABLE dgn_Element USING INTEGER PRIMARY KEY");
    ASSERT_NE( Utf8String::npos , scanRt );
    ASSERT_NE( Utf8String::npos , searchItem );
    ASSERT_NE( Utf8String::npos , searchElement );
    ASSERT_LT( scanRt , searchItem );
    ASSERT_LT( scanRt , searchElement );

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

        ASSERT_EQ(BeSQLite::EC::ECSqlStepStatus::Done, stmt.Step());
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

        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::HasRow, stmt.Step() );

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1 , ofoundId );
        ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::Done , stmt.Step() ) << L"Expected only 1 overlap";

        //  Query for the other kind of obstacle
        stmt.Reset();
        stmt.ClearBindings();
        r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBinary(stmt.GetParameterIndex("bbox"), &r1aabb, sizeof(r1aabb), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        stmt.BindText(stmt.GetParameterIndex("propertyValue"), "SomeOtherKindOfObstacle", BeSQLite::EC::IECSqlBinder::MakeCopy::No);

        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::HasRow, stmt.Step() );

        ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o1a , ofoundId );
        ASSERT_STREQ( "SomeOtherKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::Done , stmt.Step() ) << L"Expected only 1 overlap";

        //  Query should fail if I specify an item that is not found on any kind of obstacle
        stmt.Reset();
        stmt.ClearBindings();
        r1aabb = robot1->GetPlacement().CalculateRange();      // bind to new range
        stmt.BindBinary(stmt.GetParameterIndex("bbox"), &r1aabb, sizeof(r1aabb), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        stmt.BindText(stmt.GetParameterIndex("propertyValue"), "no kind of obstacle", BeSQLite::EC::IECSqlBinder::MakeCopy::No);

        ASSERT_EQ(BeSQLite::EC::ECSqlStepStatus::Done, stmt.Step());
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
        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::HasRow, stmt.Step() );

        DgnElementId ofoundId = stmt.GetValueId<DgnElementId>(0);
        ASSERT_EQ( o2 , ofoundId );
        ASSERT_STREQ( "SomeKindOfObstacle", stmt.GetValueText(1) );

        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::Done , stmt.Step() ) << L"Expected only 1 overlap";
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
        while (BeSQLite::EC::ECSqlStepStatus::HasRow == stmt.Step())
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

	// Note:  Can't use ECSql here. It only allows ECClases, and dgn_PrjRTree is not in the ecschema
	
    Statement stmt;
    stmt.Prepare(*m_db, 
        "SELECT item.ElementId, item.x01 FROM"
        "  (SELECT rt.ElementId FROM dgn_PrjRTree rt "                          //          FROM R-Tree
        "     WHERE rt.MaxX >= Dgn_bbox_value(:bbox,0) AND rt.MinX <= Dgn_bbox_value(:bbox,3)"  // select Elements in nodes that overlap bbox
        "       AND rt.MaxY >= Dgn_bbox_value(:bbox,1) AND rt.MinY <= Dgn_bbox_value(:bbox,4)"
        "       AND rt.MaxZ >= Dgn_bbox_value(:bbox,2) AND rt.MinZ <= Dgn_bbox_value(:bbox,5)) rt_res" // (call this result set "rt_res")
        "  , dgn_Element e"                                                     // JOIN Element
        "  , dgn_ElementItem item "                                             // JOIN ElementItem
        "       WHERE e.Id = rt_res.ElementId AND e.ECClassId=:ecClass"         //  select only Obstacles
        "        AND item.ElementId = e.Id AND item.x01 = :propertyValue"       //                     ... with certain items
        );

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
        stmt.BindBlob(stmt.GetParameterIndex(":bbox"), &r1aabb, sizeof(r1aabb), Statement::MakeCopy::No);
        stmt.BindId(stmt.GetParameterIndex(":ecClass"), ObstacleElement::QueryClassId(*m_db));
        stmt.BindText(stmt.GetParameterIndex(":propertyValue"), "SomeKindOfObstacle", Statement::MakeCopy::No);

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
            overlaps.insert (stmt.GetValueId<DgnElementId>(0));
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
    DgnDbTestDgnManager tdm (L"04_Plant.i.idgndb", __FILE__, Db::OPEN_Readonly, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnDbP dgndb = tdm.GetDgnProjectP();

    Statement stmt;
    //__PUBLISH_EXTRACT_START__ DgnSchemaDomain_SqlFuncs_DGN_bbox_union.sampleCode
    // This is an example of accumlating the union of bounding boxes.
    // Note that when computing a union, it only makes sense to use axis-aligned bounding boxes, not element-aligned bounding boxes.
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
    // An example of constructing a DGN_Angles object in order to test the placement angles of elements in the Db.
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

