#include <Bentley\BeTest.h>
#include <Grids/GridsApi.h>
#include "GridsTestFixtureBase.h"
#include <DgnClientFx/DgnClientApp.h>
#include "TestUtils.h"

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX

//=======================================================================================
// @bsiclass                                     Martynas.Saulius               03/2018
//=======================================================================================
struct SplineGridSurfacePlacementStrategyTestFixture : public GridsTestFixtureBase
    {
    SplineGridSurfacePlacementStrategyTestFixture() {};
    ~SplineGridSurfacePlacementStrategyTestFixture() {};

    void SetUp() override;
    void TearDown() override;

    SpatialLocationModelPtr m_model;
    SketchGridPtr m_sketchGrid;

    static DgnDbR GetDgnDb() { return *DgnClientApp::App().Project(); }
    };


//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void SplineGridSurfacePlacementStrategyTestFixture::SetUp()
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
    db.SaveChanges();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void SplineGridSurfacePlacementStrategyTestFixture::TearDown()
    {
    m_sketchGrid = nullptr;
    m_model = nullptr;
    GridsTestFixtureBase::TearDown();
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool            GetDgnExtrusionDetail
(
    DgnExtrusionDetail& extDetail,              //<= extrusion detail if success(true)
    SketchSplineGridSurfaceCR surface          //=>
)
    {
    auto pGeometrySource = surface.ToGeometrySource();
    if (nullptr == pGeometrySource)
        {
        return false;
        }

    GeometryCollection geomData(*pGeometrySource);
    if (geomData.begin() == geomData.end())
        {
        return false;
        }

    Transform elemToWorld = (*geomData.begin()).GetGeometryToWorld();

    bool status = false;

    GeometricPrimitivePtr geomPtr = (*geomData.begin()).GetGeometryPtr();
    if (geomPtr.IsValid())
        {
        ISolidPrimitivePtr solidPrimitive = geomPtr->GetAsISolidPrimitive();
        if (solidPrimitive.IsValid())
            {
            solidPrimitive->TransformInPlace(elemToWorld);
            status = solidPrimitive->TryGetDgnExtrusionDetail(extDetail);
            }
        }
    return status;
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(SplineGridSurfacePlacementStrategyTestFixture, CreateControlPointsSplineGridSurface) 
    {
    GetDgnDb().BriefcaseManager().StartBulkOperation();
    int order = 3;
    
    SplineGridSurfacePlacementStrategyPtr strat = SplineGridSurfacePlacementStrategy::Create(SplinePlacementStrategyType::ControlPoints, GetDgnDb());
    ASSERT_TRUE(strat.IsValid()) << "Strategy failed to create";
    strat->SetProperty(SplineControlPointsPlacementStrategy::prop_Order(), order);
    strat->SetProperty(SketchGridSurfacePlacementStrategy::prop_BottomElevation, 0.0);
    strat->SetProperty(SketchGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    strat->SetProperty(SketchGridSurfacePlacementStrategy::prop_Name, Utf8String("TestSketchSplineGridSurface"));

    ASSERT_FALSE(strat->IsComplete()) << "Strategy shouldn't be complete";
    strat->AddKeyPoint({ 0,0,0 });
    ASSERT_FALSE(strat->IsComplete()) << "Strategy shouldn't be complete";
    strat->AddKeyPoint({ 1,1,0 });
    ASSERT_FALSE(strat->IsComplete()) << "Strategy shouldn't be complete";
    strat->AddKeyPoint({ 2,0,0 });

    ASSERT_TRUE(strat->IsComplete()) << "Strategy should be complete";
    SketchSplineGridSurfacePtr surface = dynamic_cast<SketchSplineGridSurfaceP>(strat->FinishElement(*m_model).get());
    ASSERT_TRUE(surface.IsValid()) << "Surface should be valid";

    ICurvePrimitivePtr expectedSpline = TestUtils::CreateSpline({ { 0, 0, 0 },{ 1, 1, 0 },{ 2 ,0 ,0 } }, order);
    ICurvePrimitivePtr actualSpline = surface->GetBaseSpline();
   
    TestUtils::CompareCurves(actualSpline, expectedSpline);

    DgnExtrusionDetail detail;
    ASSERT_TRUE(GetDgnExtrusionDetail(detail, *surface)) << "Failed to get Extrusion Detail";
    ASSERT_TRUE(detail.m_extrusionVector.AlmostEqual(DVec3d::From(0,0,10))) << "Directions aren't equal";

    ASSERT_EQ(SUCCESS, GetDgnDb().SaveChanges()) << "Failed to save changes";
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(SplineGridSurfacePlacementStrategyTestFixture, CreateThroughPointsSplineGridSurface)
    {
    GetDgnDb().BriefcaseManager().StartBulkOperation();
    SplineGridSurfacePlacementStrategyPtr strat = SplineGridSurfacePlacementStrategy::Create(SplinePlacementStrategyType::ThroughPoints, GetDgnDb());
    ASSERT_TRUE(strat.IsValid()) << "Strategy failed to create";
    strat->SetProperty(SketchGridSurfacePlacementStrategy::prop_BottomElevation, 0.0);
    strat->SetProperty(SketchGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    strat->SetProperty(SketchGridSurfacePlacementStrategy::prop_Name, Utf8String("TestSketchSplineGridSurface"));

    ASSERT_FALSE(strat->IsComplete()) << "Strategy shouldn't be complete";
    strat->AddKeyPoint({ 0,0,0 });
    ASSERT_FALSE(strat->IsComplete()) << "Strategy shouldn't be complete";
    strat->AddKeyPoint({ 1,1,0 });
    ASSERT_TRUE(strat->IsComplete()) << "Strategy should be complete";
    strat->AddKeyPoint({ 2,0,0 });

    ASSERT_TRUE(strat->IsComplete()) << "Strategy should be complete";
    SketchSplineGridSurfacePtr surface = dynamic_cast<SketchSplineGridSurfaceP>(strat->FinishElement(*m_model).get());
    ASSERT_TRUE(surface.IsValid()) << "Surface should be valid";

    ICurvePrimitivePtr expectedSpline = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 1, 0 },{ 2, 0, 0 } });
    ICurvePrimitivePtr actualSpline = surface->GetBaseSpline();

    TestUtils::CompareCurves(actualSpline, expectedSpline);

    DgnExtrusionDetail detail;
    ASSERT_TRUE(GetDgnExtrusionDetail(detail, *surface)) << "Failed to get Extrusion Detail";
    ASSERT_TRUE(detail.m_extrusionVector.AlmostEqual(DVec3d::From(0, 0, 10))) << "Directions aren't equal";

    ASSERT_EQ(SUCCESS, GetDgnDb().SaveChanges()) << "Failed to save changes";
    }