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
TEST_F(SqlFunctionsTest, TestPoints)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"TestPoints.idgndb", BeSQLite::Db::OPEN_ReadWrite);

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

    //  Run some queries that check proximity of Robot1 to obstacles
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

        RobotElementCPtr robot1 = m_db->Elements().Get<RobotElement>(r1);
        ASSERT_TRUE( robot1.IsValid() );
        RobotElementPtr robot1CC = dynamic_cast<RobotElement*>(robot1->MakeWriteableCopy().get());
        ASSERT_TRUE( robot1CC.IsValid() );
        robot1CC->Translate(DVec3d::FromStartEnd(robot1->GetPlacement().GetOrigin(), obstacle1->GetPlacement().GetOrigin()));
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

        RobotElementCPtr robot1 = m_db->Elements().Get<RobotElement>(r1);
        RobotElementPtr robot1CC = dynamic_cast<RobotElement*>(robot1->MakeWriteableCopy().get());
        robot1CC->Translate(DVec3d::FromStartEnd(robot1->GetPlacement().GetOrigin(), obstacle2->GetPlacement().GetOrigin()));
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
        RobotElementCPtr robot1 = m_db->Elements().Get<RobotElement>(r1);
        RobotElementPtr robot1CC = dynamic_cast<RobotElement*>(robot1->MakeWriteableCopy().get());
        robot1CC->Translate(DVec3d::FromStartEnd(robot1->GetPlacement().GetOrigin(), DPoint3d::From(o2x,o1y,0)));
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
