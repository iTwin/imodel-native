/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <BeXml\BeXml.h>
#include <Bentley\BeTest.h>
#include <DgnPlatform\UnitTests\DgnDbTestUtils.h>
#include <DgnPlatform\UnitTests\ScopedDgnHost.h>
#include <Grids/GridsApi.h>
#include <DgnPlatform\FunctionalDomain.h>
#include "GridsTestFixtureBase.h"
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// Sets up environment for Grids testing.
// @bsiclass                                    Jonas.Valiunas                  10/2017
//=======================================================================================
struct GridsTestFixture : public GridsTestFixtureBase
    {
    protected:
        SpatialLocationModelPtr m_model;

    public:
        GridsTestFixture() {};
        ~GridsTestFixture() {};

        void SetUp() override;
        void TearDown() override;

        //! Utility for testing
        //! returns create params for orthogonal grid with values:
        //! Horizontal count = 5
        //! Vertical count = 4
        //! Horizontal interval = 15
        //! Vertical interval = 10
        //! Length = 50
        //! Height = 70
        //! Normal = (1, 0, 0)
        //! Horizontal extension = (0, 0, 0)
        //! Vertical extension = (0, 0, 0)
        //! Dimensions = false
        //! Extension = false
        OrthogonalGrid::CreateParams GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

        //! Utility for testing
        //! returns create params for orthogonal grid with values:
        //! Horizontal count = 5
        //! Vertical count = 4
        //! Horizontal interval = 15
        //! Vertical interval = 10
        //! Length = 50
        //! Height = 70
        //! Normal = (1, 0, 0)
        //! Horizontal extension = (0, 0, 0)
        //! Vertical extension = (0, 0, 0)
        //! Dimensions = true
        //! Extension = false
        OrthogonalGrid::CreateParams GetTestDefaultCreateParamsForOrthogonalGridConstrained();

        //! Utility for testing
        //! returns create params for orthogonal grid with values:
        //! Horizontal count = 5
        //! Vertical count = 4
        //! Horizontal interval = 15
        //! Vertical interval = 10
        //! Length = 50
        //! Height = 70
        //! Normal = (1, 0, 0)
        //! Horizontal extension = (1, 0, 0)
        //! Vertical extension = (0, 1, 0)
        //! Dimensions = false
        //! Extension = true
        OrthogonalGrid::CreateParams GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended();

        //! Utility for testing
        //! returns create params for orthogonal grid with values:
        //! Horizontal count = 5
        //! Vertical count = 4
        //! Horizontal interval = 15
        //! Vertical interval = 10
        //! Length = 50
        //! Height = 70
        //! Normal = (1, 0, 0)
        //! Horizontal extension = (1, 0, 0)
        //! Vertical extension = (0, 1, 0)
        //! Dimensions = true
        //! Extension = true
        OrthogonalGrid::CreateParams GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended();

        //! Utility for testing
        //! returns create params for radial grid with values:
        //! Plane count = 7
        //! Circular count = 5
        //! Plane iteration angle = 7 (DEG)
        //! Circular interval = 13
        //! Length = 70
        //! Height = 50
        //! Extension = false;
        RadialGrid::CreateParams GetTestDefaultCreateParamsForRadialGrid();
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
void GridsTestFixture::SetUp()
    {
    GridsTestFixtureBase::SetUp ();
    DgnDbR db = *DgnClientApp::App ().Project ();
    db.BriefcaseManager().StartBulkOperation();
    SubjectCPtr rootSubject = db.Elements ().GetRootSubject ();
    SpatialLocationPartitionPtr partition = SpatialLocationPartition::Create(*rootSubject, "GridSpatialPartition");
    db.Elements().Insert<SpatialLocationPartition>(*partition);
    m_model = SpatialLocationModel::CreateAndInsert (*partition);
    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
void GridsTestFixture::TearDown()
    {
    m_model = nullptr;
    GridsTestFixtureBase::TearDown ();
    }

//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGrid::CreateParams GridsTestFixture::GetTestDefaultCreateParamsForOrthogonalGridUnconstrained()
    {
    //5 horiz
    //4 vert
    DgnDbR db = *DgnClientApp::App().Project();
    return OrthogonalGrid::CreateParams(*m_model,
                                                db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                "Unconstrained Grid",
                                                15, /*defaultCoordIncX*/
                                                10, /*defaultCoordIncY*/
                                                0.0, /*defaultStaExtX*/
                                                50.0, /*defaultEndExtX*/
                                                0.0, /*defaultStaExtY*/
                                                50.0, /*defaultEndExtY*/
                                                0.0, /*defaultStaElevation*/
                                                70.0 /*defaultEndElevation*/
                                                );
    }

//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGrid::CreateParams GridsTestFixture::GetTestDefaultCreateParamsForOrthogonalGridConstrained()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    return OrthogonalGrid::CreateParams(*m_model,
                                                db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                "Constrained Grid",
                                                15, /*defaultCoordIncX*/
                                                10, /*defaultCoordIncY*/
                                                0.0, /*defaultStaExtX*/
                                                50.0, /*defaultEndExtX*/
                                                0.0, /*defaultStaExtY*/
                                                50.0, /*defaultEndExtY*/
                                                0.0, /*defaultStaElevation*/
                                                70.0 /*defaultEndElevation*/
                                                );
    }


//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGrid::CreateParams GridsTestFixture::GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    return OrthogonalGrid::CreateParams(*m_model,
                                                db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                "Unconstrained Grid",
                                                15, /*defaultCoordIncX*/
                                                10, /*defaultCoordIncY*/
                                                -1.0, /*defaultStaExtX*/
                                                49.0, /*defaultEndExtX*/
                                                -1.0, /*defaultStaExtY*/
                                                49, /*defaultEndExtY*/
                                                0.0, /*defaultStaElevation*/
                                                70.0 /*defaultEndElevation*/
                                                );
    }

//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGrid::CreateParams GridsTestFixture::GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    DVec3d horizExtTrans = DVec3d::From(1.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 1.0, 0.0);
    return OrthogonalGrid::CreateParams(*m_model,
                                        db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                        "Constrained Grid",
                                        15, /*defaultCoordIncX*/
                                        10, /*defaultCoordIncY*/
                                        -1.0, /*defaultStaExtX*/
                                        49.0, /*defaultEndExtX*/
                                        -1.0, /*defaultStaExtY*/
                                        49, /*defaultEndExtY*/
                                        0.0, /*defaultStaElevation*/
                                        70.0 /*defaultEndElevation*/
                                        );
    }

//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
RadialGrid::CreateParams GridsTestFixture::GetTestDefaultCreateParamsForRadialGrid()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    return RadialGrid::CreateParams(*m_model,
                                    db.Elements().GetRootSubject()->GetElementId(), /*scope element*/
                                    "Radial Grid",   /*name*/
                                    7 * msGeomConst_pi / 180, /*defaultAngleIncrement*/
                                    13, /*defaultRadiusIncrement*/
                                    -msGeomConst_piOver12, /*defaultStartAngle*/
                                    7 * 7 * msGeomConst_pi / 180 + msGeomConst_piOver12, /*defaultEndAngle*/
                                    0.0, /*defaultStartRadius*/
                                    70, /*defaultEndRadius*/
                                    0.0, /*defaultstaElevation*/
                                    50 /*defaultendElevation*/
                                    );
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Unconstrained_CreatedAndDeleted)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGridUnconstrained = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    DPlane3d gridPlane = orthogonalGridUnconstrained->GetPlane();
    ASSERT_TRUE(gridPlane.origin.AlmostEqual({ 0, 0, 0 })) << "Grid plane origin is incorrect";
    ASSERT_TRUE(gridPlane.normal.AlmostEqual(DVec3d::From( 0, 0, 1 ))) << "Grid plane normal is incorrect";

    db.SaveChanges ();

    db.BriefcaseManager().StartBulkOperation();

    GridPtr thisGrid = Grid::TryGet(db, db.Elements().GetRootSubject()->GetElementId(), "Unconstrained Grid");
    ASSERT_TRUE(thisGrid.IsValid()) << "Failed to get created grid";
    ASSERT_EQ(orthogonalGridUnconstrained->GetElementId(), thisGrid->GetElementId()) << "Loaded grid's element id is incorrect";

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE (orthogonalGridUnconstrained.IsValid()) << "Failed to create orthogonal grid";

    ASSERT_TRUE (orthogonalGridUnconstrained->GetSurfacesModel().IsValid()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridUnconstrained->MakeIterator ().BuildIdList<DgnElementId> ().size ();
    ASSERT_TRUE (numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";

    ASSERT_TRUE(0 == std::strcmp("Unconstrained Grid", orthogonalGridUnconstrained->GetName())) << "Grid name is not correct";

    /////////////////////////////////////////////////////////////
    // Check if axes are valid and have correct number of elements
    /////////////////////////////////////////////////////////////
    ElementIterator axesIterator = orthogonalGridUnconstrained->MakeAxesIterator();
    bvector<DgnElementId> axesIds = axesIterator.BuildIdList<DgnElementId>();
    int numAxes = axesIds.size();
    ASSERT_TRUE(numAxes == 2) << "incorrect number of axes in orthogonalGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);
    ASSERT_TRUE(verticalAxis.IsValid()) << "vertical axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(5, numHorizontal) << "incorrect number of elements in horizontal axis";

    int numVertical = verticalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(4, numVertical) << "incorrect number of elements in vertical axis";

    /////////////////////////////////////////////////////////////
    // Check if axes elements are valid
    /////////////////////////////////////////////////////////////
    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr plane = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        ASSERT_TRUE(plane.IsValid()) << "horizontal element invalid";
        horizontalElements.push_back(plane);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr plane = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        ASSERT_TRUE(plane.IsValid()) << "vertical element invalid";
        verticalElements.push_back(plane);
        }

    /////////////////////////////////////////////////////////////
    // Check if axes elements parallel and perpendicular
    /////////////////////////////////////////////////////////////
    // for any two elements e_1 and e_2 : if both elements are in same axis, they must be parallel, otherwise they must be perpendicular
    bvector<GridPlanarSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), horizontalElements.begin(), horizontalElements.end());
    allElements.insert(allElements.end(), verticalElements.begin(), verticalElements.end());

    for (GridPlanarSurfaceCPtr firstElem : allElements)
        {
        for (GridPlanarSurfaceCPtr secondElem : allElements)
            {
            if (firstElem->GetElementId() == secondElem->GetElementId())
                continue;

            bool firstInHorizontal = horizontalElements.end() != std::find_if(horizontalElements.begin(), horizontalElements.end(), [&](auto plane) {return firstElem->GetElementId() == plane->GetElementId(); });
            bool secondInHorizontal = horizontalElements.end() != std::find_if(horizontalElements.begin(), horizontalElements.end(), [&](auto plane) {return secondElem->GetElementId() == plane->GetElementId(); });

            DVec3d firstNormal = firstElem->GetPlane().normal;
            DVec3d secondNormal = secondElem->GetPlane().normal;

            if (firstInHorizontal == secondInHorizontal)
                {
                ASSERT_TRUE(firstNormal.IsParallelTo(secondNormal)) << "Grid elements in same axes are not parallel";
                }
            else
                {
                ASSERT_TRUE(firstNormal.IsPerpendicularTo(secondNormal)) << "Grid elements in different axes are not perpendicular";
                }
            }
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct placement
    /////////////////////////////////////////////////////////////
    // for horizontal elements placement origins should be: (0, 0, 0), (0, 15, 0), (0, 30, 0), (0, 45, 0), (0, 60, 0).
    // for vertical elements placement origins should be: (0, 0, 0), (10, 0, 0), (20, 0, 0), (30, 0, 0)
    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin())) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 15, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin())) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 30, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin())) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 45, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin())) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 60, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin())) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin())) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(10, 0, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin())) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(20, 0, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin())) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(30, 0, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin())) << "Vertical plane 3 origin is incorrect";

    // all horizontal elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    // all vertical elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(-msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        double length = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetLength(length)) << "Grid surface length should be accessible";
        ASSERT_EQ(50, length) << "Grid surface length is incorrect";

        double height = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetHeight(height)) << "Grid surface height should be accessible";
        ASSERT_EQ(70, height) << "Grid surface height is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid is deleted correctly
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(DgnDbStatus::Success, orthogonalGridUnconstrained->Delete()) << "Error in deleting grid";
    ASSERT_TRUE(db.Elements().Get<OrthogonalGrid>(orthogonalGridUnconstrained->GetElementId()).IsNull()) << "Grid has not been deleted";

    for (DgnElementId axisId : axesIds)
        {
        ASSERT_TRUE(db.Elements().Get<GridAxis>(axisId).IsNull()) << "Grid axis has not been deleted";
        }

    for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(db.Elements().Get<GridSurface>(surface->GetElementId()).IsNull()) << "Grid surface has not been deleted";
        }

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Unconstrained_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGridUnconstrained = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridUnconstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    Placement3d translatedPlacement(orthogonalGridUnconstrained->GetPlacement());
    translatedPlacement.SetOrigin(newBaseOrigin);
    orthogonalGridUnconstrained->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridUnconstrained->Update().IsValid()) << "orthogonal grid translation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    bvector<DgnElementId> axesIds = orthogonalGridUnconstrained->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (23, 76, 0, 0), (23, 91, 0), (23, 106, 0), (23, 121, 0), (23, 136, 0).
    // for vertical elements placement origins should be: (23, 76, 0), (33, 76, 0), (43, 76, 0), (53, 76, 0)
    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin())) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 91, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin())) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 106, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin())) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 121, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin())) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 136, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin())) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin())) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(33, 76, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin())) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(43, 76, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin())) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(53, 76, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin())) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(-msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Unconstrained_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGridUnconstrained = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridUnconstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 6; // 30 deg
    Placement3d translatedPlacement(orthogonalGridUnconstrained->GetPlacement());
    translatedPlacement.GetAnglesR().SetYaw(AngleInDegrees::FromRadians(newAngle));
    orthogonalGridUnconstrained->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridUnconstrained->Update().IsValid()) << "orthogonal grid translation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    double existingAngle;
    ASSERT_EQ(newAngle, orthogonalGridUnconstrained->GetPlacement().GetAngles().GetYaw().Radians()) << "Grid's rotation angle is incorrect";

    bvector<DgnElementId> axesIds = orthogonalGridUnconstrained->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (0, 0, 0, 0), (-7.5, 13, 0), (-15, 26, 0), (-22.5, 39, 0), (-30, 52, 0).
    // for vertical elements placement origins should be: (0, 0, 0), (8.7, 5, 0), (17.3, 10, 0), (26, 15, 0).
    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-7.5, 12.990381, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-15, 25.980762, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-22.5, 38.971143, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-30, 51.961524, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(8.660254, 5, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(17.320508, 10, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(25.980762, 15, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle - msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Unconstrained_PlacementCorrectAfterTranslationAndRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGridUnconstrained = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridUnconstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    double newAngle = msGeomConst_pi / 6; // 30 deg
    Placement3d translatedPlacement(orthogonalGridUnconstrained->GetPlacement());
    translatedPlacement.GetAnglesR().SetYaw(AngleInDegrees::FromRadians(newAngle));
    translatedPlacement.SetOrigin(newBaseOrigin);
    orthogonalGridUnconstrained->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridUnconstrained->Update().IsValid()) << "orthogonal grid translation+rotation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    ASSERT_EQ(newAngle, orthogonalGridUnconstrained->GetPlacement().GetAngles().GetYaw().Radians()) << "Grid's rotation angle is incorrect";


    bvector<DgnElementId> axesIds = orthogonalGridUnconstrained->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (23, 76, 0), (15.5, 89, 0), (8, 102, 0), (0.5, 115, 0), (-7, 128, 0).
    // for vertical elements placement origins should be: (23, 76, 0), (31.7, 81, 0), (40.3, 86, 0), (49, 91, 0).
    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(15.5, 88.990381, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(8, 101.980762, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0.5, 114.971143, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-7, 127.961524, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(31.660254, 81, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(40.320508, 86, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(48.980762, 91, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle - msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_UnconstrainedExtended_CreatedAndDeleted)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended();

    OrthogonalGridPtr orthogonalGridUnconstrainedExtended = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    DPlane3d gridPlane = orthogonalGridUnconstrainedExtended->GetPlane();
    ASSERT_TRUE(gridPlane.origin.AlmostEqual({ 0, 0, 0 })) << "Grid plane origin is incorrect";
    ASSERT_TRUE(gridPlane.normal.AlmostEqual(DVec3d::From(0, 0, 1))) << "Grid plane normal is incorrect";

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    GridPtr thisGrid = Grid::TryGet(db, db.Elements().GetRootSubject()->GetElementId(), "Unconstrained Grid");
    ASSERT_TRUE(thisGrid.IsValid()) << "Failed to get created grid";
    ASSERT_EQ(orthogonalGridUnconstrainedExtended->GetElementId(), thisGrid->GetElementId()) << "Loaded grid's element id is incorrect";

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(orthogonalGridUnconstrainedExtended.IsValid()) << "Failed to create orthogonal grid";

    ASSERT_TRUE(orthogonalGridUnconstrainedExtended->GetSurfacesModel().IsValid()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridUnconstrainedExtended->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";

    ASSERT_TRUE(0 == std::strcmp("Unconstrained Grid", orthogonalGridUnconstrainedExtended->GetName())) << "Grid name is not correct";

    /////////////////////////////////////////////////////////////
    // Check if axes are valid and have correct number of elements
    /////////////////////////////////////////////////////////////
    ElementIterator axesIterator = orthogonalGridUnconstrainedExtended->MakeAxesIterator();
    bvector<DgnElementId> axesIds = axesIterator.BuildIdList<DgnElementId>();
    int numAxes = axesIds.size();
    ASSERT_TRUE(numAxes == 2) << "incorrect number of axes in orthogonalGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);
    ASSERT_TRUE(verticalAxis.IsValid()) << "vertical axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(5, numHorizontal) << "incorrect number of elements in horizontal axis";

    int numVertical = verticalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(4, numVertical) << "incorrect number of elements in vertical axis";

    /////////////////////////////////////////////////////////////
    // Check if axes elements are valid
    /////////////////////////////////////////////////////////////
    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr plane = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        ASSERT_TRUE(plane.IsValid()) << "horizontal element invalid";
        horizontalElements.push_back(plane);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr plane = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        ASSERT_TRUE(plane.IsValid()) << "vertical element invalid";
        verticalElements.push_back(plane);
        }

    /////////////////////////////////////////////////////////////
    // Check if axes elements parallel and perpendicular
    /////////////////////////////////////////////////////////////
    // for any two elements e_1 and e_2 : if both elements are in same axis, they must be parallel, otherwise they must be perpendicular
    bvector<GridPlanarSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), horizontalElements.begin(), horizontalElements.end());
    allElements.insert(allElements.end(), verticalElements.begin(), verticalElements.end());

    for (GridPlanarSurfaceCPtr firstElem : allElements)
        {
        for (GridPlanarSurfaceCPtr secondElem : allElements)
            {
            if (firstElem->GetElementId() == secondElem->GetElementId())
                continue;

            bool firstInHorizontal = horizontalElements.end() != std::find_if(horizontalElements.begin(), horizontalElements.end(), [&](auto plane) {return firstElem->GetElementId() == plane->GetElementId(); });
            bool secondInHorizontal = horizontalElements.end() != std::find_if(horizontalElements.begin(), horizontalElements.end(), [&](auto plane) {return secondElem->GetElementId() == plane->GetElementId(); });

            DVec3d firstNormal = firstElem->GetPlane().normal;
            DVec3d secondNormal = secondElem->GetPlane().normal;

            if (firstInHorizontal == secondInHorizontal)
                {
                ASSERT_TRUE(firstNormal.IsParallelTo(secondNormal)) << "Grid elements in same axes are not parallel";
                }
            else
                {
                ASSERT_TRUE(firstNormal.IsPerpendicularTo(secondNormal)) << "Grid elements in different axes are not perpendicular";
                }
            }
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct placement
    /////////////////////////////////////////////////////////////
    // for horizontal elements placement origins should be: (0, 0, 0), (0, 15, 0), (0, 30, 0), (0, 45, 0), (0, 60, 0).
    // for vertical elements placement origins should be: (0, 0, 0), (10, 0, 0), (20, 0, 0), (30, 0, 0)
    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin())) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 15, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin())) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 30, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin())) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 45, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin())) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 60, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin())) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin())) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(10, 0, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin())) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(20, 0, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin())) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(30, 0, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin())) << "Vertical plane 3 origin is incorrect";

    // all horizontal elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        double length = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetLength(length)) << "Grid surface length should be accessible";
        ASSERT_EQ(50, length) << "Grid surface length is incorrect";

        double height = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetHeight(height)) << "Grid surface height should be accessible";
        ASSERT_EQ(70, height) << "Grid surface height is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid is deleted correctly
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(DgnDbStatus::Success, orthogonalGridUnconstrainedExtended->Delete()) << "Error in deleting grid";
    ASSERT_TRUE(db.Elements().Get<OrthogonalGrid>(orthogonalGridUnconstrainedExtended->GetElementId()).IsNull()) << "Grid has not been deleted";

    for (DgnElementId axisId : axesIds)
        {
        ASSERT_TRUE(db.Elements().Get<GridAxis>(axisId).IsNull()) << "Grid axis has not been deleted";
        }

    for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(db.Elements().Get<GridSurface>(surface->GetElementId()).IsNull()) << "Grid surface has not been deleted";
        }

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_UnconstrainedExtended_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended();

    OrthogonalGridPtr orthogonalGridUnconstrainedExtended = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridUnconstrainedExtended.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    Placement3d translatedPlacement(orthogonalGridUnconstrainedExtended->GetPlacement());
    translatedPlacement.SetOrigin(newBaseOrigin);
    orthogonalGridUnconstrainedExtended->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridUnconstrainedExtended->Update().IsValid()) << "orthogonal grid translation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    bvector<DgnElementId> axesIds = orthogonalGridUnconstrainedExtended->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (23, 76, 0, 0), (23, 91, 0), (23, 106, 0), (23, 121, 0), (23, 136, 0).
    // for vertical elements placement origins should be: (23, 76, 0), (33, 76, 0), (43, 76, 0), (53, 76, 0)
    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin())) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 91, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin())) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 106, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin())) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 121, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin())) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 136, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin())) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin())) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(33, 76, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin())) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(43, 76, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin())) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(53, 76, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin())) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(-msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_UnconstrainedExtended_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended();

    OrthogonalGridPtr orthogonalGridUnconstrainedExtended = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridUnconstrainedExtended.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 6; // 30 deg
    Placement3d translatedPlacement(orthogonalGridUnconstrainedExtended->GetPlacement());
    translatedPlacement.GetAnglesR().SetYaw(AngleInDegrees::FromRadians(newAngle));
    orthogonalGridUnconstrainedExtended->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridUnconstrainedExtended->Update().IsValid()) << "orthogonal grid translation+rotation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    bvector<DgnElementId> axesIds = orthogonalGridUnconstrainedExtended->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (0, 0, 0, 0), (-7.5, 13, 0), (-15, 26, 0), (-22.5, 39, 0), (-30, 52, 0).
    // for vertical elements placement origins should be: (0, 0, 0), (8.7, 5, 0), (17.3, 10, 0), (26, 15, 0).
    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-7.5, 12.990381, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-15, 25.980762, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-22.5, 38.971143, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-30, 51.961524, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(8.660254, 5, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(17.320508, 10, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(25.980762, 15, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle - msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Constrained_CreatedAndDeleted)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPtr orthogonalGridConstrained = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    DPlane3d gridPlane = orthogonalGridConstrained->GetPlane();
    ASSERT_TRUE(gridPlane.origin.AlmostEqual({ 0, 0, 0 })) << "Grid plane origin is incorrect";
    ASSERT_TRUE(gridPlane.normal.AlmostEqual(DVec3d::From(0, 0, 1))) << "Grid plane normal is incorrect";

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    GridPtr thisGrid = Grid::TryGet(db, db.Elements().GetRootSubject()->GetElementId(), "Constrained Grid");
    ASSERT_TRUE(thisGrid.IsValid()) << "Failed to get created grid";
    ASSERT_EQ(orthogonalGridConstrained->GetElementId(), thisGrid->GetElementId()) << "Loaded grid's element id is incorrect";

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(orthogonalGridConstrained.IsValid()) << "Failed to create orthogonal grid";

    ASSERT_TRUE(orthogonalGridConstrained->GetSurfacesModel().IsValid()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridConstrained->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";

    ASSERT_TRUE(0 == std::strcmp("Constrained Grid", orthogonalGridConstrained->GetName())) << "Grid name is not correct";

    /////////////////////////////////////////////////////////////
    // Check if axes are valid and have correct number of elements
    /////////////////////////////////////////////////////////////
    ElementIterator axesIterator = orthogonalGridConstrained->MakeAxesIterator();
    bvector<DgnElementId> axesIds = axesIterator.BuildIdList<DgnElementId>();
    int numAxes = axesIds.size();
    ASSERT_TRUE(numAxes == 2) << "incorrect number of axes in orthogonalGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);
    ASSERT_TRUE(verticalAxis.IsValid()) << "vertical axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(5, numHorizontal) << "incorrect number of elements in horizontal axis";

    int numVertical = verticalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(4, numVertical) << "incorrect number of elements in vertical axis";

    /////////////////////////////////////////////////////////////
    // Check if axes elements are valid
    /////////////////////////////////////////////////////////////
    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr plane = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        ASSERT_TRUE(plane.IsValid()) << "horizontal element invalid";
        horizontalElements.push_back(plane);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr plane = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        ASSERT_TRUE(plane.IsValid()) << "vertical element invalid";
        verticalElements.push_back(plane);
        }

    /////////////////////////////////////////////////////////////
    // Check if axes elements parallel and perpendicular
    /////////////////////////////////////////////////////////////
    // for any two elements e_1 and e_2 : if both elements are in same axis, they must be parallel, otherwise they must be perpendicular
    bvector<GridPlanarSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), horizontalElements.begin(), horizontalElements.end());
    allElements.insert(allElements.end(), verticalElements.begin(), verticalElements.end());

    for (GridPlanarSurfaceCPtr firstElem : allElements)
        {
        for (GridPlanarSurfaceCPtr secondElem : allElements)
            {
            if (firstElem->GetElementId() == secondElem->GetElementId())
                continue;

            bool firstInHorizontal = horizontalElements.end() != std::find_if(horizontalElements.begin(), horizontalElements.end(), [&](auto plane) {return firstElem->GetElementId() == plane->GetElementId(); });
            bool secondInHorizontal = horizontalElements.end() != std::find_if(horizontalElements.begin(), horizontalElements.end(), [&](auto plane) {return secondElem->GetElementId() == plane->GetElementId(); });

            DVec3d firstNormal = firstElem->GetPlane().normal;
            DVec3d secondNormal = secondElem->GetPlane().normal;

            if (firstInHorizontal == secondInHorizontal)
                {
                ASSERT_TRUE(firstNormal.IsParallelTo(secondNormal)) << "Grid elements in same axes are not parallel";
                }
            else
                {
                ASSERT_TRUE(firstNormal.IsPerpendicularTo(secondNormal)) << "Grid elements in different axes are not perpendicular";
                }
            }
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct placement
    /////////////////////////////////////////////////////////////
    // for horizontal elements placement origins should be: (0, 0, 0), (0, 15, 0), (0, 30, 0), (0, 45, 0), (0, 60, 0).
    // for vertical elements placement origins should be: (0, 0, 0), (10, 0, 0), (20, 0, 0), (30, 0, 0)
    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin())) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 15, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin())) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 30, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin())) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 45, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin())) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 60, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin())) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin())) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(10, 0, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin())) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(20, 0, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin())) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(30, 0, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin())) << "Vertical plane 3 origin is incorrect";

    // all horizontal elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        double length = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetLength(length)) << "Grid surface length should be accessible";
        ASSERT_EQ(50, length) << "Grid surface length is incorrect";

        double height = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetHeight(height)) << "Grid surface height should be accessible";
        ASSERT_EQ(70, height) << "Grid surface height is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid is deleted correctly
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(DgnDbStatus::Success, orthogonalGridConstrained->Delete()) << "Error in deleting grid";
    ASSERT_TRUE(db.Elements().Get<OrthogonalGrid>(orthogonalGridConstrained->GetElementId()).IsNull()) << "Grid has not been deleted";

    for (DgnElementId axisId : axesIds)
        {
        ASSERT_TRUE(db.Elements().Get<GridAxis>(axisId).IsNull()) << "Grid axis has not been deleted";
        }

    for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(db.Elements().Get<GridSurface>(surface->GetElementId()).IsNull()) << "Grid surface has not been deleted";
        }

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Constrained_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPtr orthogonalGridConstrained = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridConstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    Placement3d translatedPlacement(orthogonalGridConstrained->GetPlacement());
    translatedPlacement.SetOrigin(newBaseOrigin);
    orthogonalGridConstrained->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridConstrained->Update().IsValid()) << "orthogonal grid translation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    bvector<DgnElementId> axesIds = orthogonalGridConstrained->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (23, 76, 0, 0), (23, 91, 0), (23, 106, 0), (23, 121, 0), (23, 136, 0).
    // for vertical elements placement origins should be: (23, 76, 0), (33, 76, 0), (43, 76, 0), (53, 76, 0)
    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin())) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 91, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin())) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 106, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin())) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 121, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin())) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 136, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin())) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin())) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(33, 76, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin())) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(43, 76, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin())) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(53, 76, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin())) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(-msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Constrained_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPtr orthogonalGridConstrained = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridConstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 6; // 30 deg
    Placement3d translatedPlacement(orthogonalGridConstrained->GetPlacement());
    translatedPlacement.GetAnglesR().SetYaw(AngleInDegrees::FromRadians(newAngle));
    orthogonalGridConstrained->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridConstrained->Update().IsValid()) << "orthogonal grid rotation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    double existingAngle;
    ASSERT_EQ(newAngle, orthogonalGridConstrained->GetPlacement().GetAngles().GetYaw().Radians()) << "Grid's rotation angle is incorrect";
    
    bvector<DgnElementId> axesIds = orthogonalGridConstrained->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (0, 0, 0, 0), (-7.5, 13, 0), (-15, 26, 0), (-22.5, 39, 0), (-30, 52, 0).
    // for vertical elements placement origins should be: (0, 0, 0), (8.7, 5, 0), (17.3, 10, 0), (26, 15, 0).
    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-7.5, 12.990381, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-15, 25.980762, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-22.5, 38.971143, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-30, 51.961524, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(8.660254, 5, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(17.320508, 10, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(25.980762, 15, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle - msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Constrained_PlacementCorrectAfterTranslationAndRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPtr orthogonalGridConstrained = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridConstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    double newAngle = msGeomConst_pi / 6; // 30 deg
    Placement3d translatedPlacement(orthogonalGridConstrained->GetPlacement());
    translatedPlacement.GetAnglesR().SetYaw(AngleInDegrees::FromRadians(newAngle));
    translatedPlacement.SetOrigin(newBaseOrigin);
    orthogonalGridConstrained->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridConstrained->Update().IsValid()) << "orthogonal grid translation+rotation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    double existingAngle;
    ASSERT_EQ(newAngle, orthogonalGridConstrained->GetPlacement().GetAngles().GetYaw().Radians()) << "Grid's rotation angle is incorrect";

    bvector<DgnElementId> axesIds = orthogonalGridConstrained->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (23, 76, 0), (15.5, 89, 0), (8, 102, 0), (0.5, 115, 0), (-7, 128, 0).
    // for vertical elements placement origins should be: (23, 76, 0), (31.7, 81, 0), (40.3, 86, 0), (49, 91, 0).
    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(15.5, 88.990381, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(8, 101.980762, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0.5, 114.971143, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-7, 127.961524, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(31.660254, 81, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(40.320508, 86, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(48.980762, 91, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle - msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_ConstrainedExtended_CreatedAndDeleted)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended();

    OrthogonalGridPtr orthogonalGridConstrainedExtended = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    DPlane3d gridPlane = orthogonalGridConstrainedExtended->GetPlane();
    ASSERT_TRUE(gridPlane.origin.AlmostEqual({ 0, 0, 0 })) << "Grid plane origin is incorrect";
    ASSERT_TRUE(gridPlane.normal.AlmostEqual(DVec3d::From(0, 0, 1))) << "Grid plane normal is incorrect";

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    GridPtr thisGrid = Grid::TryGet(db, db.Elements().GetRootSubject()->GetElementId(), "Constrained Grid");
    ASSERT_TRUE(thisGrid.IsValid()) << "Failed to get created grid";
    ASSERT_EQ(orthogonalGridConstrainedExtended->GetElementId(), thisGrid->GetElementId()) << "Loaded grid's element id is incorrect";

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(orthogonalGridConstrainedExtended.IsValid()) << "Failed to create orthogonal grid";

    ASSERT_TRUE(orthogonalGridConstrainedExtended->GetSurfacesModel().IsValid()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridConstrainedExtended->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";

    ASSERT_TRUE(0 == std::strcmp("Constrained Grid", orthogonalGridConstrainedExtended->GetName())) << "Grid name is not correct";

    /////////////////////////////////////////////////////////////
    // Check if axes are valid and have correct number of elements
    /////////////////////////////////////////////////////////////
    ElementIterator axesIterator = orthogonalGridConstrainedExtended->MakeAxesIterator();
    bvector<DgnElementId> axesIds = axesIterator.BuildIdList<DgnElementId>();
    int numAxes = axesIds.size();
    ASSERT_TRUE(numAxes == 2) << "incorrect number of axes in orthogonalGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);
    ASSERT_TRUE(verticalAxis.IsValid()) << "vertical axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(5, numHorizontal) << "incorrect number of elements in horizontal axis";

    int numVertical = verticalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(4, numVertical) << "incorrect number of elements in vertical axis";

    /////////////////////////////////////////////////////////////
    // Check if axes elements are valid
    /////////////////////////////////////////////////////////////
    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr plane = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        ASSERT_TRUE(plane.IsValid()) << "horizontal element invalid";
        horizontalElements.push_back(plane);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr plane = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        ASSERT_TRUE(plane.IsValid()) << "vertical element invalid";
        verticalElements.push_back(plane);
        }

    /////////////////////////////////////////////////////////////
    // Check if axes elements parallel and perpendicular
    /////////////////////////////////////////////////////////////
    // for any two elements e_1 and e_2 : if both elements are in same axis, they must be parallel, otherwise they must be perpendicular
    bvector<GridPlanarSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), horizontalElements.begin(), horizontalElements.end());
    allElements.insert(allElements.end(), verticalElements.begin(), verticalElements.end());

    for (GridPlanarSurfaceCPtr firstElem : allElements)
        {
        for (GridPlanarSurfaceCPtr secondElem : allElements)
            {
            if (firstElem->GetElementId() == secondElem->GetElementId())
                continue;

            bool firstInHorizontal = horizontalElements.end() != std::find_if(horizontalElements.begin(), horizontalElements.end(), [&](auto plane) {return firstElem->GetElementId() == plane->GetElementId(); });
            bool secondInHorizontal = horizontalElements.end() != std::find_if(horizontalElements.begin(), horizontalElements.end(), [&](auto plane) {return secondElem->GetElementId() == plane->GetElementId(); });

            DVec3d firstNormal = firstElem->GetPlane().normal;
            DVec3d secondNormal = secondElem->GetPlane().normal;

            if (firstInHorizontal == secondInHorizontal)
                {
                ASSERT_TRUE(firstNormal.IsParallelTo(secondNormal)) << "Grid elements in same axes are not parallel";
                }
            else
                {
                ASSERT_TRUE(firstNormal.IsPerpendicularTo(secondNormal)) << "Grid elements in different axes are not perpendicular";
                }
            }
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct placement
    /////////////////////////////////////////////////////////////
    // for horizontal elements placement origins should be: (0, 0, 0), (0, 15, 0), (0, 30, 0), (0, 45, 0), (0, 60, 0).
    // for vertical elements placement origins should be: (0, 0, 0), (10, 0, 0), (20, 0, 0), (30, 0, 0)
    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin())) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 15, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin())) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 30, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin())) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 45, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin())) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(0, 60, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin())) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin())) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(10, 0, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin())) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(20, 0, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin())) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(30, 0, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin())) << "Vertical plane 3 origin is incorrect";

    // all horizontal elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        double length = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetLength(length)) << "Grid surface length should be accessible";
        ASSERT_EQ(50, length) << "Grid surface length is incorrect";

        double height = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetHeight(height)) << "Grid surface height should be accessible";
        ASSERT_EQ(70, height) << "Grid surface height is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid is deleted correctly
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(DgnDbStatus::Success, orthogonalGridConstrainedExtended->Delete()) << "Error in deleting grid";
    ASSERT_TRUE(db.Elements().Get<OrthogonalGrid>(orthogonalGridConstrainedExtended->GetElementId()).IsNull()) << "Grid has not been deleted";

    for (DgnElementId axisId : axesIds)
        {
        ASSERT_TRUE(db.Elements().Get<GridAxis>(axisId).IsNull()) << "Grid axis has not been deleted";
        }

    for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(db.Elements().Get<GridSurface>(surface->GetElementId()).IsNull()) << "Grid surface has not been deleted";
        }

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_ConstrainedExtended_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended();

    OrthogonalGridPtr orthogonalGridConstrainedExtended = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridConstrainedExtended.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    Placement3d translatedPlacement(orthogonalGridConstrainedExtended->GetPlacement());
    translatedPlacement.SetOrigin(newBaseOrigin);
    orthogonalGridConstrainedExtended->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridConstrainedExtended->Update().IsValid()) << "orthogonal grid translation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    bvector<DgnElementId> axesIds = orthogonalGridConstrainedExtended->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (23, 76, 0, 0), (23, 91, 0), (23, 106, 0), (23, 121, 0), (23, 136, 0).
    // for vertical elements placement origins should be: (23, 76, 0), (33, 76, 0), (43, 76, 0), (53, 76, 0)
    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin())) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 91, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin())) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 106, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin())) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 121, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin())) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(23, 136, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin())) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(23, 76, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin())) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(33, 76, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin())) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(43, 76, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin())) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(53, 76, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin())) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(-msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_ConstrainedExtended_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended();

    OrthogonalGridPtr orthogonalGridConstrainedExtended = OrthogonalGrid::CreateAndInsertWithSurfaces(createParams, 5, 4);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (orthogonalGridConstrainedExtended.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 6; // 30 deg
    Placement3d translatedPlacement(orthogonalGridConstrainedExtended->GetPlacement());
    translatedPlacement.GetAnglesR().SetYaw(AngleInDegrees::FromRadians(newAngle));
    orthogonalGridConstrainedExtended->SetPlacement(translatedPlacement);
    ASSERT_TRUE(orthogonalGridConstrainedExtended->Update().IsValid()) << "orthogonal grid translation+rotation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    double existingAngle;
    ASSERT_EQ(newAngle, orthogonalGridConstrainedExtended->GetPlacement().GetAngles().GetYaw().Radians()) << "Grid's rotation angle is incorrect";

    bvector<DgnElementId> axesIds = orthogonalGridConstrainedExtended->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
        return;
        }

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> horizontalElementIds = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(verticalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
            return;
            }

        verticalElements.push_back(surface);
        }

    // for horizontal elements placement origins should be: (0, 0, 0, 0), (-7.5, 13, 0), (-15, 26, 0), (-22.5, 39, 0), (-30, 52, 0).
    // for vertical elements placement origins should be: (0, 0, 0), (8.7, 5, 0), (17.3, 10, 0), (26, 15, 0).
    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(horizontalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-7.5, 12.990381, 0).AlmostEqual(horizontalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-15, 25.980762, 0).AlmostEqual(horizontalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-22.5, 38.971143, 0).AlmostEqual(horizontalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 3 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(-30, 51.961524, 0).AlmostEqual(horizontalElements[4]->GetPlacement().GetOrigin(), 0.1)) << "Horizontal plane 4 origin is incorrect";

    ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(verticalElements[0]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 0 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(8.660254, 5, 0).AlmostEqual(verticalElements[1]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 1 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(17.320508, 10, 0).AlmostEqual(verticalElements[2]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 2 origin is incorrect";
    ASSERT_TRUE(DPoint3d::From(25.980762, 15, 0).AlmostEqual(verticalElements[3]->GetPlacement().GetOrigin(), 0.1)) << "Vertical plane 3 origin is incorrect";

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle - msGeomConst_piOver2, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }
  
//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGrid_Empty_CreatedAndDeleted)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    RadialGrid::CreateParams params(*m_model, m_model->GetModeledElementId(), "empty radial grid", 0, 0, 0, 0, 0 , 0, 0, 10);

    RadialGridPtr empty = RadialGrid::Create(params);
    ASSERT_TRUE(empty.IsValid()) << "Failed to create empty radial grid";
    ASSERT_TRUE(empty->Insert().IsValid()) << "Failed to insert empty grid";

    ASSERT_EQ(DgnDbStatus::Success, empty->Delete());

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGrid_CreatedAndDeleted)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    RadialGrid::CreateParams createParams = GetTestDefaultCreateParamsForRadialGrid();

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsertWithSurfaces(createParams, 7, 5);

    DPlane3d gridPlane = radialGrid->GetPlane();
    ASSERT_TRUE(gridPlane.origin.AlmostEqual({ 0, 0, 0 })) << "Grid plane origin is incorrect";
    ASSERT_TRUE(gridPlane.normal.AlmostEqual(DVec3d::From(0, 0, 1))) << "Grid plane normal is incorrect";

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    GridPtr thisGrid = Grid::TryGet(db, db.Elements().GetRootSubject()->GetElementId(), "Radial Grid");
    ASSERT_TRUE(thisGrid.IsValid()) << "Failed to get created grid";
    ASSERT_EQ(radialGrid->GetElementId(), thisGrid->GetElementId()) << "Loaded grid's element id is incorrect";

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(radialGrid.IsValid()) << "Failed to create radial grid";

    ASSERT_TRUE(radialGrid->GetSurfacesModel().IsValid()) << "Failed to get radialGrid grid surfaces model";

    int numSurfaces = radialGrid->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(numSurfaces == 12) << "incorrect number of gridSurfaces in radialGrid";

    ASSERT_TRUE(0 == std::strcmp("Radial Grid", radialGrid->GetName())) << "Grid name is not correct";

    /////////////////////////////////////////////////////////////
    // Check if axes are valid and have correct number of elements
    /////////////////////////////////////////////////////////////
    ElementIterator axesIterator = radialGrid->MakeAxesIterator();
    bvector<DgnElementId> axesIds = axesIterator.BuildIdList<DgnElementId>();
    int numAxes = axesIds.size();
    ASSERT_TRUE(numAxes == 2) << "incorrect number of axes in radialGrid";

    GridAxisCPtr planeAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(planeAxis.IsValid()) << "plane axis is not present";

    GridAxisCPtr arcAxis = db.Elements().Get<GridAxis>(axesIds[1]);
    ASSERT_TRUE(arcAxis.IsValid()) << "arc axis is not present";

    int numPlanes = planeAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(7, numPlanes) << "incorrect number of elements in plane axis";

    int numArcs = arcAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(5, numArcs) << "incorrect number of elements in arc axis";

    /////////////////////////////////////////////////////////////
    // Check if axes elements are valid
    /////////////////////////////////////////////////////////////
    bvector<DgnElementId> planeElementIds = planeAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> planeElements;
    for (DgnElementId planeElementId : planeElementIds)
        {
        GridPlanarSurfaceCPtr plane = db.Elements().Get<GridPlanarSurface>(planeElementId);
        ASSERT_TRUE(plane.IsValid()) << "plane element invalid";
        planeElements.push_back(plane);
        }

    bvector<DgnElementId> arcElementIds = arcAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridArcSurfaceCPtr> arcElements;
    for (DgnElementId arcElementId : arcElementIds)
        {
        GridArcSurfaceCPtr arc = db.Elements().Get<GridArcSurface>(arcElementId);
        ASSERT_TRUE(arc.IsValid()) << "arc element invalid";
        arcElements.push_back(arc);
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct placement
    /////////////////////////////////////////////////////////////
    bvector<GridSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), planeElements.begin(), planeElements.end());
    allElements.insert(allElements.end(), arcElements.begin(), arcElements.end());
    
    // all elements placement origins should be: (0, 0, 0).
    for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(surface->GetPlacement().GetOrigin(), 0.1)) << "Grid surface origin is incorrect";
        }

    // all plane elements rotation angle should be i * iteration_angle = i * 7 * msGeomConst_pi / 180
    for (size_t i = 0; i < planeElements.size(); ++i)
        {
        GridPlanarSurfaceCPtr plane = planeElements[i];
        EXPECT_DOUBLE_EQ(i * 7 * msGeomConst_pi / 180, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    // all arc elements rotation angle should be 0
    for (GridArcSurfaceCPtr arc : arcElements)
        {
        EXPECT_DOUBLE_EQ(0, DgnGeometryUtils::PlacementToAngleXY(arc->GetPlacement())) << "Grid arc rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    // grid planes length should be 70.
    for (GridPlanarSurfaceCPtr plane : planeElements)
        {
        double length = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetLength(length)) << "Grid plane length should be accessible";
        ASSERT_EQ(70, length) << "Grid plane length is incorrect";
        }

    // i-th grid arcs length should be 2 * PI * r * ((theta) / 360)

    for (size_t i = 0; i < arcElements.size(); ++i)
        {
        GridArcSurfaceCPtr arc = arcElements[i];
        double r = (i + 1) * 13;
        double extendAngle = (2 * msGeomConst_piOver12);
        double theta = (7 * 7) * msGeomConst_pi / 180 + extendAngle;
        double expectedLength = r * theta;

        double length = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, arc->TryGetLength(length)) << "Grid arc length should be accessible";
        ASSERT_TRUE(std::fabs(expectedLength - length) < 0.1) << "Grid arc length is incorrect";
        }

    // grid surfaces height should be 50
    for (GridSurfaceCPtr surface : allElements)
        {
        double height = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, surface->TryGetHeight(height)) << "Grid surface height should be accessible";
        ASSERT_EQ(50, height) << "Grid surface height is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid is deleted correctly
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(DgnDbStatus::Success, radialGrid->Delete()) << "Error in deleting grid";
    ASSERT_TRUE(db.Elements().Get<Grid>(radialGrid->GetElementId()).IsNull()) << "Grid has not been deleted";

    for (DgnElementId axisId : axesIds)
        {
        ASSERT_TRUE(db.Elements().Get<GridAxis>(axisId).IsNull()) << "Grid axis has not been deleted";
        }

    for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(db.Elements().Get<GridSurface>(surface->GetElementId()).IsNull()) << "Grid surface has not been deleted";
        }

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGrid_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    RadialGrid::CreateParams createParams = GetTestDefaultCreateParamsForRadialGrid();

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsertWithSurfaces(createParams, 7, 5);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (radialGrid.IsNull())
        {
        ASSERT_TRUE(false) << "radial grid portion is invalid. See GridsTestFixture:RadialGrid_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(43, 57, 0);
    Placement3d translatedPlacement(radialGrid->GetPlacement());
    translatedPlacement.SetOrigin(newBaseOrigin);
    radialGrid->SetPlacement(translatedPlacement);
    ASSERT_TRUE(radialGrid->Update().IsValid()) << "radial grid translation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    bvector<DgnElementId> axesIds = radialGrid->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:RadialGrid_Created";
        return;
        }

    GridAxisCPtr planeAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr arcAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> planeElementIds = planeAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> planeElements;
    for (DgnElementId planeElementId : planeElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(planeElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:RadialGrid_Created";
            return;
            }

        planeElements.push_back(surface);
        }

    bvector<DgnElementId> arcElementIds = arcAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridArcSurfaceCPtr> arcElements;
    for (DgnElementId arcElementId : arcElementIds)
        {
        GridArcSurfaceCPtr surface = db.Elements().Get<GridArcSurface>(arcElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:RadialGrid_Created";
            return;
            }

        arcElements.push_back(surface);
        }

    bvector<GridSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), planeElements.begin(), planeElements.end());
    allElements.insert(allElements.end(), arcElements.begin(), arcElements.end());


    // all plane elements rotation angle should be i * iteration_angle = i * 7 * msGeomConst_pi / 180
    for (size_t i = 0; i < planeElements.size(); ++i)
        {
        GridPlanarSurfaceCPtr plane = planeElements[i];

        EXPECT_DOUBLE_EQ(i * 7 * msGeomConst_pi / 180, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    // all arc elements rotation angle should be 0
    for (GridArcSurfaceCPtr arc : arcElements)
        {
        EXPECT_DOUBLE_EQ(0, DgnGeometryUtils::PlacementToAngleXY(arc->GetPlacement())) << "Grid arc rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGrid_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    RadialGrid::CreateParams createParams = GetTestDefaultCreateParamsForRadialGrid();

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsertWithSurfaces(createParams, 7, 5);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (radialGrid.IsNull())
        {
        ASSERT_TRUE(false) << "radial grid portion is invalid. See GridsTestFixture:RadialGrid_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 4; // 45 DEG
    Placement3d translatedPlacement(radialGrid->GetPlacement());
    translatedPlacement.GetAnglesR().SetYaw(AngleInDegrees::FromRadians(newAngle));
    radialGrid->SetPlacement(translatedPlacement);
    ASSERT_TRUE(radialGrid->Update().IsValid()) << "radial grid rotation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    double existingAngle;
    ASSERT_EQ(newAngle, radialGrid->GetPlacement().GetAngles().GetYaw().Radians()) << "Grid's rotation angle is incorrect";

    bvector<DgnElementId> axesIds = radialGrid->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:RadialGrid_Created";
        return;
        }

    GridAxisCPtr planeAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr arcAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> planeElementIds = planeAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> planeElements;
    for (DgnElementId planeElementId : planeElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(planeElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:RadialGrid_Created";
            return;
            }

        planeElements.push_back(surface);
        }

    bvector<DgnElementId> arcElementIds = arcAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridArcSurfaceCPtr> arcElements;
    for (DgnElementId arcElementId : arcElementIds)
        {
        GridArcSurfaceCPtr surface = db.Elements().Get<GridArcSurface>(arcElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:RadialGrid_Created";
            return;
            }

        arcElements.push_back(surface);
        }

    bvector<GridSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), planeElements.begin(), planeElements.end());
    allElements.insert(allElements.end(), arcElements.begin(), arcElements.end());

    // all elements placement origins should be: (0, 0, 0).
    for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(DPoint3d::From(0, 0, 0).AlmostEqual(surface->GetPlacement().GetOrigin(), 0.1)) << "Grid surface origin is incorrect";
        }

    // all plane elements rotation angle should be newAngle + i * iteration_angle = msGeomConst_pi / 4 + i * 7 * msGeomConst_pi / 180
    for (size_t i = 0; i < planeElements.size(); ++i)
        {
        GridPlanarSurfaceCPtr plane = planeElements[i];

        EXPECT_DOUBLE_EQ(msGeomConst_pi / 4 + i * 7 * msGeomConst_pi / 180, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    // all arc elements rotation angle should be msGeomConst_pi / 4
    for (GridArcSurfaceCPtr arc : arcElements)
        {
        EXPECT_DOUBLE_EQ(msGeomConst_pi / 4, DgnGeometryUtils::PlacementToAngleXY(arc->GetPlacement())) << "Grid arc rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGrid_PlacementCorrectAfterTranslationAndRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    RadialGrid::CreateParams createParams = GetTestDefaultCreateParamsForRadialGrid();

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsertWithSurfaces(createParams, 7, 5);

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    if (radialGrid.IsNull())
        {
        ASSERT_TRUE(false) << "radial grid portion is invalid. See GridsTestFixture:RadialGrid_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(43, 57, 0);
    double newAngle = msGeomConst_pi / 4; // 45 DEG
    Placement3d translatedPlacement(radialGrid->GetPlacement());
    translatedPlacement.GetAnglesR().SetYaw(AngleInDegrees::FromRadians(newAngle));
    translatedPlacement.SetOrigin(newBaseOrigin);
    radialGrid->SetPlacement(translatedPlacement);
    ASSERT_TRUE(radialGrid->Update().IsValid()) << "radial grid translation+rotation was not successful";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    ASSERT_EQ(newAngle, radialGrid->GetPlacement().GetAngles().GetYaw().Radians()) << "Grid's rotation angle is incorrect";

    bvector<DgnElementId> axesIds = radialGrid->MakeAxesIterator().BuildIdList<DgnElementId>();
    if (2 != axesIds.size())
        {
        ASSERT_TRUE(false) << "Grid axes number is incorrect. See GridsTestFixture:RadialGrid_Created";
        return;
        }

    GridAxisCPtr planeAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    GridAxisCPtr arcAxis = db.Elements().Get<GridAxis>(axesIds[1]);

    bvector<DgnElementId> planeElementIds = planeAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlanarSurfaceCPtr> planeElements;
    for (DgnElementId planeElementId : planeElementIds)
        {
        GridPlanarSurfaceCPtr surface = db.Elements().Get<GridPlanarSurface>(planeElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:RadialGrid_Created";
            return;
            }

        planeElements.push_back(surface);
        }

    bvector<DgnElementId> arcElementIds = arcAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridArcSurfaceCPtr> arcElements;
    for (DgnElementId arcElementId : arcElementIds)
        {
        GridArcSurfaceCPtr surface = db.Elements().Get<GridArcSurface>(arcElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:RadialGrid_Created";
            return;
            }

        arcElements.push_back(surface);
        }

    bvector<GridSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), planeElements.begin(), planeElements.end());
    allElements.insert(allElements.end(), arcElements.begin(), arcElements.end());

    // all elements placement origins should be: (43, 57, 0).
    /*for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(DPoint3d::From(43, 57, 0).AlmostEqual(surface->GetPlacement().GetOrigin(), 0.1)) << "Grid surface origin is incorrect";
        }*/

    // all plane elements rotation angle should be newAngle + i * iteration_angle = msGeomConst_pi / 4 + i * 7 * msGeomConst_pi / 180
    for (size_t i = 0; i < planeElements.size(); ++i)
        {
        GridPlanarSurfaceCPtr plane = planeElements[i];

        EXPECT_DOUBLE_EQ(msGeomConst_pi / 4 + i * 7 * msGeomConst_pi / 180, DgnGeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    // all arc elements rotation angle should be msGeomConst_pi / 4
    for (GridArcSurfaceCPtr arc : arcElements)
        {
        EXPECT_DOUBLE_EQ(msGeomConst_pi / 4, DgnGeometryUtils::PlacementToAngleXY(arc->GetPlacement())) << "Grid arc rotation angle is incorrect";
        }
    }    
    
//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GridsTestFixture, SketchGrid_CreatedAndDeleted)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    SketchGridPtr sketchGrid = SketchGrid::Create(*m_model.get(), m_model->GetModeledElementId(), "Sketch Grid", 0.0, 10.0);

    DPlane3d gridPlane = sketchGrid->GetPlane();
    ASSERT_TRUE(gridPlane.origin.AlmostEqual({ 0, 0, 0 })) << "Grid plane origin is incorrect";
    ASSERT_TRUE(gridPlane.normal.AlmostEqual(DVec3d::From(0, 0, 1))) << "Grid plane normal is incorrect";

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(sketchGrid.IsValid()) << "Failed to create sketch grid";
    ASSERT_TRUE(sketchGrid->Insert().IsValid()) << "Failed to insert sketch grid";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    GridPtr thisGrid = Grid::TryGet(db, m_model->GetModeledElementId(), "Sketch Grid");
    ASSERT_TRUE(thisGrid.IsValid()) << "Failed to get created grid";
    ASSERT_EQ(sketchGrid->GetElementId(), thisGrid->GetElementId()) << "Loaded grid's element id is incorrect";

    ASSERT_TRUE(sketchGrid->GetSurfacesModel().IsValid()) << "Failed to get sketch grid surfaces model";

    int numSurfaces = sketchGrid->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(numSurfaces == 0) << "incorrect number of gridSurfaces in sketchGrid";

    ASSERT_TRUE(0 == std::strcmp("Sketch Grid", sketchGrid->GetName())) << "Grid name is not correct";

    /////////////////////////////////////////////////////////////
    // Check if axes are valid and have correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(0, sketchGrid->MakeAxesIterator().BuildIdList<DgnElementId>().size()) << "new sketch grid should contain no elements";

    Grids::GeneralGridAxisPtr gridAxis = GeneralGridAxis::CreateAndInsert(*sketchGrid);

    ASSERT_TRUE(gridAxis.IsValid()) << "Failed to create sketch grid axis";

    ASSERT_EQ(0, gridAxis->MakeIterator().BuildIdList<DgnElementId>().size()) << "New axis should contain no elements";

    /////////////////////////////////////////////////////////////
    // Check if valid grid plane surfaces can be added to sketch grid
    /////////////////////////////////////////////////////////////

    SketchLineGridSurface::CreateParams params(*sketchGrid->GetSurfacesModel().get(), *gridAxis, 0.0, 90.0, DPoint2d::From(50, 20), DPoint2d::From(50, 70));

    GridPlanarSurfacePtr plane = SketchLineGridSurface::Create(params);

    ASSERT_TRUE(plane.IsValid()) << "Failed to create grid plane surface";

    DPlane3d surfacePlane = plane->GetPlane();
    ASSERT_TRUE(surfacePlane.origin.AlmostEqual({ 50, 20, 0 })) << "plane's origin is incorrect";

    DVec3d expectedNormal = DVec3d::FromCrossProduct
    (
        DVec3d::FromStartEnd
        (
            DPoint3d::From(50, 20, 0),
            DPoint3d::From(50, 70, 0)
        ),
        DVec3d::FromStartEnd
        (
            DPoint3d::From(50, 20, 0),
            DPoint3d::From(50, 20, 90)
        )
    );

    ASSERT_TRUE(surfacePlane.normal.IsParallelTo(expectedNormal)) << "plane's normal is incorrect";

    plane->Insert();
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    bvector<DgnElementId> axisElementsAfterFirstPlaneInsert = gridAxis->MakeIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(1, axisElementsAfterFirstPlaneInsert.size()) << "Axis should contain one element now";
    ASSERT_TRUE(axisElementsAfterFirstPlaneInsert.back().IsValid()) << "Axis' element ids should be valid";
    ASSERT_EQ(axisElementsAfterFirstPlaneInsert.back(), plane->GetElementId()) << "The axis element should be the inserted grid plane";

    bvector<DgnElementId> gridElementsAfterFirstPlaneInsert = sketchGrid->MakeIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(1, gridElementsAfterFirstPlaneInsert.size());
    ASSERT_TRUE(gridElementsAfterFirstPlaneInsert.back().IsValid()) << "Grid's element ids should be valid";
    ASSERT_EQ(gridElementsAfterFirstPlaneInsert.back(), plane->GetElementId()) << "The grid element should be the inserted grid plane";

    /////////////////////////////////////////////////////////////
    // Check if valid grid arc surfaces can be added to sketch grid
    /////////////////////////////////////////////////////////////
    //DgnExtrusionDetail arcExtDetail = GeometryUtils::CreateArcExtrusionDetail(10 /*radius*/, msGeomConst_pi /*base angle*/, 10 /*height*/, 0 /*extend length*/);

    SketchArcGridSurface::CreateParams arcParams(*sketchGrid->GetSurfacesModel().get(), *gridAxis, 0, 10, GeometryUtils::CreateArc(10 /*radius*/, msGeomConst_pi /*base angle*/, 0 /*extend length*/));
    GridArcSurfacePtr arc = SketchArcGridSurface::Create(arcParams);

    ASSERT_TRUE(arc.IsValid()) << "Failed to create grid plane surface";
    arc->Insert();
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    bvector<DgnElementId> axisElementsAfterArcInsert = gridAxis->MakeIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(2, axisElementsAfterArcInsert.size()) << "Axis should contain two elements now";
    ASSERT_TRUE(axisElementsAfterArcInsert.back().IsValid()) << "Axis' element ids should be valid";
    ASSERT_EQ(axisElementsAfterArcInsert.back(), arc->GetElementId()) << "The axis element should be the inserted grid arc";

    bvector<DgnElementId> gridElementsAfterArcInsert = sketchGrid->MakeIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(2, gridElementsAfterArcInsert.size());
    ASSERT_TRUE(gridElementsAfterArcInsert.back().IsValid()) << "Grid's element ids should be valid";
    ASSERT_EQ(gridElementsAfterArcInsert.back(), arc->GetElementId()) << "The grid element should be the inserted grid arc";

    /////////////////////////////////////////////////////////////
    // Check if valid grid spline surfaces can be added to sketch grid
    /////////////////////////////////////////////////////////////
    ICurvePrimitivePtr splinePrimitive = GeometryUtils::CreateSplinePrimitive({ { 0, 0, 0 },{ 10, 0, 0 },{ 0, 10, 0 } } /*poles*/);
    SketchSplineGridSurface::CreateParams splineParams(*sketchGrid->GetSurfacesModel().get(), *gridAxis, 0.0, 10.0, *splinePrimitive);
    GridSplineSurfacePtr spline = SketchSplineGridSurface::Create(splineParams);

    ASSERT_TRUE(spline.IsValid()) << "Failed to create grid spline surface";
    spline->Insert();
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    bvector<DgnElementId> axisElementsAfterSplineInsert = gridAxis->MakeIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(3, axisElementsAfterSplineInsert.size()) << "Axis should contain two elements now";
    ASSERT_TRUE(axisElementsAfterSplineInsert.back().IsValid()) << "Axis' element ids should be valid";
    ASSERT_EQ(axisElementsAfterSplineInsert.back(), spline->GetElementId()) << "The axis element should be the inserted grid spline";

    bvector<DgnElementId> gridElementsAfterSplineInsert = sketchGrid->MakeIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(3, gridElementsAfterSplineInsert.size());
    ASSERT_TRUE(gridElementsAfterSplineInsert.back().IsValid()) << "Grid's element ids should be valid";
    ASSERT_EQ(gridElementsAfterSplineInsert.back(), spline->GetElementId()) << "The grid element should be the inserted grid spline";

    /////////////////////////////////////////////////////////////
    // Check if grid is deleted correctly
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(DgnDbStatus::Success, sketchGrid->Delete()) << "Error in deleting grid";
    ASSERT_TRUE(db.Elements().Get<Grid>(sketchGrid->GetElementId()).IsNull()) << "Grid has not been deleted";
    ASSERT_TRUE(db.Elements().Get<GridAxis>(gridAxis->GetElementId()).IsNull()) << "Grid axis has not been deleted";

    for (DgnElementId surfaceId : gridElementsAfterSplineInsert)
        {
        ASSERT_TRUE(db.Elements().Get<GridSurface>(surfaceId).IsNull()) << "Grid surface has not been deleted";
        }

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F (GridsTestFixture, InsertHandlerCreatedElements)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();
    db.BriefcaseManager().StartBulkOperation();

    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), GRIDS_CATEGORY_CODE_Uncategorized);
    {
    // Check grid plane create from handler
    SketchLineGridSurfaceHandler& planeHandler = SketchLineGridSurfaceHandler::GetHandler();
    DgnClassId planeClassId = db.Domains().GetClassId(planeHandler);
    DgnElement::CreateParams planeParams(db, m_model->GetModelId(), planeClassId);

    GridPlanarSurfacePtr invalidGridPlane_FromHandler = dynamic_cast<GridPlanarSurface *>(planeHandler.Create(planeParams).get());
    ASSERT_TRUE(invalidGridPlane_FromHandler.IsValid()) << "element created from handler shouldn't be a nullptr";

    
    invalidGridPlane_FromHandler->SetCategoryId(categoryId);
    ASSERT_TRUE(invalidGridPlane_FromHandler->Insert().IsNull()) << "Element insertion should fail";

    ASSERT_TRUE(!invalidGridPlane_FromHandler->GetElementId().IsValid()) << "element id should be invalid";
    }

    {
    // Check grid arc create from handler
    SketchArcGridSurfaceHandler& arcHandler = SketchArcGridSurfaceHandler::GetHandler();
    DgnClassId arcClassId = db.Domains().GetClassId(arcHandler);
    DgnElement::CreateParams arcParams(db, m_model->GetModelId(), arcClassId);

    GridArcSurfacePtr invalidGridArc_FromHandler = dynamic_cast<GridArcSurface *>(arcHandler.Create(arcParams).get());
    ASSERT_TRUE(invalidGridArc_FromHandler.IsValid()) << "element created from handler shouldn't be a nullptr";

    invalidGridArc_FromHandler->SetCategoryId(categoryId);
    ASSERT_TRUE(invalidGridArc_FromHandler->Insert().IsNull()) << "Element insertion should fail";

    ASSERT_TRUE(!invalidGridArc_FromHandler->GetElementId().IsValid()) << "element id should be invalid";
    }

    {
    // Check grid spline create from handler
    SketchSplineGridSurfaceHandler& splineHandler = SketchSplineGridSurfaceHandler::GetHandler();
    DgnClassId splineClassId = db.Domains().GetClassId(splineHandler);
    DgnElement::CreateParams splineParams(db, m_model->GetModelId(), splineClassId);

    GridSplineSurfacePtr invalidGridSpline_FromHandler = dynamic_cast<GridSplineSurface *>(splineHandler.Create(splineParams).get());
    ASSERT_TRUE(invalidGridSpline_FromHandler.IsValid()) << "element created from handler shouldn't be a nullptr";

    invalidGridSpline_FromHandler->SetCategoryId(categoryId);
    ASSERT_TRUE(invalidGridSpline_FromHandler->Insert().IsNull()) << "Element insertion should fail";

    ASSERT_TRUE(!invalidGridSpline_FromHandler->GetElementId().IsValid()) << "element id should be invalid";
    }

    {
    // create new definition model
    GeneralGridAxisHandler& handler = GeneralGridAxisHandler::GetHandler ();
    DgnClassId classId = db.Domains ().GetClassId (handler);
    DgnElement::CreateParams params (db, m_model->GetModelId (), classId);

    DgnElementPtr element = handler.Create (params);
    element->Insert ();

    ASSERT_TRUE(!element->GetElementId().IsValid()) << "should fail to insert axis created via handler";
    }
    db.SaveChanges ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F (GridsTestFixture, InsertUpdateInvalidGeometrySurfaces)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();
    db.BriefcaseManager().StartBulkOperation();

    SketchGridPtr grid = SketchGrid::Create (*m_model, m_model->GetModeledElementId(), "SketchGrid-1", 0.0, 10.0);
    grid->Insert ();
    GeneralGridAxisPtr axis1 = GeneralGridAxis::CreateAndInsert (*grid);

    /////////////////////////////////////////////////////////////
    // Check if invalid grid plane surfaces can't be added to sketch grid
    /////////////////////////////////////////////////////////////
    // Check grid plane from a zero-length line
    SketchLineGridSurface::CreateParams lineParams(*grid->GetSurfacesModel(), *axis1, 0.0, 10.0, DPoint2d::FromZero(), DPoint2d::FromZero());
    GridPlanarSurfacePtr invalidGridPlane_Empty = SketchLineGridSurface::Create(lineParams);

    ASSERT_TRUE(invalidGridPlane_Empty->Insert().IsNull()) << "Invalid grid plane surface has been created";

    /////////////////////////////////////////////////////////////
    // Check if invalid grid spline surfaces can't be added to sketch grid
    /////////////////////////////////////////////////////////////
    // Check grid spline created from extrusion with plane as base
    ICurvePrimitivePtr linePrimitive = ICurvePrimitive::CreateLine(DPoint3d::FromZero(), DPoint3d::From(0.0,10.0,10.0));
    SketchSplineGridSurface::CreateParams nonZPlanelineParams(*grid->GetSurfacesModel(), *axis1, 0.0, 10.0, *linePrimitive);
    GridSplineSurfacePtr invalidGridSpline_Plane = SketchSplineGridSurface::Create(nonZPlanelineParams);

    ASSERT_TRUE(invalidGridSpline_Plane->Insert().IsNull()) << "Invalid grid spline surface has been created";

    // Check sketch grid arc surface created from invalid arc (0 radius)
    SketchArcGridSurface::CreateParams arcParams(*grid->GetSurfacesModel(), *axis1, 0, 10, DEllipse3d::FromCenterRadiusXY(DPoint3d::FromZero(), 0.0));
    GridArcSurfacePtr invalidGridSpline_Arc = SketchArcGridSurface::Create(arcParams);
    ASSERT_TRUE(invalidGridSpline_Arc->Insert().IsNull()) << "Invalid grid spline surface has been created";

    /////////////////////////////////////////////////////////////
    // Check if valid geometry can be set to grid surfaces
    /////////////////////////////////////////////////////////////
    // Check valid sketch grid line surface

    SketchLineGridSurface::CreateParams validLineParams(*grid->GetSurfacesModel(), *axis1, 0.0, 10.0, DPoint2d::FromZero(), DPoint2d::From(10.0, 10.0));
    GridPlanarSurfacePtr validPlaneSurface = SketchLineGridSurface::Create(validLineParams);

    validPlaneSurface->Insert();
    ASSERT_TRUE(validPlaneSurface->GetElementId().IsValid()) << "failed to insert a valid-planar gridplanesurface";
    
    CurveVectorPtr validGridPlaneVector = CurveVector::CreateLinear({ { 0, 0, 0 },{ 10, 0, 0 },{ 10, 0, 10 },{ 0, 0, 10 },{ 0, 0, 0 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    validPlaneSurface->SetCurveVector(*validGridPlaneVector.get());

    DgnDbStatus status;
    validPlaneSurface->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status) << "Failed to update plane with valid curve vector";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();
    {
    // Check valid plane extrusion detail
    DgnExtrusionDetail validGridPlaneExtDetail = GeometryUtils::CreatePlaneExtrusionDetail({ 0, 10, 0 }, { 10, 10, 0 }, 50);

    Dgn::GeometrySourceP geomElem = validPlaneSurface->ToGeometrySourceP();
    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*geomElem);

    builder->Append(*ISolidPrimitive::CreateDgnExtrusion(validGridPlaneExtDetail), Dgn::GeometryBuilder::CoordSystem::World);
    builder->Finish(*geomElem);

    validPlaneSurface->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status) << "Failed to update plane with valid dgn extrusion";
    db.SaveChanges();
    }
    // Check valid arc extrusion detail
    db.BriefcaseManager().StartBulkOperation();
    DPoint3d center = DPoint3d::From(0.0, 0.0, 0.0);
    DPoint3d start = DPoint3d::From(0.0, -100.0, 0.0);
    DPoint3d end = DPoint3d::From(0.0, 0.0, 100.0);

    SketchArcGridSurface::CreateParams validArcParams(*grid->GetSurfacesModel(), *axis1, 0.0, 100.0, DEllipse3d::FromArcCenterStartEnd(center, start, end));
    GridArcSurfacePtr validArcSurface = SketchArcGridSurface::Create(validArcParams);

    validArcSurface->Insert();
    ASSERT_TRUE(validArcSurface->GetElementId().IsValid()) << "failed to insert a valid gridarcsurface";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();
    {
    DgnExtrusionDetail validGridArcExtDetail = GeometryUtils::CreateArcExtrusionDetail(20, msGeomConst_pi / 2, 50);

    Dgn::GeometrySourceP geomElem = validArcSurface->ToGeometrySourceP();
    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*geomElem);

    builder->Append(*ISolidPrimitive::CreateDgnExtrusion(validGridArcExtDetail), Dgn::GeometryBuilder::CoordSystem::World);
    builder->Finish(*geomElem);

    validArcSurface->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status) << "Failed to update arc with valid dgn extrusion";
    db.SaveChanges();
    }
    db.BriefcaseManager().StartBulkOperation();
    // Check valid spline extrusion detail
    ICurvePrimitivePtr validSplinePrimitive = GeometryUtils::CreateSplinePrimitive({ { 0, 0, 0 },{ 10, 0, 0 },{ 0, 10, 0 } } /*poles*/);
    SketchSplineGridSurface::CreateParams validSplineSurfaceParams(*grid->GetSurfacesModel(), *axis1, 0.0, 10.0, *validSplinePrimitive);
    GridSplineSurfacePtr validSplineSurface = SketchSplineGridSurface::Create(validSplineSurfaceParams);

    ASSERT_TRUE(validSplineSurface.IsValid()) << "Failed to create grid spline surface";

    validSplineSurface->Insert();
    ASSERT_TRUE(validSplineSurface->GetElementId().IsValid()) << "failed to insert a valid gridsplinesurface";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();
    {
    DgnExtrusionDetail validGridSplineExtDetail = GeometryUtils::CreateSplineExtrusionDetail({ { 0, 0, 0 },{ 10, 0, 0 },{ 10, 30, 0 },{ 0, 0, 0 } }, 50);

    Dgn::GeometrySourceP geomElem = validSplineSurface->ToGeometrySourceP();
    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*geomElem);

    builder->Append(*ISolidPrimitive::CreateDgnExtrusion(validGridSplineExtDetail), Dgn::GeometryBuilder::CoordSystem::World);
    builder->Finish(*geomElem);

    validSplineSurface->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status) << "Failed to update arc with valid dgn extrusion";
    db.SaveChanges();
    }
    db.BriefcaseManager().StartBulkOperation();
    /////////////////////////////////////////////////////////////
    // Check if invalid geometry can't be set to grid surfaces
    /////////////////////////////////////////////////////////////
    // Check grid plane update through geometry builder
    GridPlanarSurfacePtr planeSurfaceToUpdate = db.Elements().GetForEdit<GridPlanarSurface>(validPlaneSurface->GetElementId());

    Dgn::GeometrySourceP geomElem = planeSurfaceToUpdate->ToGeometrySourceP();
    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*geomElem);

    DPoint3d nonPlanarPoints[] = { { 100,100,0 },{ 200,100,0 },{ 200,200,0 },{ 100,200,100 },{ 100,100,0 } };
    CurveVectorPtr invalidVector;
    invalidVector = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    ICurvePrimitivePtr prim = ICurvePrimitive::CreateLineString(nonPlanarPoints, 4);
    invalidVector->push_back(prim);
    builder->Append(*invalidVector->Clone(), Dgn::GeometryBuilder::CoordSystem::World);
    builder->Finish(*geomElem);

    planeSurfaceToUpdate->Update(&status);
    ASSERT_TRUE(status == DgnDbStatus::Success) << "should still succeed to update.. would reset the geometry stream though";
    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGridCurvesAreCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Create Grid surfaces representing floors
    /////////////////////////////////////////////////////////////
    double heightInterval = 10;
    int gridIteration = 0;
    bvector<DPoint3d> baseShape = { {0, 0, 0}, {20, 0, 0}, {20, 20, 0}, {0, 20, 0}, {0, 0, 0} };
    bvector<CurveVectorPtr> floorPlaneCurves = bvector<CurveVectorPtr>(3); // 3 surfaces will be created
    for (CurveVectorPtr& curveShape : floorPlaneCurves)
        {
        bvector<DPoint3d> thisShape = baseShape;
        std::transform(thisShape.begin(), thisShape.end(), thisShape.begin(), [&](DPoint3d point) -> DPoint3d {point.z = heightInterval * gridIteration; return point; });
        curveShape = CurveVector::CreateLinear(thisShape, CurveVector::BOUNDARY_TYPE_Outer);
        ++gridIteration;
        }

    ElevationGridCPtr floorGrid = ElevationGrid::CreateAndInsertWithSurfaces (ElevationGrid::CreateParams (*m_model,
                                                                            db.Elements ().GetRootSubject ()->GetElementId (),
                                                                            "Floor-Grid"),
                                                                            floorPlaneCurves);
                                                                                            
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Check validity of grid plane surfaces
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(floorGrid.IsValid()) << "Failed to create floor grid";
    ASSERT_TRUE(floorGrid->GetSurfacesModel().IsValid()) << "Failed to get floor grid surfaces model";

    int numSurfaces = floorGrid->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(3, numSurfaces) << "incorrect number of gridPlaneSurfaces in floorGrid";

    ElementIterator axesIterator = floorGrid->MakeAxesIterator();
    bvector<DgnElementId> axesIds = axesIterator.BuildIdList<DgnElementId>();
    int numAxes = axesIds.size();
    ASSERT_TRUE(numAxes == 1) << "incorrect number of axes in floorGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(3 == numHorizontal) << "axis should contain 3 planes";

    bvector<GridPlanarSurfacePtr> floorGridPlanes;
    for (DgnElementId planeId : horizontalAxis->MakeIterator().BuildIdList<DgnElementId>())
        {
        GridPlanarSurfacePtr floorSurface = db.Elements().GetForEdit<GridPlanarSurface>(planeId);
        ASSERT_TRUE(floorSurface.IsValid()) << "Failed to get floor plane surface";
        floorGridPlanes.push_back(floorSurface);
        }

    /////////////////////////////////////////////////////////////
    // Create orthogonal grid
    /////////////////////////////////////////////////////////////
    OrthogonalGrid::CreateParams orthogonalParams = OrthogonalGrid::CreateParams(*m_model,
                                                                                 db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                                                 "Orthogonal Grid",
                                                                                 15, /*defaultCoordIncX*/
                                                                                 10, /*defaultCoordIncY*/
                                                                                 0.0, /*defaultStaExtX*/
                                                                                 50.0, /*defaultEndExtX*/
                                                                                 0.0, /*defaultStaExtY*/
                                                                                 50.0, /*defaultEndExtY*/
                                                                                 - 2*BUILDING_TOLERANCE, /*defaultStaElevation*/
                                                                                 30.0 + 2*BUILDING_TOLERANCE /*defaultEndElevation*/
                                                                                 );

    OrthogonalGridPtr orthogonalGrid = OrthogonalGrid::CreateAndInsertWithSurfaces(orthogonalParams, 2, 1);
    ASSERT_TRUE(orthogonalGrid.IsValid()) << "Failed to create orthogonal grid";

    GridCurvesSetPtr curvesPortion = GridCurvesSet::Create(*m_model);

    curvesPortion->Insert();

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Check intersection curves with orthogonal grid
    /////////////////////////////////////////////////////////////
    // Check intersection success
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        BentleyStatus status = orthogonalGrid->IntersectGridSurface(floorGridSurface.get(), *curvesPortion);
        ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";
        }

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    // Check if grid curves are all created and have valid geometry
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        ElementIdIterator floorGridCurveBundlesIterator = floorGridSurface->MakeGridCurveBundleIterator();
        ASSERT_EQ(3, floorGridCurveBundlesIterator.BuildIdList<DgnElementId>().size());
        
        double elevation = floorGridSurface->GetPlane().origin.z;
        bvector<ICurvePrimitiveCPtr> expectedGeometries =   //forwards or backwards doesn't matter
            {
            ICurvePrimitive::CreateLineString({ { 20, 0, elevation },{ 0, 0, elevation } }),
            ICurvePrimitive::CreateLineString({ { 0, 15, elevation },{ 20, 15, elevation } }),
            ICurvePrimitive::CreateLineString({ { 0, 20, elevation },{ 0, 0, elevation } }),
            ICurvePrimitive::CreateLineString({ { 0, 0, elevation },{ 20, 0, elevation } }),
            ICurvePrimitive::CreateLineString({ { 20, 15, elevation },{ 0, 15, elevation } }),
            ICurvePrimitive::CreateLineString({ { 0, 0, elevation },{ 0, 20, elevation } })
            };

        bset<Dgn::DgnElementId> intersectingSurfaces;
        for (DgnElementId bundleId : floorGridCurveBundlesIterator.BuildIdList<DgnElementId>())
            {
            GridCurveBundleCPtr curveBundle = GridCurveBundle::Get(db, bundleId);
            ASSERT_TRUE(curveBundle.IsValid());

            GridCurveCPtr curve = curveBundle->GetGridCurve();
            ASSERT_TRUE(curve.IsValid()) << "Failed to get grid curve";

            bvector<Dgn::DgnElementId> surfaceIds = curve->GetIntersectingSurfaceIds();
            ASSERT_EQ(surfaceIds.size(), 2);
            intersectingSurfaces.insert(surfaceIds.begin(), surfaceIds.end());

            ASSERT_NE(expectedGeometries.end(), std::find_if(expectedGeometries.begin(),
                                                             expectedGeometries.end(),
                                                             [&](ICurvePrimitiveCPtr expectedCurve) {return expectedCurve->IsSameStructureAndGeometry(*curve->GetCurve(), 0.1); }))
                << "Grid curve geometry is not as expected";
            }

        // Make sure intersecting surfaces are correct
        bvector<DgnElementId> expectedIds = orthogonalGrid->MakeIterator().BuildIdList<DgnElementId>();
        expectedIds.push_back(floorGridSurface->GetElementId());
        ASSERT_EQ(expectedIds.size(), intersectingSurfaces.size());
        for (Dgn::DgnElementId intersecting : intersectingSurfaces)
            {
            ASSERT_NE(expectedIds.end(), std::find(expectedIds.begin(), expectedIds.end(), intersecting)) << "Intersecting surface id is incorrect";
            }
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGridCurvesAreCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Create Grid surfaces representing floors
    /////////////////////////////////////////////////////////////
    double heightInterval = 10;
    int gridIteration = 0;
    bvector<DPoint3d> baseShape = { {0, 0, 0}, {20, 0, 0}, {20, 20, 0}, {0, 20, 0}, {0, 0, 0} };
    bvector<CurveVectorPtr> floorPlaneCurves = bvector<CurveVectorPtr>(3); // 3 surfaces will be created
    for (CurveVectorPtr& curveShape : floorPlaneCurves)
        {
        bvector<DPoint3d> thisShape = baseShape;
        std::transform(thisShape.begin(), thisShape.end(), thisShape.begin(), [&](DPoint3d point) -> DPoint3d {point.z = heightInterval * gridIteration; return point; });
        curveShape = CurveVector::CreateLinear(thisShape, CurveVector::BOUNDARY_TYPE_Outer);
        ++gridIteration;
        }

    ElevationGridCPtr floorGrid = ElevationGrid::CreateAndInsertWithSurfaces (ElevationGrid::CreateParams (*m_model,
                                                                              db.Elements ().GetRootSubject ()->GetElementId (),
                                                                              "Floor-Grid"),
                                                                              floorPlaneCurves);
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Check validity of grid plane surfaces
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(floorGrid.IsValid()) << "Failed to create floor grid";
    ASSERT_TRUE(floorGrid->GetSurfacesModel().IsValid()) << "Failed to get floor grid surfaces model";

    int numSurfaces = floorGrid->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(3, numSurfaces) << "incorrect number of gridPlaneSurfaces in floorGrid";

    ElementIterator axesIterator = floorGrid->MakeAxesIterator();
    bvector<DgnElementId> axesIds = axesIterator.BuildIdList<DgnElementId>();
    int numAxes = axesIds.size();
    ASSERT_TRUE(numAxes == 1) << "incorrect number of axes in floorGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(3 == numHorizontal) << "axis should contain 3 planes";

    bvector<GridPlanarSurfacePtr> floorGridPlanes;
    for (DgnElementId planeId : horizontalAxis->MakeIterator().BuildIdList<DgnElementId>())
        {
        GridPlanarSurfacePtr floorSurface = db.Elements().GetForEdit<GridPlanarSurface>(planeId);
        ASSERT_TRUE(floorSurface.IsValid()) << "Failed to get floor plane surface";
        floorGridPlanes.push_back(floorSurface);
        }

    /////////////////////////////////////////////////////////////
    // Create radial grid
    /////////////////////////////////////////////////////////////
    RadialGrid::CreateParams radialParams = RadialGrid::CreateParams(*m_model,
                                                                     db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                                     "Radial Grid",
                                                                     msGeomConst_pi / 6, /*defaultAngleIncrement*/
                                                                     10, /*defaultRadiusIncrement*/
                                                                     0.0, /*defaultStartAngle*/
                                                                     msGeomConst_pi / 6 * 2, /*defaultEndAngle*/
                                                                     0.0, /*defaultStartRadius*/
                                                                     50, /*defaultEndRadius*/
                                                                     -BUILDING_TOLERANCE, /*defaultstaElevation*/
                                                                     30+BUILDING_TOLERANCE /*defaultendElevation*/
                                                                     );

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsertWithSurfaces(radialParams, 2, 2);
    ASSERT_TRUE(radialGrid.IsValid()) << "Failed to create radial grid";

    GridCurvesSetPtr curvesPortion = GridCurvesSet::Create(*m_model);
    curvesPortion->Insert();
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Check intersection curves with radial grid
    /////////////////////////////////////////////////////////////
    // Check intersection success
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        BentleyStatus status = radialGrid->IntersectGridSurface(floorGridSurface.get(), *curvesPortion);
        ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";
        }

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    // Check if grid curves are all created and have valid geometry
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        ElementIdIterator floorGridCurveBundlesIterator = floorGridSurface->MakeGridCurveBundleIterator();
        ASSERT_EQ(4, floorGridCurveBundlesIterator.BuildIdList<DgnElementId>().size()); // TODO correct to 4 after arced grid curves can be created
        
        double elevation = floorGridSurface->GetPlane().origin.z;
        bvector<ICurvePrimitiveCPtr> expectedGeometries =
            {
            ICurvePrimitive::CreateLineString({{20, 0, elevation },
                                               {0, 0, elevation}}),
            ICurvePrimitive::CreateLineString({{20, 20 * std::tan(msGeomConst_pi / 6), elevation},
                                               {0.0, 0.0, elevation}}),
            ICurvePrimitive::CreateArc(DEllipse3d::FromArcCenterStartEnd({0, 0, elevation },
                                                                         {10 * std::cos(0), 10 * std::sin(0), elevation },
                                                                         {10 * std::cos(msGeomConst_pi / 6 * 2), 10 * std::sin(msGeomConst_pi / 6 * 2), elevation })),
            ICurvePrimitive::CreateArc(DEllipse3d::FromArcCenterStartEnd({0, 0, elevation },
                                                                         {20 * std::cos(0), 20 * std::sin(0), elevation },
                                                                         {20 * std::cos(msGeomConst_pi / 6 * 2), 20 * std::sin(msGeomConst_pi / 6 * 2), elevation }))
            };

        bset<Dgn::DgnElementId> intersectingSurfaces;
        for (DgnElementId bundleId : floorGridCurveBundlesIterator.BuildIdList<DgnElementId>())
            {
            GridCurveBundleCPtr curveBundle = GridCurveBundle::Get(db, bundleId);
            ASSERT_TRUE(curveBundle.IsValid());

            GridCurveCPtr curve = curveBundle->GetGridCurve();
            ASSERT_TRUE(curve.IsValid()) << "Failed to get grid curve";

            bvector<Dgn::DgnElementId> surfaceIds = curve->GetIntersectingSurfaceIds();
            ASSERT_EQ(surfaceIds.size(), 2);
            intersectingSurfaces.insert(surfaceIds.begin(), surfaceIds.end());

            ASSERT_NE(expectedGeometries.end(), std::find_if(expectedGeometries.begin(),
                                                             expectedGeometries.end(),
                                                             [&] (ICurvePrimitiveCPtr expectedCurve) { return expectedCurve->IsSameStructureAndGeometry(*curve->GetCurve(), 0.1); }))
                << "Grid curve geometry is not as expected";
            }     

        // Make sure intersecting surfaces are correct
        bvector<DgnElementId> expectedIds = radialGrid->MakeIterator().BuildIdList<DgnElementId>();
        expectedIds.push_back(floorGridSurface->GetElementId());
        ASSERT_EQ(expectedIds.size(), intersectingSurfaces.size());
        for (Dgn::DgnElementId intersecting : intersectingSurfaces)
            {
            ASSERT_NE(expectedIds.end(), std::find(expectedIds.begin(), expectedIds.end(), intersecting)) << "Intersecting surface id is incorrect";
            }
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, SketchGridCurvesAreCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Create Grid surfaces representing floors
    /////////////////////////////////////////////////////////////
    double heightInterval = 10;
    int gridIteration = 0;
    bvector<DPoint3d> baseShape = { {0, 0, 0}, {10, 0, 0}, {10, 10, 0}, {0, 10, 0}, {0, 0, 0} };
    bvector<CurveVectorPtr> floorPlaneCurves = bvector<CurveVectorPtr>(3); // 3 surfaces will be created
    for (CurveVectorPtr& curveShape : floorPlaneCurves)
        {
        bvector<DPoint3d> thisShape = baseShape;
        std::transform(thisShape.begin(), thisShape.end(), thisShape.begin(), [&](DPoint3d point) -> DPoint3d {point.z = heightInterval * gridIteration; return point; });
        curveShape = CurveVector::CreateLinear(thisShape, CurveVector::BOUNDARY_TYPE_Outer);
        ++gridIteration;
        }

    ElevationGridCPtr floorGrid = ElevationGrid::CreateAndInsertWithSurfaces (ElevationGrid::CreateParams (*m_model,
                                                                              db.Elements ().GetRootSubject ()->GetElementId (),
                                                                              "Floor-Grid"),
                                                                              floorPlaneCurves);
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Check validity of grid plane surfaces
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(floorGrid.IsValid()) << "Failed to create floor grid";
    ASSERT_TRUE(floorGrid->GetSurfacesModel().IsValid()) << "Failed to get floor grid surfaces model";

    int numSurfaces = floorGrid->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_EQ(3, numSurfaces) << "incorrect number of gridPlaneSurfaces in floorGrid";

    ElementIterator axesIterator = floorGrid->MakeAxesIterator();
    bvector<DgnElementId> axesIds = axesIterator.BuildIdList<DgnElementId>();
    int numAxes = axesIds.size();
    ASSERT_TRUE(numAxes == 1) << "incorrect number of axes in floorGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(3 == numHorizontal) << "axis should contain 3 planes";

    bvector<GridPlanarSurfacePtr> floorGridPlanes;
    for (DgnElementId planeId : horizontalAxis->MakeIterator().BuildIdList<DgnElementId>())
        {
        GridPlanarSurfacePtr floorSurface = db.Elements().GetForEdit<GridPlanarSurface>(planeId);
        ASSERT_TRUE(floorSurface.IsValid()) << "Failed to get floor plane surface";
        floorGridPlanes.push_back(floorSurface);
        }

    /////////////////////////////////////////////////////////////
    // Create sketch grid
    /////////////////////////////////////////////////////////////
    SketchGridPtr sketchGrid = SketchGrid::Create(*m_model, m_model->GetModeledElementId(), "Sketch Grid", 0.0, 10.0);
    ASSERT_TRUE(sketchGrid.IsValid()) << "Failed to create sketch grid";

    ASSERT_TRUE(sketchGrid->Insert().IsValid()) << "Failed to insert sketch grid";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    Grids::GridAxisPtr gridAxis = GeneralGridAxis::CreateAndInsert(*sketchGrid);

    ASSERT_TRUE(gridAxis.IsValid()) << "Failed to create sketch grid axis";
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    DgnExtrusionDetail planeExtDetail = GeometryUtils::CreatePlaneExtrusionDetail({ 0, 0, -BUILDING_TOLERANCE }, { 25, 25, -BUILDING_TOLERANCE }, 30 + 2*BUILDING_TOLERANCE);
    SketchLineGridSurface::CreateParams lineSurfParams(*sketchGrid->GetSurfacesModel(), *gridAxis, -BUILDING_TOLERANCE, 30 + BUILDING_TOLERANCE, { 0, 0 }, { 25, 25 });
    GridPlanarSurfacePtr plane = SketchLineGridSurface::Create(lineSurfParams);
    ASSERT_TRUE(plane.IsValid()) << "Failed to create grid plane surface";

    plane->Insert();

    SketchArcGridSurface::CreateParams arcSurfParams(*sketchGrid->GetSurfacesModel(), *gridAxis, -BUILDING_TOLERANCE, 30 + BUILDING_TOLERANCE, GeometryUtils::CreateArc(10 /*radius*/, 30 * msGeomConst_pi / 180 /*base angle*/, 0 /*extend length*/));
    GridArcSurfacePtr arc = SketchArcGridSurface::Create(arcSurfParams);
    ASSERT_TRUE(arc.IsValid()) << "Failed to create grid plane surface";

    arc->Insert();

    ICurvePrimitivePtr splinePrimitive = GeometryUtils::CreateSplinePrimitive({ { 0, 0, 0 },{ 2, 3, 0 },{8, 5, 0 } } /*poles*/);
    SketchSplineGridSurface::CreateParams splineSurfParams(*sketchGrid->GetSurfacesModel(), *gridAxis, -BUILDING_TOLERANCE, 30 + BUILDING_TOLERANCE, *splinePrimitive);
    GridSplineSurfacePtr spline = SketchSplineGridSurface::Create(splineSurfParams);
    ASSERT_TRUE(spline.IsValid()) << "Failed to create grid spline surface";

    spline->Insert();

    GridCurvesSetPtr curvesPortion = GridCurvesSet::Create(*m_model);

    curvesPortion->Insert();

    /////////////////////////////////////////////////////////////
    // Check intersection curves with sketch grid
    /////////////////////////////////////////////////////////////
    // Check intersection success
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        BentleyStatus status = sketchGrid->IntersectGridSurface(floorGridSurface.get(), *curvesPortion);
        ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";
        }

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    // Check if grid curves are all created and have valid geometry
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        ElementIdIterator floorGridCurveBundlesIterator = floorGridSurface->MakeGridCurveBundleIterator();
        ASSERT_EQ(3, floorGridCurveBundlesIterator.BuildIdList<DgnElementId>().size());
      
        double elevation = floorGridSurface->GetPlane().origin.z;

        bvector<double> splineWeights = { 1.0, 1.0, 1.0 };
        bvector<double> splineKnots = { 0, 1, 2, 3, 4, 5 };

        bvector<ICurvePrimitiveCPtr> expectedGeometries =
            {
            ICurvePrimitive::CreateLineString({{10, 10, elevation }, 
                                               {0, 0, elevation}}),
            ICurvePrimitive::CreateArc(DEllipse3d::FromArcCenterStartEnd({0, 0, elevation }, 
                                                                         {10 * std::cos(0), 10 * std::sin(0), elevation },
                                                                         {10 * std::cos(30.0 * msGeomConst_pi / 180.0), 10 * std::sin(30.0 * msGeomConst_pi / 180.0), elevation })),
            ICurvePrimitive::CreateBsplineCurve(MSBsplineCurve::CreateFromPolesAndOrder({ { 1, 1.5, 0 },{ 2, 3, 0 },{ 5, 4, 0 } }, &splineWeights, &splineKnots, 3, false, false))
            };

        bset<Dgn::DgnElementId> intersectingSurfaces;
        for (DgnElementId bundleId : floorGridCurveBundlesIterator.BuildIdList<DgnElementId>())
            {
            GridCurveBundleCPtr curveBundle = GridCurveBundle::Get(db, bundleId);
            ASSERT_TRUE(curveBundle.IsValid());

            GridCurveCPtr curve = curveBundle->GetGridCurve();
            ASSERT_TRUE(curve.IsValid()) << "Failed to get grid curve";

            bvector<Dgn::DgnElementId> surfaceIds = curve->GetIntersectingSurfaceIds();
            ASSERT_EQ(surfaceIds.size(), 2);
            intersectingSurfaces.insert(surfaceIds.begin(), surfaceIds.end());

            if (nullptr != curve->GetCurve()->GetBsplineCurveCP() ||
                nullptr != curve->GetCurve()->GetInterpolationCurveCP()) //if this is a spline, ignore for now..
                continue; 

            ASSERT_NE(expectedGeometries.end(), std::find_if(expectedGeometries.begin(),
                                                             expectedGeometries.end(),
                                                             [&](ICurvePrimitiveCPtr expectedCurve) {return expectedCurve->IsSameStructureAndGeometry(*curve->GetCurve(), 0.1); }))
                << "Grid curve geometry is not as expected";
            }

        // Make sure intersecting surfaces are correct
        bvector<DgnElementId> expectedIds = sketchGrid->MakeIterator().BuildIdList<DgnElementId>();
        expectedIds.push_back(floorGridSurface->GetElementId());
        ASSERT_EQ(expectedIds.size(), intersectingSurfaces.size());
        for (Dgn::DgnElementId intersecting : intersectingSurfaces)
            {
            ASSERT_NE(expectedIds.end(), std::find(expectedIds.begin(), expectedIds.end(), intersecting)) << "Intersecting surface id is incorrect";
            }
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, GridArc_Created)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Create Grid arc with curve primitive containing arc
    /////////////////////////////////////////////////////////////
    ICurvePrimitivePtr arcCurve = GeometryUtils::CreateArc({ 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
    ASSERT_TRUE(arcCurve.IsValid()) << "Failed to create arc curve";

    // Make sure arc curve's geometry is correct
    DEllipse3d arcEllipse;
    ASSERT_TRUE(arcCurve->TryGetArc(arcEllipse)) << "Created arcCurve is not an ellipse";
    ASSERT_TRUE(arcEllipse.center.AlmostEqual({ 0, 0, 0 })) << "Created arcCurve's center point is incorrect" ;
    
    DPoint3d arcStart, arcEnd;
    arcEllipse.EvaluateEndPoints(arcStart, arcEnd);
    ASSERT_TRUE(arcStart.AlmostEqual({ 5, 0, 0 })) << "Created arcCurve's start point is incorrect";
    ASSERT_TRUE(arcEnd.AlmostEqual({ 0, 5, 0 })) << "Created arcCurve's end point is incorrect";
    
    double arcAngle = DVec3d::FromStartEnd(DPoint3d::From( 0, 0, 0 ), DPoint3d::From(5, 0, 0 )).AngleToXY(DVec3d::FromStartEnd(DPoint3d::From(0, 0, 0 ), DPoint3d::From(0, 5, 0 )));
    ASSERT_EQ(arcEllipse.ArcLength(), arcAngle * 5) << "Created arcCurve's length is incorrect";

    GridCurvesSetPtr curvesPortion = GridCurvesSet::Create(*m_model);

    curvesPortion->Insert();

    // Try creating the grid arc
    GridArcPtr arc = GridArc::Create(*curvesPortion, arcCurve);
    ASSERT_TRUE(arc.IsValid()) << "Error when creating grid arc";


    ASSERT_TRUE(arc->Insert().IsValid()) << "Error when inserting grid arc";
    ICurvePrimitivePtr actualGeometry = arc->GetCurve();

    ASSERT_TRUE(actualGeometry.IsValid()) << "Failed to get grid arc's geometry";
    ASSERT_TRUE(actualGeometry->IsSameStructureAndGeometry(*arcCurve.get(), BUILDING_TOLERANCE));

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Create Grid arc with curve primitive containing not arc
    /////////////////////////////////////////////////////////////
    ICurvePrimitivePtr lineCurve = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 5, 0, 0 });
    ASSERT_TRUE(lineCurve.IsValid()) << "Failed to create line curve";

    GridArcPtr lineArc = GridArc::Create(*curvesPortion, lineCurve);
    ASSERT_TRUE(lineArc.IsValid()) << "Error when creating grid arc";


    ASSERT_TRUE(lineArc->Insert().IsNull()) << "Should not be able to insert GridArc with invalid geometry";
    
    /////////////////////////////////////////////////////////////
    // Try setting a valid GridArc with invalid geometry
    /////////////////////////////////////////////////////////////
    arc->SetCurve(lineCurve);

    ASSERT_TRUE(arc->Update().IsNull()) << "Should not be able to update GridArc with invalid geometry";

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, GridLine_Created)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Create Grid arc with curve primitive containing line
    /////////////////////////////////////////////////////////////
    ICurvePrimitivePtr lineCurve = ICurvePrimitive::CreateLineString({ { 0, 0, 0 }, { 5, 0, 0 } });
    ASSERT_TRUE(lineCurve.IsValid()) << "Failed to create line curve";

    // Make sure line curve's geometry is correct
    bvector<DPoint3d> const * line = lineCurve->GetLineStringCP();
    ASSERT_TRUE(nullptr != line) << "Created lineCurve is not a line";
    ASSERT_EQ(2, line->size());
    ASSERT_NE(line->end(), std::find_if(line->begin(), line->end(), [&](DPoint3d linePoint) 
        {
        return linePoint.AlmostEqual({ 0, 0, 0 });
        })) << "Created line curve geometry is incorrect";
    ASSERT_NE(line->end(), std::find_if(line->begin(), line->end(), [&](DPoint3d linePoint)
        {
        return linePoint.AlmostEqual({ 5, 0, 0 });
        })) << "Created line curve geometry is incorrect";

    GridCurvesSetPtr curvesPortion = GridCurvesSet::Create(*m_model);

    curvesPortion->Insert();
    // Try creating the grid line
    GridLinePtr gridLine = GridLine::Create(*curvesPortion, lineCurve);
    ASSERT_TRUE(gridLine.IsValid()) << "Error when creating grid line";

    ASSERT_TRUE(gridLine->Insert().IsValid()) << "Error when inserting grid line";
    ICurvePrimitivePtr actualGeometry = gridLine->GetCurve();

    ASSERT_TRUE(actualGeometry.IsValid()) << "Failed to get grid line's geometry";
    ASSERT_TRUE(actualGeometry->IsSameStructureAndGeometry(*lineCurve.get(), BUILDING_TOLERANCE));

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Create Grid line with curve primitive containing not line
    /////////////////////////////////////////////////////////////
    ICurvePrimitivePtr arcCurve = GeometryUtils::CreateArc({ 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
    ASSERT_TRUE(arcCurve.IsValid()) << "Failed to create line curve";

    GridLinePtr arcLine = GridLine::Create(*curvesPortion, arcCurve);
    ASSERT_TRUE(arcLine.IsValid()) << "Error when creating grid line";


    ASSERT_TRUE(arcLine->Insert().IsNull()) << "Should not be able to insert GridLine with invalid geometry";

    /////////////////////////////////////////////////////////////
    // Try setting a valid GridLine with invalid geometry
    /////////////////////////////////////////////////////////////
    gridLine->SetCurve(arcCurve);

    ASSERT_TRUE(gridLine->Update().IsNull()) << "Should not be able to update GridLine with invalid geometry";

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, GridSpline_Created)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    GridCurvesSetPtr curvesPortion = GridCurvesSet::Create(*m_model);

    curvesPortion->Insert();
    /////////////////////////////////////////////////////////////
    // Create Grid spline with curve primitive containing spline
    /////////////////////////////////////////////////////////////
    bvector<double> splineWeights = { 1.0, 1.0, 1.0 };
    bvector<double> splineKnots = { 0, 1, 2, 3, 4, 5 };
    ICurvePrimitivePtr splineCurve = ICurvePrimitive::CreateBsplineCurve(MSBsplineCurve::CreateFromPolesAndOrder({ { 0, 0, 0 },{ 10, 0, 0 },{ 0, 10, 0 } }, &splineWeights, &splineKnots, 3, false, false));
    ASSERT_TRUE(splineCurve.IsValid()) << "Failed to create spline curve";

    // Try creating the grid line
    GridSplinePtr gridSpline = GridSpline::Create(*curvesPortion, splineCurve);
    ASSERT_TRUE(gridSpline.IsValid()) << "Error when creating grid spline";


    ASSERT_TRUE(gridSpline->Insert().IsValid()) << "Error when inserting grid spline";
    ICurvePrimitivePtr actualGeometry = gridSpline->GetCurve();

    ASSERT_TRUE(actualGeometry.IsValid()) << "Failed to get grid line's geometry";
    ASSERT_TRUE(actualGeometry->IsSameStructureAndGeometry(*splineCurve.get(), BUILDING_TOLERANCE));

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Create Grid spline with curve primitive containing not spline
    /////////////////////////////////////////////////////////////
    ICurvePrimitivePtr arcCurve = GeometryUtils::CreateArc({ 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
    ASSERT_TRUE(arcCurve.IsValid()) << "Failed to create spline curve";

    GridSplinePtr arcSpline = GridSpline::Create(*curvesPortion, arcCurve);
    ASSERT_TRUE(arcSpline.IsValid()) << "Error when creating grid spline";


    ASSERT_TRUE(arcSpline->Insert().IsNull()) << "Should not be able to insert GridSpline with invalid geometry";

    /////////////////////////////////////////////////////////////
    // Try setting a valid GridSpline with invalid geometry
    /////////////////////////////////////////////////////////////
    gridSpline->SetCurve(arcCurve);

    ASSERT_TRUE(gridSpline->Update().IsNull()) << "Should not be able to update GridSpline with invalid geometry";

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, GridAxis_GridHasNoSubmodel_CreateReturnsNullptr)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    SketchGridPtr grid = SketchGrid::Create(*m_model, m_model->GetModeledElementId(), "Grid", 0.0, 10.0);

    ASSERT_TRUE(grid->Insert().IsValid()) << "Failed to insert grid";

    Grids::GridAxisPtr gridAxis = GeneralGridAxis::Create(*grid);

    ASSERT_TRUE(gridAxis.IsNull()) << "Failed to create grid axis";

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, GridAxis_Created)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    SketchGridPtr grid = SketchGrid::Create(*m_model, m_model->GetModeledElementId(), "Grid", 0.0, 10.0);

    ASSERT_TRUE(grid->Insert().IsValid()) << "Failed to insert grid";
    grid->GetSurfacesModel();

    /////////////////////////////////////////////////////////////
    // Use Create method to create grid axis
    /////////////////////////////////////////////////////////////

    Grids::GridAxisPtr gridAxis0 = GeneralGridAxis::Create(*grid);

    ASSERT_TRUE(gridAxis0.IsValid()) << "Failed to create grid axis";
    ASSERT_FALSE(gridAxis0->GetElementId().IsValid()) << "Grid axis should not have been inserted yet";

    ASSERT_TRUE(gridAxis0->Insert().IsValid()) << "Failed to insert created grid axis";
    ASSERT_TRUE(gridAxis0->GetElementId().IsValid()) << "Inserted grid axis' id should be valid";

    ASSERT_EQ(grid->GetElementId(), gridAxis0->GetGridId()) << "Grid axis' grid id is incorrect";

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Use CreateAndInsert method to create grid axis
    /////////////////////////////////////////////////////////////
    Grids::GridAxisPtr gridAxis1 = GeneralGridAxis::CreateAndInsert(*grid);
    ASSERT_TRUE(gridAxis1.IsValid()) << "Failed to create and insert grid axis";
    ASSERT_TRUE(gridAxis1->GetElementId().IsValid()) << "Inserted grid axis' id should be valid";

    ASSERT_EQ(grid->GetElementId(), gridAxis1->GetGridId()) << "Grid axis' grid id is incorrect";

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Check grid axis' elements
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(0, gridAxis1->MakeIterator().BuildIdList<DgnElementId>().size()) << "Grid axis should contain no elements";
    
    SketchLineGridSurface::CreateParams lineSurfParams(*grid->GetSurfacesModel(), *gridAxis1, 0.0, 90, { 50, 20 }, { 50, 70 });
    GridPlanarSurfacePtr plane = SketchLineGridSurface::Create(lineSurfParams);

    ASSERT_TRUE(plane.IsValid()) << "Failed to create grid plane surface";

    plane->Insert();
    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    ASSERT_EQ(1, gridAxis1->MakeIterator().BuildIdList<DgnElementId>().size()) << "Grid axis should contain the inserted grid plane";

    plane->Delete();
    db.SaveChanges();

    ASSERT_EQ(0, gridAxis1->MakeIterator().BuildIdList<DgnElementId>().size()) << "Grid axis should no longer contain the grid plane";
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GridsTestFixture, SetName)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    SketchGridPtr grid = SketchGrid::Create(*m_model, m_model->GetModeledElementId(), "Grid", 0.0, 10.0);

    ASSERT_TRUE(grid->Insert().IsValid()) << "Failed to insert grid";

    ASSERT_EQ(SUCCESS, db.SaveChanges());
    db.BriefcaseManager().StartBulkOperation();

    ASSERT_STREQ(grid->GetName(), "Grid");

    grid->SetName("NewName");

    ASSERT_TRUE(grid->Update().IsValid());

    ASSERT_EQ(SUCCESS, db.SaveChanges());

    ASSERT_STREQ(grid->GetName(), "NewName");
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GridsTestFixture, Orthogonal_CreateAndInsertWithSurfaces_NotUniqueNameReturnsNullInsteadOfCrashing)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    Utf8String name = "TestName";

    OrthogonalGrid::CreateParams params1(*m_model, m_model->GetModeledElementId(), name.c_str(), 0, 0, 0, 0, 0, 0, 0, 0);
    ASSERT_TRUE(OrthogonalGrid::CreateAndInsertWithSurfaces(params1, 0, 0).IsValid());

    OrthogonalGrid::CreateParams params2(*m_model, m_model->GetModeledElementId(), name.c_str(), 0, 0, 0, 0, 0, 0, 0, 0);
    ASSERT_FALSE(OrthogonalGrid::CreateAndInsertWithSurfaces(params1, 0, 0).IsValid());

    ASSERT_EQ(SUCCESS, db.SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, GridSurfacesTests)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    SketchGridPtr grid = SketchGrid::Create(*m_model, m_model->GetModeledElementId(), "Grid", 0.0, 10.0);

    ASSERT_TRUE(grid->Insert().IsValid()) << "Failed to insert grid";

    Grids::GeneralGridAxisPtr gridAxis = GeneralGridAxis::CreateAndInsert(*grid);
    ASSERT_TRUE(gridAxis.IsValid()) << "Failed to create and insert grid axis";

    SketchLineGridSurface::CreateParams lineSurfParams(*grid->GetSurfacesModel(), *gridAxis, 0.0, 20, { 0, 0 }, { 10, 0 });
    SketchLineGridSurfacePtr surface = SketchLineGridSurface::Create(lineSurfParams);

    ASSERT_TRUE(surface.IsValid()) << "Failed to create grid plane surface";

    ASSERT_TRUE(surface->Insert().IsValid()) << "Failed to insert grid plane surface";

    db.SaveChanges();
    db.BriefcaseManager().StartBulkOperation();

    /////////////////////////////////////////////////////////////
    // Check created grid surface
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(grid->GetElementId(), surface->GetGridId()) << "Surface's grid id is incorrect";
    ASSERT_EQ(gridAxis->GetElementId(), surface->GetAxisId()) << "Surface's axis id is incorrect";
    
    double gridHeight;
    ASSERT_EQ(BentleyStatus::SUCCESS, surface->TryGetHeight(gridHeight)) << "Failed to get surface's height";
    ASSERT_EQ(20, gridHeight) << "Surface's height is incorrect";

    double gridLength;
    ASSERT_EQ(BentleyStatus::SUCCESS, surface->TryGetLength(gridLength)) << "Failed to get surface's length";
    ASSERT_EQ(10, gridLength) << "Surface's length is incorrect";

    CurveVectorPtr expectedBase = CurveVector::CreateLinear({ {0, 0, 0}, {10, 0, 0} }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
    ASSERT_TRUE(expectedBase.IsValid()) << "Failed to create imitating surface's base curve vector";

    CurveVectorPtr actualBase = surface->GetSurfaceVector();
    ASSERT_TRUE(actualBase.IsValid()) << "Failed to get surface's curve vector";
    ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase.get(), BUILDING_TOLERANCE)) << "Grid's base curve vector is incorrect";
    
    /////////////////////////////////////////////////////////////
    // Try modifying grid surface's geometry
    /////////////////////////////////////////////////////////////
    { // Try rotating around origin
            {
            surface->RotateXY(msGeomConst_pi / 4);

            ASSERT_TRUE(surface->Update().IsValid()) << "Failed to update modified surface";

            CurveVectorPtr expectedBase = CurveVector::CreateLinear({ { 0, 0, 0 },{ 7.07, 7.07, 0 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
            ASSERT_TRUE(expectedBase.IsValid()) << "Failed to create imitating surface's base curve vector";

            CurveVectorPtr actualBase = surface->GetSurfaceVector();
            ASSERT_TRUE(actualBase.IsValid()) << "Failed to get surface's curve vector";
            ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase.get(), 0.1)) << "Grid's base curve vector is incorrect";
            }

            // Return to previous state
            {
            surface->RotateXY(-msGeomConst_pi / 4);

            ASSERT_TRUE(surface->Update().IsValid()) << "Failed to update modified surface";

            CurveVectorPtr expectedBase = CurveVector::CreateLinear({ { 0, 0, 0 },{ 10, 0, 0 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
            ASSERT_TRUE(expectedBase.IsValid()) << "Failed to create imitating surface's base curve vector";

            CurveVectorPtr actualBase = surface->GetSurfaceVector();
            ASSERT_TRUE(actualBase.IsValid()) << "Failed to get surface's curve vector";
            ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase.get(), 0.1)) << "Grid's base curve vector is incorrect";
            }
    }

    { // Try rotating around given point
            {
            surface->RotateXY({ 10, 0, 0 }, msGeomConst_pi / 4);

            ASSERT_TRUE(surface->Update().IsValid()) << "Failed to update modified surface";

            CurveVectorPtr expectedBase = CurveVector::CreateLinear({ { 2.93, -7.07, 0 },{ 10, 0, 0 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
            ASSERT_TRUE(expectedBase.IsValid()) << "Failed to create imitating surface's base curve vector";

            CurveVectorPtr actualBase = surface->GetSurfaceVector();
            ASSERT_TRUE(actualBase.IsValid()) << "Failed to get surface's curve vector";
            ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase.get(), 0.1)) << "Grid's base curve vector is incorrect";
            }

            // Return to previous state
            {
            surface->RotateXY({ 10, 0, 0 }, -msGeomConst_pi / 4);

            ASSERT_TRUE(surface->Update().IsValid()) << "Failed to update modified surface";

            CurveVectorPtr expectedBase = CurveVector::CreateLinear({ { 0, 0, 0 },{ 10, 0, 0 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
            ASSERT_TRUE(expectedBase.IsValid()) << "Failed to create imitating surface's base curve vector";

            CurveVectorPtr actualBase = surface->GetSurfaceVector();
            ASSERT_TRUE(actualBase.IsValid()) << "Failed to get surface's curve vector";
            ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase.get(), 0.1)) << "Grid's base curve vector is incorrect";
            }
    }

    { // Try translate by given vector
            {
            surface->TranslateXY(DVec2d::From(10, 10));

            ASSERT_TRUE(surface->Update().IsValid()) << "Failed to update modified surface";

            CurveVectorPtr expectedBase = CurveVector::CreateLinear({ { 10, 10, 0 },{ 20, 10, 0 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
            ASSERT_TRUE(expectedBase.IsValid()) << "Failed to create imitating surface's base curve vector";

            CurveVectorPtr actualBase = surface->GetSurfaceVector();
            ASSERT_TRUE(actualBase.IsValid()) << "Failed to get surface's curve vector";
            ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase.get(), 0.1)) << "Grid's base curve vector is incorrect";
            }

            // Return to previous state
            {
            surface->TranslateXY(DVec2d::From(-10, -10));

            ASSERT_TRUE(surface->Update().IsValid()) << "Failed to update modified surface";

            CurveVectorPtr expectedBase = CurveVector::CreateLinear({ { 0, 0, 0 },{ 10, 0, 0 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
            ASSERT_TRUE(expectedBase.IsValid()) << "Failed to create imitating surface's base curve vector";

            CurveVectorPtr actualBase = surface->GetSurfaceVector();
            ASSERT_TRUE(actualBase.IsValid()) << "Failed to get surface's curve vector";
            ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase.get(), 0.1)) << "Grid's base curve vector is incorrect";
            }
    }

    { // Try translate by given vector
            {
            surface->Translate(DVec3d::From( 10, 10, 10 ));

            ASSERT_TRUE(surface->Update().IsValid()) << "Failed to update modified surface";

            CurveVectorPtr expectedBase = CurveVector::CreateLinear({ { 10, 10, 10 },{ 20, 10, 10 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
            ASSERT_TRUE(expectedBase.IsValid()) << "Failed to create imitating surface's base curve vector";

            CurveVectorPtr actualBase = surface->GetSurfaceVector();
            ASSERT_TRUE(actualBase.IsValid()) << "Failed to get surface's curve vector";
            ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase.get(), 0.1)) << "Grid's base curve vector is incorrect";
            }

            // Return to previous state
            {
            surface->Translate(DVec3d::From(-10, -10, -10));

            ASSERT_TRUE(surface->Update().IsValid()) << "Failed to update modified surface";

            CurveVectorPtr expectedBase = CurveVector::CreateLinear({ { 0, 0, 0 },{ 10, 0, 0 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
            ASSERT_TRUE(expectedBase.IsValid()) << "Failed to create imitating surface's base curve vector";

            CurveVectorPtr actualBase = surface->GetSurfaceVector();
            ASSERT_TRUE(actualBase.IsValid()) << "Failed to get surface's curve vector";
            ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase.get(), 0.1)) << "Grid's base curve vector is incorrect";
            }
    }

    {
      // Try changing surface's base curve
    CurveVectorPtr old = surface->GetSurfaceVector();
    ASSERT_TRUE(old.IsValid()) << "Failed to get existing curve vector";
    }
    db.SaveChanges();
    }
    //---------------------------------------------------------------------------------------
    // @betest                                      Martynas.Saulius                02/2018
    //--------------+---------------+---------------+---------------+---------------+-------- 
    TEST_F(GridsTestFixture, GridCurvesSetTests) {

        DgnDbR db = *DgnClientApp::App().Project();
        db.BriefcaseManager().StartBulkOperation();

        { //Creation, Insertion, Update validity
            GridCurvesSetPtr portion = GridCurvesSet::Create(*m_model);
            ASSERT_TRUE(portion.IsValid()) << "Failed to create grid curves portion";

            ASSERT_TRUE(portion->Insert().IsValid()) << "Failed to insert grid curves portion";
            DgnDbStatus stat;

            portion->Update(&stat);
            ASSERT_EQ(DgnDbStatus::Success, stat) << "Failed to update inserted grid curves portion";
        }

        DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), GRIDS_CATEGORY_CODE_Uncategorized);
        { // Check grid curves portion created from handler
            GridCurvesSetHandler& portionHandler = GridCurvesSetHandler::GetHandler();
            DgnClassId portionClassId = db.Domains().GetClassId(portionHandler);
            DgnElement::CreateParams portionParams(db, m_model->GetModelId(), portionClassId);

            GridCurvesSetPtr invalidGridCurvesSet_FromHandler = dynamic_cast<GridCurvesSet *>(portionHandler.Create(portionParams).get());
            ASSERT_TRUE(invalidGridCurvesSet_FromHandler.IsValid()) << "Grid curves portion element created from handler shouldn't be a nullptr";


            invalidGridCurvesSet_FromHandler->SetCategoryId(categoryId);

            ASSERT_TRUE(invalidGridCurvesSet_FromHandler->Insert().IsValid()) << "Grid curves portion element via handler insertion failed";

            ASSERT_TRUE(invalidGridCurvesSet_FromHandler->GetElementId().IsValid()) << "Grid curves portion element id via handler is invalid";
        }
        db.SaveChanges();
    }
//---------------------------------------------------------------------------------------
// @betest                                      Martynas.Saulius                02/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, GridCurveDependancyFromPortionsTest) {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), GRIDS_CATEGORY_CODE_Uncategorized);
    { // Check grid line created from handler
        GridLineHandler& lineHandler = GridLineHandler::GetHandler();
        DgnClassId lineClassId = db.Domains().GetClassId(lineHandler);
        DgnElement::CreateParams lineParams(db, m_model->GetModelId(), lineClassId);

        GridLinePtr invalidGridLine_FromHandler = dynamic_cast<GridLine *>(lineHandler.Create(lineParams).get());
        ICurvePrimitivePtr lineCurve = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 5, 0, 0 });
        invalidGridLine_FromHandler->SetCategoryId(categoryId);
        invalidGridLine_FromHandler->SetCurve(lineCurve);
        DgnDbStatus stat;

        invalidGridLine_FromHandler->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::ValidationFailed, stat) << "Grid Line was inserted into wrong model sucessfully";
    }
    { // Check grid arc created from handler
        GridArcHandler& arcHandler = GridArcHandler::GetHandler();
        DgnClassId arcClassId = db.Domains().GetClassId(arcHandler);
        DgnElement::CreateParams arcParams(db, m_model->GetModelId(), arcClassId);

        GridArcPtr invalidGridArc_FromHandler = dynamic_cast<GridArc *>(arcHandler.Create(arcParams).get());
        ICurvePrimitivePtr arcCurve = GeometryUtils::CreateArc({ 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
        invalidGridArc_FromHandler->SetCategoryId(categoryId);
        invalidGridArc_FromHandler->SetCurve(arcCurve);
        DgnDbStatus stat;

        invalidGridArc_FromHandler->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::ValidationFailed, stat) << "Grid Arc was inserted into wrong model sucessfully";
    }
    { // Check grid spline created from handler
        GridSplineHandler& splineHandler = GridSplineHandler::GetHandler();
        DgnClassId splineClassId = db.Domains().GetClassId(splineHandler);
        DgnElement::CreateParams splineParams(db, m_model->GetModelId(), splineClassId);

        GridSplinePtr invalidGridSpline_FromHandler = dynamic_cast<GridSpline *>(splineHandler.Create(splineParams).get());
        bvector<double> splineWeights = { 1.0, 1.0, 1.0 };
        bvector<double> splineKnots = { 0, 1, 2, 3, 4, 5 };
        ICurvePrimitivePtr splineCurve = ICurvePrimitive::CreateBsplineCurve(MSBsplineCurve::CreateFromPolesAndOrder({ { 0, 0, 0 },{ 10, 0, 0 },{ 0, 10, 0 } }, &splineWeights, &splineKnots, 3, false, false));
        invalidGridSpline_FromHandler->SetCategoryId(categoryId);
        invalidGridSpline_FromHandler->SetCurve(splineCurve);
        DgnDbStatus stat;

        invalidGridSpline_FromHandler->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::ValidationFailed, stat) << "Grid Spline was inserted into wrong model sucessfully";
    }
    db.SaveChanges();
}

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  07/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, TryCreateOrthogonalGridAndSurfaceInSingleRequest) 
{
    DgnDbR db = *DgnClientApp::App ().Project ();
    db.BriefcaseManager().StartBulkOperation();

    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr thisGrid = OrthogonalGrid::Create(createParams);
    ASSERT_TRUE(thisGrid.IsValid());
    ASSERT_TRUE(thisGrid->Insert().IsValid());

    OrthogonalAxisXPtr horizontalAxis = OrthogonalAxisX::CreateAndInsert(*thisGrid);
    OrthogonalAxisYPtr verticalAxis = OrthogonalAxisY::CreateAndInsert(*thisGrid);
    ASSERT_TRUE(horizontalAxis.IsValid());
    ASSERT_TRUE(verticalAxis.IsValid());

    //now try to create the gridSurface
    PlanCartesianGridSurface::CreateParams surfParams(*thisGrid->GetSurfacesModel(), *horizontalAxis, 0.0, 0.0, 10.0, 0.0, 15.0);
    PlanCartesianGridSurfacePtr surface = PlanCartesianGridSurface::Create(surfParams);
    ASSERT_TRUE(surface.IsValid());
    ASSERT_TRUE(surface->Insert().IsValid());
    db.SaveChanges();
}

//---------------------------------------------------------------------------------------
// @betest                                      Elonas.Seviakovas               07/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, IntersectGridSurface_ElevationSurfaceWithoutCurve_SingleIntersectionCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    // Create Orthogonal grid and surface
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGrid = OrthogonalGrid::CreateAndInsert(createParams);
    ASSERT_TRUE(orthogonalGrid.IsValid());

    PlanCartesianGridSurface::CreateParams surfaceParams(*orthogonalGrid->GetSurfacesModel(), *orthogonalGrid->GetXAxis(), 0.0, 0.0, 10.0, 0.0, 15.0);
    PlanCartesianGridSurfacePtr surface = PlanCartesianGridSurface::CreateAndInsert(surfaceParams);
    ASSERT_TRUE(surface.IsValid());

    surface->SetUserLabel("A");
    surface->Update();

    // Create elevation grid and surface
    ElevationGridPtr floorGrid = ElevationGrid::CreateAndInsert(ElevationGrid::CreateParams(*m_model,db.Elements().GetRootSubject()->GetElementId(),"Floor-Grid"));
    ASSERT_TRUE(floorGrid.IsValid());

    floorGrid->SetPlacement(Placement3d({100.0, 100.0, 0.0}, YawPitchRollAngles::FromDegrees(0.0, 0.0, 0.0)));
    floorGrid->Update();

    ElevationGridSurface::CreateParams elevationSurfaceParams(*floorGrid->GetSurfacesModel(), *floorGrid->GetAxis(), nullptr, 5.0);
    ElevationGridSurfacePtr elevationSurface = ElevationGridSurface::Create(elevationSurfaceParams);
    ASSERT_TRUE(elevationSurface.IsValid());

    elevationSurface->SetUserLabel("Floor 4");

    ASSERT_TRUE(elevationSurface->Insert().IsValid());

    // Intersect grids
    GridCurvesSetPtr curvesSet = GridCurvesSet::Create(*m_model);
    curvesSet->Insert();

    BentleyStatus status = orthogonalGrid->IntersectGridSurface(elevationSurface.get(), *curvesSet);
    ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";

    db.SaveChanges();

    auto expectedCurve = ICurvePrimitive::CreateLineString({{10, 0, 5}, {0, 0, 5}});

    // Check if any intersections produced 
    int bundleCount = 0;
    for (ElementIdIteratorEntry const& bundleId : elevationSurface->MakeGridCurveBundleIterator())
        {
        GridCurveBundleCPtr curveBundle = db.Elements().Get<GridCurveBundle>(bundleId.GetElementId());
        if(curveBundle.IsNull())
            continue;

        GridCurveCPtr curve = curveBundle->GetGridCurve();

        ASSERT_TRUE(curve.IsValid()) << "No intersection for a curve bundle computed.";
        
        AxisAlignedBox3d range = curve->CalculateRange3d();
        
        ASSERT_TRUE(range.DiagonalDistance() > 0) << "Curve is zero length";

        ASSERT_TRUE(curve->GetCurve()->IsSameStructureAndGeometry(*expectedCurve, 0.1));

        ASSERT_TRUE(Utf8String(curve->GetUserLabel()) == "Floor 4-A");

        bundleCount++;
        }

    ASSERT_EQ(1, bundleCount) << "Expected intersection count does not match.";
    }

//---------------------------------------------------------------------------------------
// @betest                                      Elonas.Seviakovas               07/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, IntersectGridSurface_CrossingElevationSurfaces_SingleIntersectionCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    // Create elevation grid and surface
    ElevationGridPtr floorGrid1 = ElevationGrid::CreateAndInsert(ElevationGrid::CreateParams(*m_model, db.Elements().GetRootSubject()->GetElementId(), "Floor-Grid1"));
    ASSERT_TRUE(floorGrid1.IsValid());

    Placement3d gridPlacement;
    gridPlacement.TryApplyTransform(Transform::From(RotMatrix::FromRotate90(DVec3d::UnitX())));

    floorGrid1->SetPlacement(gridPlacement);
    floorGrid1->Update();

    CurveVectorPtr surfaceOutline = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    ElevationGridSurface::CreateParams elevationSurfaceParams1(*floorGrid1->GetSurfacesModel(), *floorGrid1->GetAxis(), surfaceOutline.get(), 0.0);
    ElevationGridSurfacePtr elevationSurface1 = ElevationGridSurface::Create(elevationSurfaceParams1);
    ASSERT_TRUE(elevationSurface1.IsValid());

    elevationSurface1->SetUserLabel("Floor 1");

    ASSERT_TRUE(elevationSurface1->Insert().IsValid());

    // Create elevation grid and surface
    ElevationGridCPtr floorGrid2 = ElevationGrid::CreateAndInsert(ElevationGrid::CreateParams(*m_model,db.Elements().GetRootSubject()->GetElementId(),"Floor-Grid2"));
    ASSERT_TRUE(floorGrid2.IsValid());

    ElevationGridSurface::CreateParams elevationSurfaceParams2(*floorGrid2->GetSurfacesModel(), *floorGrid2->GetAxis(), nullptr, 0.0);
    ElevationGridSurfacePtr elevationSurface2 = ElevationGridSurface::Create(elevationSurfaceParams2);
    ASSERT_TRUE(elevationSurface2.IsValid());

    elevationSurface2->SetUserLabel("Floor 2");

    ASSERT_TRUE(elevationSurface2->Insert().IsValid());

    // Intersect grids
    GridCurvesSetPtr curvesSet = GridCurvesSet::Create(*m_model);
    curvesSet->Insert();

    BentleyStatus status = floorGrid1->IntersectGridSurface(elevationSurface2.get(), *curvesSet);
    ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";

    db.SaveChanges();

    // Check if any intersections produced 
    for (ElementIdIteratorEntry const& bundleId : elevationSurface1->MakeGridCurveBundleIterator())
        {
        GridCurveBundleCPtr curveBundle = db.Elements().Get<GridCurveBundle>(bundleId.GetElementId());
        if(curveBundle.IsNull())
            continue;

        GridCurveCPtr curve = curveBundle->GetGridCurve();

        ASSERT_TRUE(curve.IsValid()) << "No intersection for a curve bundle computed.";
        
        AxisAlignedBox3d range = curve->CalculateRange3d();
        
        ASSERT_TRUE(range.DiagonalDistance() > 0) << "Curve is zero length";

        ASSERT_TRUE(Utf8String(curve->GetUserLabel()) == "Floor 1-Floor 2");
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Elonas.Seviakovas               07/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, IntersectGridSurface_ElevationSurfaceWithoutCurveAndAboveOtherSurface_NoIntersectionsCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    // Create Orthogonal grid and surface
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGrid = OrthogonalGrid::CreateAndInsert(createParams);
    ASSERT_TRUE(orthogonalGrid.IsValid());

    PlanCartesianGridSurface::CreateParams surfaceParams(*orthogonalGrid->GetSurfacesModel(), *orthogonalGrid->GetXAxis(), 0.0, 0.0, 10.0, -10.0, -5.0);
    PlanCartesianGridSurfacePtr surface = PlanCartesianGridSurface::CreateAndInsert(surfaceParams);
    ASSERT_TRUE(surface.IsValid());

    // Create elevation grid and surface
    ElevationGridPtr floorGrid = ElevationGrid::CreateAndInsert(ElevationGrid::CreateParams(*m_model,db.Elements().GetRootSubject()->GetElementId(),"Floor-Grid"));
    ASSERT_TRUE(floorGrid.IsValid());

    floorGrid->SetPlacement(Placement3d(DPoint3d{0.0, 0.0, 7.0}, YawPitchRollAngles()));
    floorGrid->Update();

    ElevationGridSurface::CreateParams elevationSurfaceParams(*floorGrid->GetSurfacesModel(), *floorGrid->GetAxis(), nullptr, 10.0);
    ElevationGridSurfacePtr elevationSurface = ElevationGridSurface::Create(elevationSurfaceParams);
    ASSERT_TRUE(elevationSurface.IsValid());
    ASSERT_TRUE(elevationSurface->Insert().IsValid());

    // Intersect grids
    GridCurvesSetPtr curvesSet = GridCurvesSet::Create(*m_model);
    curvesSet->Insert();

    BentleyStatus status = orthogonalGrid->IntersectGridSurface(elevationSurface.get(), *curvesSet);
    ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";

    db.SaveChanges();

    // Check if any intersections produced 
    for (ElementIdIteratorEntry const& bundleId : elevationSurface->MakeGridCurveBundleIterator())
        {
        GridCurveBundleCPtr curveBundle = db.Elements().Get<GridCurveBundle>(bundleId.GetElementId());
        if(curveBundle.IsNull())
            continue;

        GridCurveCPtr curve = curveBundle->GetGridCurve();

        ASSERT_TRUE(curve.IsNull()) << "Curve bundle has a curve";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Elonas.Seviakovas               07/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, IntersectGridSurface_ElevationSurfaceWithGridArcSurface_CreatesIntersection)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    // Create radial grid and surface
    RadialGrid::CreateParams createParams = GetTestDefaultCreateParamsForRadialGrid();
    RadialGridPtr radialGrid = RadialGrid::CreateAndInsert(createParams);
    ASSERT_TRUE(radialGrid.IsValid());

    double radius = 3.33;
    double startAngle = 0.0;
    double endAngle = 2.0;
    PlanCircumferentialGridSurface::CreateParams surfaceParams(*radialGrid->GetSurfacesModel(), *radialGrid->GetCircularAxis(), radius, startAngle, endAngle, 0.0, 5.0);
    PlanCircumferentialGridSurfacePtr surface = PlanCircumferentialGridSurface::CreateAndInsert(surfaceParams);
    ASSERT_TRUE(surface.IsValid());

    // Create elevation grid and surface
    ElevationGridPtr floorGrid = ElevationGrid::CreateAndInsert(ElevationGrid::CreateParams(*m_model,db.Elements().GetRootSubject()->GetElementId(),"Floor-Grid"));
    ASSERT_TRUE(floorGrid.IsValid());

    double elevation = 2.5;
    ElevationGridSurface::CreateParams elevationSurfaceParams(*floorGrid->GetSurfacesModel(), *floorGrid->GetAxis(), nullptr, elevation);
    ElevationGridSurfacePtr elevationSurface = ElevationGridSurface::Create(elevationSurfaceParams);
    ASSERT_TRUE(elevationSurface.IsValid());
    ASSERT_TRUE(elevationSurface->Insert().IsValid());

    // Intersect grids
    GridCurvesSetPtr curvesSet = GridCurvesSet::Create(*m_model);
    curvesSet->Insert();

    BentleyStatus status = radialGrid->IntersectGridSurface(elevationSurface.get(), *curvesSet);
    ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";

    db.SaveChanges();

    // Create an expected curve
    DVec3d xVec = DVec3d::From(radius, 0.0, 0.0);
    DVec3d yVec = DVec3d::From(0.0, radius, 0.0);
    DPoint3d center = DPoint3d::From(0.0, 0.0, elevation);
    DEllipse3d ellipse = DEllipse3d::FromVectors(center, xVec, yVec, startAngle, endAngle - startAngle);
    auto expectedCurve = ICurvePrimitive::CreateArc(ellipse);

    // Check if any intersections produced 
    for (ElementIdIteratorEntry const& bundleId : elevationSurface->MakeGridCurveBundleIterator())
        {
        GridCurveBundleCPtr curveBundle = db.Elements().Get<GridCurveBundle>(bundleId.GetElementId());
        if(curveBundle.IsNull())
            continue;

        GridCurveCPtr curve = curveBundle->GetGridCurve();

        ASSERT_TRUE(curve.IsValid()) << "No intersection for a curve bundle computed.";

        AxisAlignedBox3d range = curve->CalculateRange3d();
        ASSERT_TRUE(range.DiagonalDistance() > 0) << "Curve is zero length";

        ASSERT_TRUE(curve->GetCurve()->IsSameStructureAndGeometry(*expectedCurve, 0.1));
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Elonas.Seviakovas               07/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, IntersectGridSurface_SplineSketchSurface_ReturnsSplineIntersection)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    // Create Orthogonal grid and surface
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGrid = OrthogonalGrid::CreateAndInsert(createParams);
    ASSERT_TRUE(orthogonalGrid.IsValid());

    PlanCartesianGridSurface::CreateParams surfaceParams(*orthogonalGrid->GetSurfacesModel(), *orthogonalGrid->GetXAxis(), 0.0, 0.0, 10.0, 0.0, 20.0);
    PlanCartesianGridSurfacePtr surface = PlanCartesianGridSurface::CreateAndInsert(surfaceParams);
    ASSERT_TRUE(surface.IsValid());

    // Create Sketch grid and surface
    SketchGridPtr sketchGrid = SketchGrid::Create(*m_model, m_model->GetModeledElementId(), "SketchGrid", 0, 1);
    ASSERT_TRUE(sketchGrid.IsValid());

    ASSERT_TRUE(sketchGrid->Insert().IsValid());

    Placement3d gridPlacement;
    gridPlacement.TryApplyTransform(Transform::From(RotMatrix::FromRotate90(DVec3d::UnitX())));
    gridPlacement.TryApplyTransform(Transform::From(DPoint3d::From(5.0, 0.0, 5.0)));

    sketchGrid->SetPlacement(gridPlacement);
    sketchGrid->Update();

    GeneralGridAxisPtr sketchGridAxis = GeneralGridAxis::CreateAndInsert(*sketchGrid);

    DPoint3d points[]{ {1.0, -1.0},  {0.0, 0.0}, {2.0, 1.0}, {3.0, 2.0}, {4.0, 1.0}, {5.0, 1.0} };
    MSBsplineCurvePtr bspline = MSBsplineCurve::CreateFromPolesAndOrder(points, 5, 3);
    ICurvePrimitivePtr splineCurve = ICurvePrimitive::CreateBsplineCurve(bspline);

    SketchSplineGridSurface::CreateParams sketchSurfaceParams(*sketchGrid->GetSurfacesModel(), *sketchGridAxis, -5.0, 5.0, *splineCurve);
    auto sketchSurface = SketchSplineGridSurface::Create(sketchSurfaceParams);

    ASSERT_TRUE(sketchSurface.IsValid());
    ASSERT_TRUE(sketchSurface->Insert().IsValid());

    // Intersect grids
    GridCurvesSetPtr curvesSet = GridCurvesSet::Create(*m_model);
    curvesSet->Insert();

    BentleyStatus status = orthogonalGrid->IntersectGridSurface(sketchSurface.get(), *curvesSet);
    ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";

    db.SaveChanges();

    DPoint3d expectedPoints[]{ {6.0, 0.0, 4.0}, {5.0, 0.0, 5.0}, {7.0, 0.0, 6.0}, {8.0, 0.0, 7.0}, {9.0, 0.0, 6.0}, {10.0, 0.0, 6.0} };
    MSBsplineCurvePtr expectedSpline = MSBsplineCurve::CreateFromPolesAndOrder(expectedPoints, 5, 3);
    ICurvePrimitivePtr expectedCurve = ICurvePrimitive::CreateBsplineCurve(expectedSpline);

    // Check if any intersections produced 
    for (ElementIdIteratorEntry const& bundleId : sketchSurface->MakeGridCurveBundleIterator())
        {
        GridCurveBundleCPtr curveBundle = db.Elements().Get<GridCurveBundle>(bundleId.GetElementId());
        if(curveBundle.IsNull())
            continue;

        GridCurveCPtr curve = curveBundle->GetGridCurve();

        ASSERT_TRUE(curve.IsValid()) << "No intersection for a curve bundle computed.";

        AxisAlignedBox3d range = curve->CalculateRange3d();
        ASSERT_TRUE(range.DiagonalDistance() > 0) << "Curve is zero length";

        ASSERT_TRUE(curve->GetCurve()->IsSameStructureAndGeometry(*expectedCurve, 0.1));
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Elonas.Seviakovas               07/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, IntersectGridSurface_IntersectSecondTimeWithNewSurface_NewIntersectionCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    // Create Orthogonal grid and surface
    OrthogonalGrid::CreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGrid = OrthogonalGrid::CreateAndInsert(createParams);
    ASSERT_TRUE(orthogonalGrid.IsValid());

    PlanCartesianGridSurface::CreateParams surfaceParams(*orthogonalGrid->GetSurfacesModel(), *orthogonalGrid->GetXAxis(), 0.0, 0.0, 10.0, 0.0, 15.0);
    PlanCartesianGridSurfacePtr surface = PlanCartesianGridSurface::CreateAndInsert(surfaceParams);
    ASSERT_TRUE(surface.IsValid());

    // Create elevation grid and surface
    ElevationGridCPtr floorGrid = ElevationGrid::CreateAndInsert(ElevationGrid::CreateParams(*m_model,db.Elements().GetRootSubject()->GetElementId(),"Floor-Grid"));
    ASSERT_TRUE(floorGrid.IsValid());

    ElevationGridSurface::CreateParams elevationSurfaceParams(*floorGrid->GetSurfacesModel(), *floorGrid->GetAxis(), nullptr, 5.0);
    ElevationGridSurfacePtr elevationSurface = ElevationGridSurface::Create(elevationSurfaceParams);
    ASSERT_TRUE(elevationSurface.IsValid());
    ASSERT_TRUE(elevationSurface->Insert().IsValid());

    // Intersect grids
    GridCurvesSetPtr curvesSet = GridCurvesSet::Create(*m_model);
    curvesSet->Insert();

    BentleyStatus status = orthogonalGrid->IntersectGridSurface(elevationSurface.get(), *curvesSet);
    ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";

    db.SaveChanges();

    auto expectedCurve = ICurvePrimitive::CreateLineString({{10, 0, 5}, {0, 0, 5}});

    // Check if any intersections produced 
    int bundleCount = 0;
    for (ElementIdIteratorEntry const& bundleId : elevationSurface->MakeGridCurveBundleIterator())
        {
        GridCurveBundleCPtr curveBundle = db.Elements().Get<GridCurveBundle>(bundleId.GetElementId());
        if(curveBundle.IsNull())
            continue;

        bundleCount++;
        }

    ASSERT_EQ(1, bundleCount) << "Expected intersection count does not match.";

    PlanCartesianGridSurface::CreateParams surfaceParams2(*orthogonalGrid->GetSurfacesModel(), *orthogonalGrid->GetXAxis(), 5.0, 0.0, 10.0, 0.0, 15.0);
    PlanCartesianGridSurfacePtr surface2 = PlanCartesianGridSurface::CreateAndInsert(surfaceParams2);
    ASSERT_TRUE(surface2.IsValid());

    status = orthogonalGrid->IntersectGridSurface(elevationSurface.get(), *curvesSet);
    ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";

    db.SaveChanges();

    bundleCount = 0;
    for (ElementIdIteratorEntry const& bundleId : elevationSurface->MakeGridCurveBundleIterator())
        {
        GridCurveBundleCPtr curveBundle = db.Elements().Get<GridCurveBundle>(bundleId.GetElementId());
        if(curveBundle.IsNull())
            continue;

        bundleCount++;
        }

    ASSERT_EQ(2, bundleCount) << "Expected intersection count does not match.";
    }