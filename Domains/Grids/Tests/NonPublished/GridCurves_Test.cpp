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
// @bsiclass                                    Haroldas.Vitunskas              04/2019
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

        ElevationGridCPtr InsertElevationGrid() const;
        OrthogonalGridCPtr InsertOrthogonalGrid() const;
        GridCurveBundleCPtr InsertOrthogonalGridCurves() const;
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
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
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GridCurvesTest::TearDown()
    {
    m_model = nullptr;
    GridsTestFixtureBase::TearDown();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
ElevationGridCPtr GridCurvesTest::InsertElevationGrid() const
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
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//--------------------------------------------------------------------------------------- 
OrthogonalGridCPtr GridCurvesTest::InsertOrthogonalGrid() const
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::CreateParams orthogonalParams (*m_model,
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
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
GridCurveBundleCPtr GridCurvesTest::InsertOrthogonalGridCurves() const
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
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
static GridSurfaceCPtr getGridSurface(GridCurveCR curve)
    {
    bvector<DgnElementId> intersectingSurfaces = curve.GetIntersectingSurfaceIds ();
    for (DgnElementId surfaceId : intersectingSurfaces)
        {
        GridSurfaceCPtr surface = curve.GetDgnDb ().Elements ().Get<GridSurface> (surfaceId);
        if (nullptr == dynamic_cast<ElevationGridSurfaceCP>(surface.get ()))
            return surface;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @betest                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
TEST_F (GridCurvesTest, GridCurvesHaveLabels)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    GridCurveBundleCPtr curveBundle = InsertOrthogonalGridCurves();
    ASSERT_TRUE (curveBundle.IsValid());

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    GridCurvePtr curve = dynamic_cast<GridCurve*>(curveBundle->GetGridCurve()->CopyForEdit().get());
    ASSERT_TRUE (curve.IsValid());

    GridLabelCPtr gridLabel = curve->GetNonElevationSurfaceGridLabel();
    EXPECT_TRUE (gridLabel.IsNull());

    GridLabelPtr newLabel = GridLabel::Create (*getGridSurface(*curve), "Label", true, true);
    ASSERT_TRUE (newLabel.IsValid());

    DgnDbStatus status;
    newLabel->Insert (&status);
    EXPECT_EQ (DgnDbStatus::Success, status);
    EXPECT_EQ (BeSQLite::BE_SQLITE_OK, db.SaveChanges());

    gridLabel = curve->GetNonElevationSurfaceGridLabel();
    ASSERT_TRUE (gridLabel.IsValid());
    EXPECT_EQ (gridLabel->GetElementId(), newLabel->GetElementId());

    EXPECT_STRCASEEQ ("Label", gridLabel->GetLabel().c_str());
    EXPECT_TRUE (gridLabel->HasLabelAtStart());
    EXPECT_TRUE (gridLabel->HasLabelAtEnd());
    }
