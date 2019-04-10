/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ArcGridSurfacePlacementStrategy_Test.cpp $
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
struct ArcGridSurfacePlacementStrategyTestFixture : public GridsTestFixtureBase
    {
    ArcGridSurfacePlacementStrategyTestFixture() {};
    ~ArcGridSurfacePlacementStrategyTestFixture() {};

    void SetUp() override;
    void TearDown() override;

    SpatialLocationModelPtr m_model;
    SketchGridPtr m_sketchGrid;

    static DgnDbR GetDgnDb() { return *DgnClientApp::App().Project(); }
    };


//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategyTestFixture::SetUp()
    {
    GridsTestFixtureBase::SetUp();
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SpatialLocationPartitionPtr partition = SpatialLocationPartition::Create(*rootSubject, "GridSpatialPartition");
    db.Elements().Insert<SpatialLocationPartition>(*partition);
    m_model = SpatialLocationModel::CreateAndInsert(*partition);
    m_sketchGrid = SketchGrid::Create(*m_model.get(), partition->GetElementId(), "Sketch grid", 0.0, 10.0);
    m_sketchGrid->Insert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas                12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
void ArcGridSurfacePlacementStrategyTestFixture::TearDown()
    {
    m_sketchGrid = nullptr;
    m_model = nullptr;
    GridsTestFixtureBase::TearDown();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                  02/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(ArcGridSurfacePlacementStrategyTestFixture, CreateSketchArcGridSurface)
    {
    ArcGridSurfacePlacementStrategyPtr sut = ArcGridSurfacePlacementStrategy::Create(ArcPlacementMethod::StartMidEnd, GetDgnDb());
    ASSERT_TRUE(sut.IsValid());

    sut->SetProperty(SketchGridSurfacePlacementStrategy::prop_BottomElevation, 0.0);
    sut->SetProperty(SketchGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    sut->SetProperty(SketchGridSurfacePlacementStrategy::prop_Name, Utf8String("TestSketchArcGridSurface"));

    ASSERT_FALSE(sut->IsComplete());
    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->IsComplete());
    sut->AddKeyPoint({1,1,0});
    ASSERT_FALSE(sut->IsComplete());
    sut->AddKeyPoint({2,0,0});

    ASSERT_TRUE(sut->IsComplete());
    SketchArcGridSurfacePtr surface = dynamic_cast<SketchArcGridSurfaceP>(sut->FinishElement(*m_model).get());
    ASSERT_TRUE(surface.IsValid());

    DEllipse3d expectedArc = DEllipse3d::FromPointsOnArc({0,0,0}, {1,1,0}, {2,0,0});
    DEllipse3d actualArc;
    ASSERT_EQ(BentleyStatus::SUCCESS, surface->GetBaseArc(actualArc));
    ASSERT_TRUE(actualArc.IsAlmostEqual(expectedArc, DoubleOps::SmallCoordinateRelTol()));

    ASSERT_EQ(SUCCESS, GetDgnDb().SaveChanges());
    }