/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/LineGridSurfaceManipulationStrategy_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <Grids/GridsApi.h>
#include "GridsTestFixtureBase.h"
#include <DgnClientFx/DgnClientApp.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               02/2018
//=======================================================================================
struct LineGridSurfaceManipulationStrategyTestFixture : public GridsTestFixtureBase
    {
    LineGridSurfaceManipulationStrategyTestFixture() {};
    ~LineGridSurfaceManipulationStrategyTestFixture() {};

    void SetUp() override;
    void TearDown() override;

    SpatialLocationModelPtr m_model;
    SketchGridPtr m_sketchGrid;

    static DgnDbR GetDgnDb() { return *DgnClientApp::App().Project(); }
    };


//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------+---------------+---------------+---------------+---------------+------
void LineGridSurfaceManipulationStrategyTestFixture::SetUp()
    {
    GridsTestFixtureBase::SetUp();
    DgnDbR db = GetDgnDb();
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SpatialLocationPartitionPtr partition = SpatialLocationPartition::Create(*rootSubject, "GridSpatialPartition");
    BuildingLocks_LockElementForOperation(*partition.get(), BeSQLite::DbOpcode::Insert, "SpatialLocationPartition : Insert for LineGridSurfacePlacementStrategy_Test");
    db.Elements().Insert<SpatialLocationPartition>(*partition);
    m_model = SpatialLocationModel::CreateAndInsert(*partition);
    m_sketchGrid = SketchGrid::Create(*m_model.get(), partition->GetElementId(), "Sketch grid", 0.0, 10.0);
    BuildingLocks_LockElementForOperation(*m_sketchGrid.get(), BeSQLite::DbOpcode::Insert, "SketchGrid :  Insert for LineGridSurfacePlacementStrategy_Test");
    m_sketchGrid->Insert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas                12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
void LineGridSurfaceManipulationStrategyTestFixture::TearDown()
    {
    m_sketchGrid = nullptr;
    m_model = nullptr;
    GridsTestFixtureBase::TearDown();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                  04/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(LineGridSurfaceManipulationStrategyTestFixture, ModifyExisting)
    {
    SketchGridPtr grid = SketchGrid::Create(*m_model, m_model->GetModeledElementId(), "TEST_GRID", 0, 10);
    ASSERT_TRUE(grid.IsValid());
    BuildingLocks_LockElementForOperation(*grid.get(), BeSQLite::DbOpcode::Insert, "SketchGrid :  Insert for LineGridSurfacePlacementStrategy_Test");
    ASSERT_TRUE(grid->Insert().IsValid());

    Dgn::DgnModelCR defModel = BuildingUtils::GetGroupInformationModel(GetDgnDb());
    GridAxisPtr axis = GeneralGridAxis::CreateAndInsert(defModel, *grid);
    ASSERT_TRUE(axis.IsValid());

    SketchLineGridSurface::CreateParams surfaceCreateParams(*m_model, *axis, 0, 10, DPoint2d::From(0,0), DPoint2d::From(10,0));
    SketchLineGridSurfacePtr surface = SketchLineGridSurface::Create(surfaceCreateParams);
    ASSERT_TRUE(surface.IsValid());
    BuildingLocks_LockElementForOperation(*surface.get(), BeSQLite::DbOpcode::Insert, "SketchGrid :  Insert for LineGridSurfacePlacementStrategy_Test");
    ASSERT_TRUE(surface->Insert().IsValid());

    ASSERT_EQ(SUCCESS, GetDgnDb().SaveChanges());

    LineGridSurfaceManipulationStrategyPtr sut = LineGridSurfaceManipulationStrategy::Create(*surface);
    ASSERT_TRUE(sut.IsValid());

    DPoint2d updatedStart, updatedEnd;

    sut->UpdateDynamicKeyPoint({1,0,0}, 0);
    SketchLineGridSurfacePtr updatedSurface = dynamic_cast<SketchLineGridSurfaceP>(sut->FinishElement().get());
    ASSERT_TRUE(updatedSurface.IsValid());
    updatedSurface->GetBaseLine(updatedStart, updatedEnd);
    ASSERT_TRUE(updatedStart.AlmostEqual({1,0}));
    ASSERT_TRUE(updatedEnd.AlmostEqual({10,0}));

    sut->ReplaceKeyPoint({5,1,0}, 1);
    updatedSurface = dynamic_cast<SketchLineGridSurfaceP>(sut->FinishElement().get());
    ASSERT_TRUE(updatedSurface.IsValid());
    updatedSurface->GetBaseLine(updatedStart, updatedEnd);
    ASSERT_TRUE(updatedStart.AlmostEqual({0,0}));
    ASSERT_TRUE(updatedEnd.AlmostEqual({5,1}));

    ASSERT_EQ(SUCCESS, GetDgnDb().SaveChanges());

    updatedSurface = SketchLineGridSurface::GetForEdit(GetDgnDb(), surface->GetElementId());
    ASSERT_TRUE(updatedSurface.IsValid());
    updatedSurface->GetBaseLine(updatedStart, updatedEnd);
    ASSERT_TRUE(updatedStart.AlmostEqual({0,0}));
    ASSERT_TRUE(updatedEnd.AlmostEqual({5,1}));
    }