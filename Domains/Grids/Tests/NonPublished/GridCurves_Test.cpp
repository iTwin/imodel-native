/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GridCurves_Test.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/GridsApi.h>
#include "GridsTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_GRIDS

//=======================================================================================
// Sets up environment for Grid Curves testing.
// @bsiclass                                    Haroldas.Vitunskas              03/2019
//=======================================================================================
struct GridCurvesTest : GridsTestFixtureBase
    {
    protected:
        SpatialLocationModelPtr m_model;

    public:
        GridCurvesTest() {};
        ~GridCurvesTest() {};

        void SetUp() override;
        void TearDown() override;

        ElevationGridCPtr InsertElevationGrid();
        OrthogonalGridCPtr InsertOrthogonalGrid();
        GridCurveBundleCPtr InsertOrthogonalGridCurves();
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas              03/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
void GridCurvesTest::SetUp()
    {
    GridsTestFixtureBase::SetUp();
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SpatialLocationPartitionPtr partition = SpatialLocationPartition::Create (*rootSubject, "GridSpatialPartition");
    db.Elements().Insert<SpatialLocationPartition> (*partition);
    m_model = SpatialLocationModel::CreateAndInsert (*partition);
    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas              03/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
void GridCurvesTest::TearDown()
    {
    m_model = nullptr;
    GridsTestFixtureBase::TearDown();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas              03/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
ElevationGridCPtr GridCurvesTest::InsertElevationGrid()
    {
    double heightInterval = 10;
    int gridIteration = 0;
    bvector<DPoint3d> baseShape = { {0, 0, 0}, {20, 0, 0}, {20, 20, 0}, {0, 20, 0}, {0, 0, 0} };
    bvector<CurveVectorPtr> floorPlaneCurves = bvector<CurveVectorPtr> (3); // 3 surfaces will be created
    for (CurveVectorPtr& curveShape : floorPlaneCurves)
        {
        bvector<DPoint3d> thisShape = baseShape;
        std::transform (thisShape.begin(), thisShape.end(), thisShape.begin(), [&] (DPoint3d point) -> DPoint3d {point.z = heightInterval * gridIteration; return point; });
        curveShape = CurveVector::CreateLinear (thisShape, CurveVector::BOUNDARY_TYPE_Outer);
        ++gridIteration;
        }

    DgnDbR db = *DgnClientApp::App().Project();
    return ElevationGrid::CreateAndInsertWithSurfaces (ElevationGrid::CreateParams (*m_model,
        db.Elements().GetRootSubject()->GetElementId(),
        "Floor-Grid"),
        floorPlaneCurves);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas              03/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGridCPtr GridCurvesTest::InsertOrthogonalGrid()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::CreateParams orthogonalParams = OrthogonalGrid::CreateParams (*m_model,
        db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
        "Orthogonal Grid",
        15, /*defaultCoordIncX*/
        10, /*defaultCoordIncY*/
        0.0, /*defaultStaExtX*/
        50.0, /*defaultEndExtX*/
        0.0, /*defaultStaExtY*/
        50.0, /*defaultEndExtY*/
        -2 * BUILDING_TOLERANCE, /*defaultStaElevation*/
        30.0 + 2 * BUILDING_TOLERANCE /*defaultEndElevation*/
    );

    return OrthogonalGrid::CreateAndInsertWithSurfaces (orthogonalParams, 2, 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas              03/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
GridCurveBundleCPtr GridCurvesTest::InsertOrthogonalGridCurves()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    ElevationGridCPtr elevationGrid = InsertElevationGrid();
    OrthogonalGridCPtr orthogonalGrid = InsertOrthogonalGrid();

    GridCurvesSetPtr curvesPortion = GridCurvesSet::Create (*m_model);
    curvesPortion->Insert();

    DgnElementId firstElevationSurfaceId = elevationGrid->GetSurfacesModel()->MakeIterator().BuildIdList<DgnElementId>().front();
    GridPlanarSurfaceCPtr floorGridSurface = db.Elements().Get<GridPlanarSurface> (firstElevationSurfaceId);
    orthogonalGrid->IntersectGridSurface (floorGridSurface.get(), *curvesPortion);

    DgnElementId firstCurveBundleId = floorGridSurface->MakeGridCurveBundleIterator().BuildIdList<DgnElementId>().front();
    return db.Elements().Get<GridCurveBundle> (firstCurveBundleId);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              03/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F (GridCurvesTest, GridCurvesHaveBubbleProperties)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    GridCurveBundleCPtr curveBundle = InsertOrthogonalGridCurves();
    ASSERT_TRUE (curveBundle.IsValid());

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    GridCurvePtr curve = dynamic_cast<GridCurve*>(curveBundle->GetGridCurve()->CopyForEdit().get());
    ASSERT_TRUE (curve.IsValid()) << "Failed to get grid curve";

    ASSERT_FALSE (curve->GetBubbleAtStart());
    ASSERT_FALSE (curve->GetBubbleAtEnd());
    
    curve->SetBubbleAtStart (true);
    ASSERT_TRUE (curve->GetBubbleAtStart());

    curve->SetBubbleAtEnd (true);
    ASSERT_TRUE (curve->GetBubbleAtEnd());

    db.SaveChanges();
    }
