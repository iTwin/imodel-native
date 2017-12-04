/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/Grids_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeXml\BeXml.h>
#include <Bentley\BeTest.h>
#include <DgnPlatform\UnitTests\DgnDbTestUtils.h>
#include <DgnPlatform\UnitTests\ScopedDgnHost.h>
#include <Grids/GridsApi.h>
#include <DgnPlatform\FunctionalDomain.h>
#include "GridsTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BUILDING
USING_NAMESPACE_CONSTRAINTMODEL
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
        OrthogonalGrid::StandardCreateParams GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

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
        OrthogonalGrid::StandardCreateParams GetTestDefaultCreateParamsForOrthogonalGridConstrained();

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
        OrthogonalGrid::StandardCreateParams GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended();

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
        OrthogonalGrid::StandardCreateParams GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended();

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
    SubjectCPtr rootSubject = db.Elements ().GetRootSubject ();
    SpatialLocationPartitionCPtr partition = SpatialLocationPartition::CreateAndInsert (*rootSubject, "GridSpatialPartition");
    m_model = SpatialLocationModel::CreateAndInsert (*partition);
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
OrthogonalGrid::StandardCreateParams GridsTestFixture::GetTestDefaultCreateParamsForOrthogonalGridUnconstrained()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    DVec3d horizExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    return OrthogonalGrid::StandardCreateParams(m_model.get(),
                                                       db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                       5, /*horizontal count*/
                                                       4, /*vertical count*/
                                                       15, /*horizontal interval*/
                                                       10, /*vertical interval*/
                                                       50, /*length*/
                                                       70, /*height*/
                                                       horizExtTrans,
                                                       vertExtTrans,
                                                       false, /*create dimensions*/
                                                       false, /*extebd height*/
                                                       "Unconstrained Grid");
    }

//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGrid::StandardCreateParams GridsTestFixture::GetTestDefaultCreateParamsForOrthogonalGridConstrained()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    DVec3d horizExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    return OrthogonalGrid::StandardCreateParams(m_model.get(),
                                                       db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                       5, /*horizontal count*/
                                                       4, /*vertical count*/
                                                       15, /*horizontal interval*/
                                                       10, /*vertical interval*/
                                                       50, /*length*/
                                                       70, /*height*/
                                                       horizExtTrans,
                                                       vertExtTrans,
                                                       true, /*create dimensions*/
                                                       false, /*extebd height*/
                                                       "Constrained Grid");
    }


//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGrid::StandardCreateParams GridsTestFixture::GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    DVec3d horizExtTrans = DVec3d::From(1.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 1.0, 0.0);
    return OrthogonalGrid::StandardCreateParams(m_model.get(),
                                                       db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                       5, /*horizontal count*/
                                                       4, /*vertical count*/
                                                       15, /*horizontal interval*/
                                                       10, /*vertical interval*/
                                                       50, /*length*/
                                                       70, /*height*/
                                                       horizExtTrans,
                                                       vertExtTrans,
                                                       false, /*create dimensions*/
                                                       true, /*extend height*/
                                                       "Unconstrained Grid");
    }

//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGrid::StandardCreateParams GridsTestFixture::GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    DVec3d horizExtTrans = DVec3d::From(1.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 1.0, 0.0);
    return OrthogonalGrid::StandardCreateParams(m_model.get(),
                                                       db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                       5, /*horizontal count*/
                                                       4, /*vertical count*/
                                                       15, /*horizontal interval*/
                                                       10, /*vertical interval*/
                                                       50, /*length*/
                                                       70, /*height*/
                                                       horizExtTrans,
                                                       vertExtTrans,
                                                       true, /*create dimensions*/
                                                       true, /*extend height*/
                                                       "Constrained Grid");
    }

//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
RadialGrid::CreateParams GridsTestFixture::GetTestDefaultCreateParamsForRadialGrid()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    return RadialGrid::CreateParams(m_model.get(),
                                           db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                           7, /*plane count*/
                                           5, /*circular count*/
                                           7 * msGeomConst_pi / 180, /*plane iteration angle*/
                                           13, /*circular interval*/
                                           70, /*length*/
                                           50, /*height*/
                                           "Radial Grid",
                                           false /*extend heihgt*/);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Unconstrained_Created)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGridUnconstrained = OrthogonalGrid::CreateAndInsert (createParams);

    db.SaveChanges ();

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

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
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
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Unconstrained_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGridUnconstrained = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridUnconstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridUnconstrained->TranslateToPoint(newBaseOrigin)) << "orthogonal grid translation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Unconstrained_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGridUnconstrained = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridUnconstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 6; // 30 deg
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridUnconstrained->RotateToAngleXY(newAngle)) << "orthogonal grid rotation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Unconstrained_PlacementCorrectAfterTranslationAndRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPtr orthogonalGridUnconstrained = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridUnconstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridUnconstrained->TranslateToPoint(newBaseOrigin)) << "orthogonal grid translation was not successful";
    double newAngle = msGeomConst_pi / 6; // 30 deg
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridUnconstrained->RotateToAngleXY(newAngle)) << "orthogonal grid rotation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_UnconstrainedExtended_Created)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended();

    OrthogonalGridPtr orthogonalGridUnconstrainedExtended = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

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

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        double length = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetLength(length)) << "Grid surface length should be accessible";
        ASSERT_EQ(52, length) << "Grid surface length is incorrect";

        double height = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetHeight(height)) << "Grid surface height should be accessible";
        ASSERT_EQ(70 + 2 * BUILDING_TOLERANCE, height) << "Grid surface height is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_UnconstrainedExtended_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended();

    OrthogonalGridPtr orthogonalGridUnconstrainedExtended = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridUnconstrainedExtended.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridUnconstrainedExtended->TranslateToPoint(newBaseOrigin)) << "orthogonal grid translation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_UnconstrainedExtended_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridUnconstrainedExtended();

    OrthogonalGridPtr orthogonalGridUnconstrainedExtended = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridUnconstrainedExtended.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 6; // 30 deg
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridUnconstrainedExtended->RotateToAngleXY(newAngle)) << "orthogonal grid rotation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Constrained_Created)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPtr orthogonalGridConstrained = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

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

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
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
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Constrained_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPtr orthogonalGridConstrained = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridConstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridConstrained->TranslateToPoint(newBaseOrigin)) << "orthogonal grid translation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Constrained_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPtr orthogonalGridConstrained = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridConstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 6; // 30 deg
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridConstrained->RotateToAngleXY(newAngle)) << "orthogonal grid rotation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Constrained_PlacementCorrectAfterTranslationAndRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPtr orthogonalGridConstrained = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridConstrained.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridConstrained->TranslateToPoint(newBaseOrigin)) << "orthogonal grid translation was not successful";
    double newAngle = msGeomConst_pi / 6; // 30 deg
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridConstrained->RotateToAngleXY(newAngle)) << "orthogonal grid rotation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_ConstrainedExtended_Created)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended();

    OrthogonalGridPtr orthogonalGridConstrainedExtended = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

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

    // all elements placement rotation angle should be 0
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlanarSurfaceCPtr plane : allElements)
        {
        double length = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetLength(length)) << "Grid surface length should be accessible";
        ASSERT_EQ(52, length) << "Grid surface length is incorrect";

        double height = 0;
        ASSERT_EQ(BentleyStatus::SUCCESS, plane->TryGetHeight(height)) << "Grid surface height should be accessible";
        ASSERT_EQ(70 + 2 * BUILDING_TOLERANCE, height) << "Grid surface height is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_ConstrainedExtended_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended();

    OrthogonalGridPtr orthogonalGridConstrainedExtended = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridConstrainedExtended.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(23, 76, 0);
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridConstrainedExtended->TranslateToPoint(newBaseOrigin)) << "orthogonal grid translation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_ConstrainedExtended_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    OrthogonalGrid::StandardCreateParams createParams = GetTestDefaultCreateParamsForOrthogonalGridConstrainedExtended();

    OrthogonalGridPtr orthogonalGridConstrainedExtended = OrthogonalGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (orthogonalGridConstrainedExtended.IsNull())
        {
        ASSERT_TRUE(false) << "orthogonal grid portion is invalid. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 6; // 30 deg
    ASSERT_EQ(Dgn::RepositoryStatus::Success, orthogonalGridConstrainedExtended->RotateToAngleXY(newAngle)) << "orthogonal grid rotation was not successful";
    db.SaveChanges();

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
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlanarSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }
    
//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGrid_Created)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    RadialGrid::CreateParams createParams = GetTestDefaultCreateParamsForRadialGrid();

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsert(createParams);
    db.SaveChanges();

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
        ASSERT_EQ(i * 7 * msGeomConst_pi / 180, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    // all arc elements rotation angle should be 0
    for (GridArcSurfaceCPtr arc : arcElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(arc->GetPlacement())) << "Grid arc rotation angle is incorrect";
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
        double extendLength = (2 * UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH)) / r;
        double theta = (7 * 7) * msGeomConst_pi / 180 + extendLength;
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
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGrid_PlacementCorrectAfterTranslation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    RadialGrid::CreateParams createParams = GetTestDefaultCreateParamsForRadialGrid();

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (radialGrid.IsNull())
        {
        ASSERT_TRUE(false) << "radial grid portion is invalid. See GridsTestFixture:RadialGrid_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(43, 57, 0);
    ASSERT_EQ(Dgn::RepositoryStatus::Success, radialGrid->TranslateToPoint(newBaseOrigin)) << "radial grid translation was not successful";
    db.SaveChanges();

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
    for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(DPoint3d::From(43, 57, 0).AlmostEqual(surface->GetPlacement().GetOrigin(), 0.1)) << "Grid surface origin is incorrect";
        }

    // all plane elements rotation angle should be i * iteration_angle = i * 7 * msGeomConst_pi / 180
    for (size_t i = 0; i < planeElements.size(); ++i)
        {
        GridPlanarSurfaceCPtr plane = planeElements[i];

        ASSERT_EQ(i * 7 * msGeomConst_pi / 180, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    // all arc elements rotation angle should be 0
    for (GridArcSurfaceCPtr arc : arcElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(arc->GetPlacement())) << "Grid arc rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGrid_PlacementCorrectAfterRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    RadialGrid::CreateParams createParams = GetTestDefaultCreateParamsForRadialGrid();

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (radialGrid.IsNull())
        {
        ASSERT_TRUE(false) << "radial grid portion is invalid. See GridsTestFixture:RadialGrid_Created";
        return;
        }

    double newAngle = msGeomConst_pi / 4; // 45 DEG
    ASSERT_EQ(Dgn::RepositoryStatus::Success, radialGrid->RotateToAngleXY(newAngle)) << "radial grid rotation was not successful";
    db.SaveChanges();

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

        ASSERT_EQ(msGeomConst_pi / 4 + i * 7 * msGeomConst_pi / 180, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    // all arc elements rotation angle should be msGeomConst_pi / 4
    for (GridArcSurfaceCPtr arc : arcElements)
        {
        ASSERT_EQ(msGeomConst_pi / 4, GeometryUtils::PlacementToAngleXY(arc->GetPlacement())) << "Grid arc rotation angle is incorrect";
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGrid_PlacementCorrectAfterTranslationAndRotation)
    {
    DgnDbR db = *DgnClientApp::App().Project();
    RadialGrid::CreateParams createParams = GetTestDefaultCreateParamsForRadialGrid();

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsert(createParams);

    db.SaveChanges();

    if (radialGrid.IsNull())
        {
        ASSERT_TRUE(false) << "radial grid portion is invalid. See GridsTestFixture:RadialGrid_Created";
        return;
        }

    DPoint3d newBaseOrigin = DPoint3d::From(43, 57, 0);
    ASSERT_EQ(Dgn::RepositoryStatus::Success, radialGrid->TranslateToPoint(newBaseOrigin)) << "radial grid translation was not successful";

    double newAngle = msGeomConst_pi / 4; // 45 DEG
    ASSERT_EQ(Dgn::RepositoryStatus::Success, radialGrid->RotateToAngleXY(newAngle)) << "radial grid rotation was not successful";
    db.SaveChanges();

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
    for (GridSurfaceCPtr surface : allElements)
        {
        ASSERT_TRUE(DPoint3d::From(43, 57, 0).AlmostEqual(surface->GetPlacement().GetOrigin(), 0.1)) << "Grid surface origin is incorrect";
        }

    // all plane elements rotation angle should be newAngle + i * iteration_angle = msGeomConst_pi / 4 + i * 7 * msGeomConst_pi / 180
    for (size_t i = 0; i < planeElements.size(); ++i)
        {
        GridPlanarSurfaceCPtr plane = planeElements[i];

        ASSERT_EQ(msGeomConst_pi / 4 + i * 7 * msGeomConst_pi / 180, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    // all arc elements rotation angle should be msGeomConst_pi / 4
    for (GridArcSurfaceCPtr arc : arcElements)
        {
        ASSERT_EQ(msGeomConst_pi / 4, GeometryUtils::PlacementToAngleXY(arc->GetPlacement())) << "Grid arc rotation angle is incorrect";
        }
    }    
    
//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GridsTestFixture, SketchGrid_Created)
    {
    DgnDbR db = *DgnClientApp::App().Project();

    SketchGridPtr sketchGrid = SketchGrid::Create(*m_model.get(), "Sketch Grid");

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(sketchGrid.IsValid()) << "Failed to create sketch grid";

    ASSERT_TRUE(sketchGrid->Insert().IsValid()) << "Failed to insert sketch grid";
    db.SaveChanges();

    ASSERT_TRUE(sketchGrid->GetSurfacesModel().IsValid()) << "Failed to get sketch grid surfaces model";

    int numSurfaces = sketchGrid->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(numSurfaces == 0) << "incorrect number of gridSurfaces in sketchGrid";

    ASSERT_TRUE(0 == std::strcmp("Sketch Grid", sketchGrid->GetName())) << "Grid name is not correct";

    /////////////////////////////////////////////////////////////
    // Check if axes are valid and have correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_EQ(0, sketchGrid->MakeAxesIterator().BuildIdList<DgnElementId>().size()) << "new sketch grid should contain no elements";

    Dgn::DefinitionModelCR defModel = db.GetDictionaryModel();
    Grids::GridAxisPtr gridAxis = GridAxis::CreateAndInsert(defModel, *sketchGrid);

    ASSERT_TRUE(gridAxis.IsValid()) << "Failed to create sketch grid axis";

    ASSERT_EQ(0, gridAxis->MakeIterator().BuildIdList<DgnElementId>().size()) << "New axis should contain no elements";

    /////////////////////////////////////////////////////////////
    // Check if valid grid plane surfaces can be added to sketch grid
    /////////////////////////////////////////////////////////////
    DgnExtrusionDetail planeExtDetail = GeometryUtils::CreatePlaneExtrusionDetail({ 50, 20, 0 }, { 50, 70, 0 }, 90);
    GridPlanarSurfacePtr plane = GridPlanarSurface::Create(*sketchGrid->GetSurfacesModel().get(), gridAxis, planeExtDetail);

    ASSERT_TRUE(plane.IsValid()) << "Failed to create grid plane surface";

    plane->Insert();
    db.SaveChanges();

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
    DgnExtrusionDetail arcExtDetail = GeometryUtils::CreateArcExtrusionDetail(10 /*radius*/, msGeomConst_pi /*base angle*/, 10 /*height*/, 0 /*extend length*/);
    GridArcSurfacePtr arc = GridArcSurface::Create(*sketchGrid->GetSurfacesModel().get(), gridAxis, arcExtDetail);

    ASSERT_TRUE(arc.IsValid()) << "Failed to create grid plane surface";

    arc->Insert();
    db.SaveChanges();

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
    DgnExtrusionDetail splineExtDetail = GeometryUtils::CreateSplineExtrusionDetail({ { 0, 0, 0 },{ 10, 0, 0 },{ 0, 10, 0 } } /*poles*/, 10 /*height*/);
    GridSplineSurfacePtr spline = GridSplineSurface::Create(*sketchGrid->GetSurfacesModel().get(), gridAxis, splineExtDetail);

    ASSERT_TRUE(spline.IsValid()) << "Failed to create grid spline surface";

    spline->Insert();
    db.SaveChanges();

    bvector<DgnElementId> axisElementsAfterSplineInsert = gridAxis->MakeIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(3, axisElementsAfterSplineInsert.size()) << "Axis should contain two elements now";
    ASSERT_TRUE(axisElementsAfterSplineInsert.back().IsValid()) << "Axis' element ids should be valid";
    ASSERT_EQ(axisElementsAfterSplineInsert.back(), spline->GetElementId()) << "The axis element should be the inserted grid spline";

    bvector<DgnElementId> gridElementsAfterSplineInsert = sketchGrid->MakeIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(3, gridElementsAfterSplineInsert.size());
    ASSERT_TRUE(gridElementsAfterSplineInsert.back().IsValid()) << "Grid's element ids should be valid";
    ASSERT_EQ(gridElementsAfterSplineInsert.back(), spline->GetElementId()) << "The grid element should be the inserted grid spline";
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F (GridsTestFixture, InsertHandlerCreatedElements)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();

    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), GRIDS_CATEGORY_CODE_Uncategorized);
    {
    // Check grid plane create from handler
    GridPlanarSurfaceHandler& planeHandler = GridPlanarSurfaceHandler::GetHandler();
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
    GridArcSurfaceHandler& arcHandler = GridArcSurfaceHandler::GetHandler();
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
    GridSplineSurfaceHandler& splineHandler = GridSplineSurfaceHandler::GetHandler();
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
    GridAxisHandler& handler = GridAxisHandler::GetHandler ();
    DgnClassId classId = db.Domains ().GetClassId (handler);
    DgnElement::CreateParams params (db, m_model->GetModelId (), classId);

    DgnElementPtr element = handler.Create (params);
    element->Insert ();

    ASSERT_TRUE (!element->GetElementId ().IsValid ()) << "should fail to insert axis created via handler";
    }
    db.SaveChanges ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F (GridsTestFixture, InsertUpdateInvalidGeometrySurfaces)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();

    SketchGridPtr grid = SketchGrid::Create (*m_model, "SketchGrid-1");
    grid->Insert ();
    GridAxisPtr axis1 = GridAxis::CreateAndInsert (db.GetDictionaryModel (), *grid);

    /////////////////////////////////////////////////////////////
    // Check if invalid grid plane surfaces can't be added to sketch grid
    /////////////////////////////////////////////////////////////
    // Check grid plane from an empty curve vector
    CurveVectorPtr emptyVector = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    GridPlanarSurfacePtr invalidGridPlane_Empty = GridPlanarSurface::Create(*grid->GetSurfacesModel().get(), axis1, emptyVector);

    ASSERT_TRUE(invalidGridPlane_Empty.IsNull()) << "Invalid grid plane surface has been created";

    // Check grid plane created from extrusion with arc as base
    DgnExtrusionDetail arcExtDetail = GeometryUtils::CreateArcExtrusionDetail(10 /*radius*/, msGeomConst_pi /*base angle*/, 10 /*height*/, 0 /*extend length*/);
    GridPlanarSurfacePtr invalidGridPlane_Arc = GridPlanarSurface::Create(*grid->GetSurfacesModel().get(), axis1, arcExtDetail);

    ASSERT_TRUE(invalidGridPlane_Arc.IsNull()) << "Invalid grid plane surface has been created";

    // Check grid plane created from extrusion with spline as base
    DgnExtrusionDetail splineExtDetail = GeometryUtils::CreateSplineExtrusionDetail({ { 0, 0, 0 },{ 10, 0, 0 },{ 0, 10, 0 } } /*poles*/, 10 /*height*/);
    GridPlanarSurfacePtr invalidGridPlane_Spline = GridPlanarSurface::Create(*grid->GetSurfacesModel().get(), axis1, splineExtDetail);

    ASSERT_TRUE(invalidGridPlane_Spline.IsNull()) << "Invalid grid plane surface has been created";

    // Check grid plane created from non planar curve vector
    DPoint3d nonPlanarPoints[] = { { 100,100,0 },{ 200,100,0 },{ 200,200,0 },{ 100,200,100 },{ 100,100,0 } };
    CurveVectorPtr invalidVector;
    invalidVector = CurveVector::Create (CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    ICurvePrimitivePtr prim = ICurvePrimitive::CreateLineString (nonPlanarPoints, 4);
    invalidVector->push_back (prim);
    GridPlanarSurfacePtr invalidPlaneSurface = GridPlanarSurface::Create (*grid->GetSurfacesModel (), axis1, invalidVector->Clone());

    ASSERT_TRUE(invalidPlaneSurface.IsNull()) << "a grid plane with invalid geometry should not be created";

    /////////////////////////////////////////////////////////////
    // Check if invalid grid arc surfaces can't be added to sketch grid
    /////////////////////////////////////////////////////////////
    // Check grid arc created from extrusion with plane as base
    DgnExtrusionDetail planeExtDetail = GeometryUtils::CreatePlaneExtrusionDetail({ 50, 20, 0 }, { 50, 70, 0 }, 90);
    GridArcSurfacePtr invalidGridArc_Plane = GridArcSurface::Create(*grid->GetSurfacesModel().get(), axis1, planeExtDetail);
    ASSERT_TRUE(invalidGridArc_Plane.IsNull()) << "Invalid grid arc surface has been created";

    // Check grid arc created from extrusion with spline as base
    GridArcSurfacePtr invalidGridArc_Spline = GridArcSurface::Create(*grid->GetSurfacesModel().get(), axis1, splineExtDetail);
    ASSERT_TRUE(invalidGridArc_Spline.IsNull()) << "Invalid grid arc surface has been created";

    /////////////////////////////////////////////////////////////
    // Check if invalid grid spline surfaces can't be added to sketch grid
    /////////////////////////////////////////////////////////////
    // Check grid spline created from extrusion with plane as base
    GridSplineSurfacePtr invalidGridSpline_Plane = GridSplineSurface::Create(*grid->GetSurfacesModel().get(), axis1, planeExtDetail);
    ASSERT_TRUE(invalidGridSpline_Plane.IsNull()) << "Invalid grid spline surface has been created";

    // Check grid spline created from extrusion with spline as base
    GridSplineSurfacePtr invalidGridSpline_Arc = GridSplineSurface::Create(*grid->GetSurfacesModel().get(), axis1, arcExtDetail);
    ASSERT_TRUE(invalidGridSpline_Arc.IsNull()) << "Invalid grid spline surface has been created";

    /////////////////////////////////////////////////////////////
    // Check if valid geometry can be set to grid surfaces
    /////////////////////////////////////////////////////////////
    // Check valid plane curve vector
    bvector<DPoint3d> planarPoints = { { 100,100,0 },{ 200,100,0 },{ 200,220,0 },{ 100,200,0 } };
    CurveVectorPtr validVector = CurveVector::CreateLinear(planarPoints, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);

    GridPlanarSurfacePtr validPlaneSurface = GridPlanarSurface::Create(*grid->GetSurfacesModel(), axis1, validVector->Clone());

    validPlaneSurface->Insert();
    ASSERT_TRUE(validPlaneSurface->GetElementId().IsValid()) << "failed to insert a valid-planar gridplanesurface";
    
    CurveVectorPtr validGridPlaneVector = CurveVector::CreateLinear({ { 0, 0, 0 },{ 10, 0, 0 },{ 10, 0, 10 },{ 0, 0, 10 },{ 0, 0, 0 } }, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    validPlaneSurface->SetCurveVector(*validGridPlaneVector.get());

    DgnDbStatus status;
    validPlaneSurface->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status) << "Failed to update plane with valid curve vector";
    db.SaveChanges();

    // Check valid plane extrusion detail
    DgnExtrusionDetail validGridPlaneExtDetail = GeometryUtils::CreatePlaneExtrusionDetail({ 0, 10, 0 }, { 10, 10, 0 }, 50);
    ASSERT_EQ(BentleyStatus::SUCCESS, validPlaneSurface->SetGeometry(ISolidPrimitive::CreateDgnExtrusion(validGridPlaneExtDetail))) << "Failed to set valid dgn extrusion";
    validPlaneSurface->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status) << "Failed to update plane with valid dgn extrusion";
    db.SaveChanges();

    // Check valid arc extrusion detail
    DPoint3d center = DPoint3d::From(0.0, 0.0, 0.0);
    DPoint3d start = DPoint3d::From(0.0, -100.0, 0.0);
    DPoint3d end = DPoint3d::From(0.0, 0.0, 100.0);
    ICurvePrimitivePtr arcPrimitive = ICurvePrimitive::CreateArc(DEllipse3d::FromArcCenterStartEnd(center, start, end));
    CurveVectorPtr arcCurve = CurveVector::Create(arcPrimitive);

    DVec3d extrusionUp = DVec3d::From(100.0, 0.0, 0.0);

    DgnExtrusionDetail extDetail = DgnExtrusionDetail(arcCurve, extrusionUp, false);

    GridArcSurfacePtr validArcSurface = GridArcSurface::Create(*grid->GetSurfacesModel(), axis1, extDetail);

    validArcSurface->Insert();
    ASSERT_TRUE(validArcSurface->GetElementId().IsValid()) << "failed to insert a valid gridarcsurface";
    db.SaveChanges();

    DgnExtrusionDetail validGridArcExtDetail = GeometryUtils::CreateArcExtrusionDetail(20, msGeomConst_pi / 2, 50);
    ASSERT_EQ(BentleyStatus::SUCCESS, validArcSurface->SetGeometry(ISolidPrimitive::CreateDgnExtrusion(validGridArcExtDetail))) << "Failed to set valid dgn extrusion";
    validArcSurface->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status) << "Failed to update arc with valid dgn extrusion";
    db.SaveChanges();

    // Check valid spline extrusion detail
    DgnExtrusionDetail validSplineExtDetail = GeometryUtils::CreateSplineExtrusionDetail({ { 0, 0, 0 },{ 10, 0, 0 },{ 0, 10, 0 } } /*poles*/, 10 /*height*/);
    GridSplineSurfacePtr validSplineSurface = GridSplineSurface::Create(*grid->GetSurfacesModel().get(), axis1, validSplineExtDetail);

    ASSERT_TRUE(validSplineSurface.IsValid()) << "Failed to create grid spline surface";

    validSplineSurface->Insert();
    ASSERT_TRUE(validSplineSurface->GetElementId().IsValid()) << "failed to insert a valid gridsplinesurface";
    db.SaveChanges();

    DgnExtrusionDetail validGridSplineExtDetail = GeometryUtils::CreateSplineExtrusionDetail({ { 0, 0, 0 },{ 10, 0, 0 },{ 10, 30, 0 },{ 0, 0, 0 } }, 50);
    ASSERT_EQ(BentleyStatus::SUCCESS, validSplineSurface->SetGeometry(ISolidPrimitive::CreateDgnExtrusion(validGridSplineExtDetail))) << "Failed to set valid dgn extrusion";
    validSplineSurface->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status) << "Failed to update arc with valid dgn extrusion";
    db.SaveChanges();

    /////////////////////////////////////////////////////////////
    // Check if invalid geometry can't be set to grid surfaces
    /////////////////////////////////////////////////////////////
    // Check grid plane update through geometry builder
    GridPlanarSurfacePtr planeSurfaceToUpdate = db.Elements().GetForEdit<GridPlanarSurface>(validPlaneSurface->GetElementId());

    Dgn::GeometrySourceP geomElem = planeSurfaceToUpdate->ToGeometrySourceP();
    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*geomElem);

    builder->Append(*invalidVector->Clone(), Dgn::GeometryBuilder::CoordSystem::World);
    builder->Finish(*geomElem);
    planeSurfaceToUpdate->Update(&status);
    ASSERT_TRUE(status != DgnDbStatus::Success) << "should fail to update non-planar GridPlanarSurface";
    
    // Check invalid plane curve vector
    validPlaneSurface->SetCurveVector(*emptyVector.get());
    validPlaneSurface->Update(&status);
    ASSERT_NE(DgnDbStatus::Success, status) << "Updating plane with invalid curve vector should not be allowed";
    db.SaveChanges();

    // Check invalid plane extrusion detail
    ASSERT_NE(BentleyStatus::SUCCESS, validPlaneSurface->SetGeometry(ISolidPrimitive::CreateDgnExtrusion(arcExtDetail))) << "Setting invalid dgn extrusion should not be allowed";
    ASSERT_NE(BentleyStatus::SUCCESS, validPlaneSurface->SetGeometry(ISolidPrimitive::CreateDgnExtrusion(splineExtDetail))) << "Setting invalid dgn extrusion should not be allowed";

    // Check invalid arc exturion detail
    ASSERT_NE(BentleyStatus::SUCCESS, validArcSurface->SetGeometry(ISolidPrimitive::CreateDgnExtrusion(planeExtDetail))) << "Setting invalid dgn extrusion should not be allowed";
    ASSERT_NE(BentleyStatus::SUCCESS, validArcSurface->SetGeometry(ISolidPrimitive::CreateDgnExtrusion(splineExtDetail))) << "Setting invalid dgn extrusion should not be allowed";

    // Check invalid spline extrusion detail
    ASSERT_NE(BentleyStatus::SUCCESS, validSplineSurface->SetGeometry(ISolidPrimitive::CreateDgnExtrusion(planeExtDetail))) << "Setting invalid dgn extrusion should not be allowed";
    ASSERT_NE(BentleyStatus::SUCCESS, validSplineSurface->SetGeometry(ISolidPrimitive::CreateDgnExtrusion(arcExtDetail))) << "Setting invalid dgn extrusion should not be allowed";
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGridCurvesAreCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();

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
    
    OrthogonalGridCPtr floorGrid = OrthogonalGrid::CreateAndInsertBySurfaces(floorPlaneCurves, 
                                                                                           bvector<CurveVectorPtr>(),
                                                                                           OrthogonalGrid::CreateParams(m_model.get(),
                                                                                                                               db.Elements().GetRootSubject()->GetElementId(),
                                                                                                                               true,
                                                                                                                               "Floor-Grid"));
    db.SaveChanges();

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
    ASSERT_TRUE(numAxes == 2) << "incorrect number of axes in floorGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);
    ASSERT_TRUE(verticalAxis.IsValid()) << "vertical axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    int numVertical = verticalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE((0 == numHorizontal && 3 == numVertical) || (3 == numHorizontal && 0 == numVertical)) << "One the axes must be empty, the other should contai 3 planes";

    GridAxisCPtr floorAxis = (3 == numHorizontal) ? horizontalAxis : verticalAxis;
    bvector<GridPlanarSurfacePtr> floorGridPlanes;
    for (DgnElementId planeId : floorAxis->MakeIterator().BuildIdList<DgnElementId>())
        {
        GridPlanarSurfacePtr floorSurface = db.Elements().GetForEdit<GridPlanarSurface>(planeId);
        ASSERT_TRUE(floorSurface.IsValid()) << "Failed to get floor plane surface";
        floorGridPlanes.push_back(floorSurface);
        }

    /////////////////////////////////////////////////////////////
    // Create orthogonal grid
    /////////////////////////////////////////////////////////////
    DVec3d horizExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    OrthogonalGrid::StandardCreateParams orthogonalParams = OrthogonalGrid::StandardCreateParams(m_model.get(),
                                                                                                               db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                                                                               2, /*horizontal count*/
                                                                                                               1, /*vertical count*/
                                                                                                               15, /*horizontal interval*/
                                                                                                               10, /*vertical interval*/
                                                                                                               50, /*length*/
                                                                                                               30, /*height*/
                                                                                                               horizExtTrans,
                                                                                                               vertExtTrans,
                                                                                                               true, /*create dimensions*/
                                                                                                               true, /*extend height*/
                                                                                                               "Orthogonal Grid");

    OrthogonalGridPtr orthogonalGrid = OrthogonalGrid::CreateAndInsert(orthogonalParams);
    ASSERT_TRUE(orthogonalGrid.IsValid()) << "Failed to create orthogonal grid";

    db.SaveChanges();

    /////////////////////////////////////////////////////////////
    // Check intersection curves with orthogonal grid
    /////////////////////////////////////////////////////////////
    // Check intersection success
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        BentleyStatus status = orthogonalGrid->IntersectGridSurface(floorGridSurface.get(), *m_model.get());
        ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";
        }

    db.SaveChanges();

    // Check if grid curves are all created and have valid geometry
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        ElementIterator floorGridCurvesIterator = floorGridSurface->MakeCreatedCurvesIterator();
        ASSERT_EQ(3, floorGridCurvesIterator.BuildIdList<DgnElementId>().size());
        
        double elevation = floorGridSurface->GetPlane().origin.z;
        bvector<ICurvePrimitiveCPtr> expectedGeometries =
            {
            ICurvePrimitive::CreateLineString({ { 0, 0, elevation },{ 50, 0, elevation } }),
            ICurvePrimitive::CreateLineString({ { 0, 15, elevation },{ 50, 15, elevation } }),
            ICurvePrimitive::CreateLineString({ { 0, 0, elevation },{ 0, 50, elevation } })
            };

        for (DgnElementId curveId : floorGridCurvesIterator.BuildIdList<DgnElementId>())
            {
            GridCurveCPtr curve = db.Elements().Get<GridCurve>(curveId);
            ASSERT_TRUE(curve.IsValid()) << "Failed to get grid curve";
            ASSERT_NE(expectedGeometries.end(), std::find_if(expectedGeometries.begin(),
                                                             expectedGeometries.end(),
                                                             [&](ICurvePrimitiveCPtr expectedCurve) {return expectedCurve->IsSameStructureAndGeometry(*curve->GetCurve(), 0.1); }))
                << "Grid curve geometry is not as expected";
            }
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, RadialGridCurvesAreCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();

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
    
    OrthogonalGridCPtr floorGrid = OrthogonalGrid::CreateAndInsertBySurfaces(floorPlaneCurves, 
                                                                                           bvector<CurveVectorPtr>(),
                                                                                           OrthogonalGrid::CreateParams(m_model.get(),
                                                                                                                               db.Elements().GetRootSubject()->GetElementId(),
                                                                                                                               true,
                                                                                                                               "Floor-Grid"));
    db.SaveChanges();

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
    ASSERT_TRUE(numAxes == 2) << "incorrect number of axes in floorGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);
    ASSERT_TRUE(verticalAxis.IsValid()) << "vertical axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    int numVertical = verticalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE((0 == numHorizontal && 3 == numVertical) || (3 == numHorizontal && 0 == numVertical)) << "One the axes must be empty, the other should contai 3 planes";

    GridAxisCPtr floorAxis = (3 == numHorizontal) ? horizontalAxis : verticalAxis;
    bvector<GridPlanarSurfacePtr> floorGridPlanes;
    for (DgnElementId planeId : floorAxis->MakeIterator().BuildIdList<DgnElementId>())
        {
        GridPlanarSurfacePtr floorSurface = db.Elements().GetForEdit<GridPlanarSurface>(planeId);
        ASSERT_TRUE(floorSurface.IsValid()) << "Failed to get floor plane surface";
        floorGridPlanes.push_back(floorSurface);
        }

    /////////////////////////////////////////////////////////////
    // Create radial grid
    /////////////////////////////////////////////////////////////
    RadialGrid::CreateParams radialParams = RadialGrid::CreateParams(m_model.get(),
                                                                                   db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                                                   2, /*plane count*/
                                                                                   2, /*arc count*/
                                                                                   30 * msGeomConst_pi / 180, /*plane iteration angle*/
                                                                                   10, /*circular interval*/
                                                                                   50, /*length*/
                                                                                   30, /*height*/
                                                                                   "Radial Grid",
                                                                                   true /*extend height*/);

    RadialGridPtr radialGrid = RadialGrid::CreateAndInsert(radialParams);
    ASSERT_TRUE(radialGrid.IsValid()) << "Failed to create radial grid";

    db.SaveChanges();

    /////////////////////////////////////////////////////////////
    // Check intersection curves with radial grid
    /////////////////////////////////////////////////////////////
    // Check intersection success
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        BentleyStatus status = radialGrid->IntersectGridSurface(floorGridSurface.get(), *m_model.get());
        ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";
        }

    db.SaveChanges();

    // Check if grid curves are all created and have valid geometry
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        ElementIterator floorGridCurvesIterator = floorGridSurface->MakeCreatedCurvesIterator();
        ASSERT_EQ(2, floorGridCurvesIterator.BuildIdList<DgnElementId>().size()); // TODO correct to 4 after arced grid curves can be created
        
        double elevation = floorGridSurface->GetPlane().origin.z;
        bvector<ICurvePrimitiveCPtr> expectedGeometries =
            {
            ICurvePrimitive::CreateLineString({{0, 0, elevation }, 
                                               {50, 0, elevation}}),
            ICurvePrimitive::CreateLineString({{0, 0, elevation}, 
                                               {50 * std::cos(30.0 * msGeomConst_pi / 180.0), 50 * std::sin(30.0 * msGeomConst_pi / 180.0), elevation}}),
            ICurvePrimitive::CreateArc(DEllipse3d::FromArcCenterStartEnd({0, 0, elevation }, 
                                                                         {10 * std::cos(0 - UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH) / 20), 10 * std::sin(0 - UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH) / 20), elevation },
                                                                         {10 * std::cos(30.0 * msGeomConst_pi / 180.0 + UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH) / 20), 10 * std::sin(30.0 * msGeomConst_pi / 180.0 + UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH) / 20), elevation })),
            ICurvePrimitive::CreateArc(DEllipse3d::FromArcCenterStartEnd({0, 0, elevation }, 
                                                                         {20 * std::cos(0 - UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH) / 40), 20 * std::sin(0 - UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH) / 40), elevation },
                                                                         {20 * std::cos(30.0 * msGeomConst_pi / 180.0 + UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH) / 40), 20 * std::sin(30.0 * msGeomConst_pi / 180.0 + UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH) / 40), elevation }))
            };

        for (DgnElementId curveId : floorGridCurvesIterator.BuildIdList<DgnElementId>())
            {
            GridCurveCPtr curve = db.Elements().Get<GridCurve>(curveId);
            ASSERT_TRUE(curve.IsValid()) << "Failed to get grid curve";

            ASSERT_NE(expectedGeometries.end(), std::find_if(expectedGeometries.begin(),
                                                             expectedGeometries.end(),
                                                             [&](ICurvePrimitiveCPtr expectedCurve) {return expectedCurve->IsSameStructureAndGeometry(*curve->GetCurve(), 0.1); }))
                << "Grid curve geometry is not as expected";
            }        
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, SketchGridCurvesAreCreated)
    {
    DgnDbR db = *DgnClientApp::App().Project();

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
    
    OrthogonalGridCPtr floorGrid = OrthogonalGrid::CreateAndInsertBySurfaces(floorPlaneCurves, 
                                                                                           bvector<CurveVectorPtr>(),
                                                                                           OrthogonalGrid::CreateParams(m_model.get(),
                                                                                                                               db.Elements().GetRootSubject()->GetElementId(),
                                                                                                                               true,
                                                                                                                               "Floor-Grid"));
    db.SaveChanges();

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
    ASSERT_TRUE(numAxes == 2) << "incorrect number of axes in floorGrid";

    GridAxisCPtr horizontalAxis = db.Elements().Get<GridAxis>(axesIds[0]);
    ASSERT_TRUE(horizontalAxis.IsValid()) << "horizontal axis is not present";

    GridAxisCPtr verticalAxis = db.Elements().Get<GridAxis>(axesIds[1]);
    ASSERT_TRUE(verticalAxis.IsValid()) << "vertical axis is not present";

    int numHorizontal = horizontalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    int numVertical = verticalAxis->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE((0 == numHorizontal && 3 == numVertical) || (3 == numHorizontal && 0 == numVertical)) << "One the axes must be empty, the other should contai 3 planes";

    GridAxisCPtr floorAxis = (3 == numHorizontal) ? horizontalAxis : verticalAxis;
    bvector<GridPlanarSurfacePtr> floorGridPlanes;
    for (DgnElementId planeId : floorAxis->MakeIterator().BuildIdList<DgnElementId>())
        {
        GridPlanarSurfacePtr floorSurface = db.Elements().GetForEdit<GridPlanarSurface>(planeId);
        ASSERT_TRUE(floorSurface.IsValid()) << "Failed to get floor plane surface";
        floorGridPlanes.push_back(floorSurface);
        }

    /////////////////////////////////////////////////////////////
    // Create sketch grid
    /////////////////////////////////////////////////////////////
    SketchGridPtr sketchGrid = SketchGrid::Create(*m_model.get(), "Sketch Grid");
    ASSERT_TRUE(sketchGrid.IsValid()) << "Failed to create sketch grid";
    ASSERT_TRUE(sketchGrid->Insert().IsValid()) << "Failed to insert sketch grid";
    db.SaveChanges();

    Dgn::DefinitionModelCR defModel = db.GetDictionaryModel();
    Grids::GridAxisPtr gridAxis = GridAxis::CreateAndInsert(defModel, *sketchGrid);

    ASSERT_TRUE(gridAxis.IsValid()) << "Failed to create sketch grid axis";
    db.SaveChanges();

    DgnExtrusionDetail planeExtDetail = GeometryUtils::CreatePlaneExtrusionDetail({ 0, 0, -BUILDING_TOLERANCE }, { 25, 25, -BUILDING_TOLERANCE }, 30 + 2*BUILDING_TOLERANCE);
    GridPlanarSurfacePtr plane = GridPlanarSurface::Create(*sketchGrid->GetSurfacesModel().get(), gridAxis, planeExtDetail);
    ASSERT_TRUE(plane.IsValid()) << "Failed to create grid plane surface";
    plane->Insert();

    DgnExtrusionDetail arcExtDetail = GeometryUtils::CreateArcExtrusionDetail(10 /*radius*/, 30 * msGeomConst_pi / 180 /*base angle*/, 30 + 2 *BUILDING_TOLERANCE /*height*/, 0 /*extend length*/);
    GridArcSurfacePtr arc = GridArcSurface::Create(*sketchGrid->GetSurfacesModel().get(), gridAxis, arcExtDetail);
    ASSERT_TRUE(arc.IsValid()) << "Failed to create grid plane surface";
    arc->Insert();

    DgnExtrusionDetail splineExtDetail = GeometryUtils::CreateSplineExtrusionDetail({ { 0, 0, -BUILDING_TOLERANCE },{ 10, 0, -BUILDING_TOLERANCE },{ 0, 10, -BUILDING_TOLERANCE } } /*poles*/, 30 + 2 * BUILDING_TOLERANCE /*height*/);
    GridSplineSurfacePtr spline = GridSplineSurface::Create(*sketchGrid->GetSurfacesModel().get(), gridAxis, splineExtDetail);
    ASSERT_TRUE(spline.IsValid()) << "Failed to create grid spline surface";
    spline->Insert();

    /////////////////////////////////////////////////////////////
    // Check intersection curves with sketch grid
    /////////////////////////////////////////////////////////////
    // Check intersection success
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        BentleyStatus status = sketchGrid->IntersectGridSurface(floorGridSurface.get(), *m_model.get());
        ASSERT_EQ(BentleyStatus::SUCCESS, status) << "Failed to intersect grid surfaces";
        }

    db.SaveChanges();

    // Check if grid curves are all created and have valid geometry
    for (GridPlanarSurfacePtr floorGridSurface : floorGridPlanes)
        {
        ElementIterator floorGridCurvesIterator = floorGridSurface->MakeCreatedCurvesIterator();
        ASSERT_EQ(1, floorGridCurvesIterator.BuildIdList<DgnElementId>().size()); // TODO correct to 3 after arced and splined grid curves can be created
      
        double elevation = floorGridSurface->GetPlane().origin.z;

        bvector<double> splineWeights = { 1.0, 1.0, 1.0 };
        bvector<double> splineKnots = { 0, 1, 2, 3, 4, 5 };

        bvector<ICurvePrimitiveCPtr> expectedGeometries =
            {
            ICurvePrimitive::CreateLineString({{0, 0, elevation }, 
                                               {25, 25, elevation}}),
            ICurvePrimitive::CreateArc(DEllipse3d::FromArcCenterStartEnd({0, 0, elevation }, 
                                                                         {10 * std::cos(0), 10 * std::sin(0), elevation },
                                                                         {10 * std::cos(30.0 * msGeomConst_pi / 180.0), 10 * std::sin(30.0 * msGeomConst_pi / 180.0), elevation })),
            ICurvePrimitive::CreateBsplineCurve(MSBsplineCurve::CreateFromPolesAndOrder({ { 0, 0, 0 },{ 10, 0, 0 },{ 0, 10, 0 } }, &splineWeights, &splineKnots, 3, false, false))
            };

        for (DgnElementId curveId : floorGridCurvesIterator.BuildIdList<DgnElementId>())
            {
            GridCurveCPtr curve = db.Elements().Get<GridCurve>(curveId);
            ASSERT_TRUE(curve.IsValid()) << "Failed to get grid curve";
            ASSERT_NE(expectedGeometries.end(), std::find_if(expectedGeometries.begin(),
                                                             expectedGeometries.end(),
                                                             [&](ICurvePrimitiveCPtr expectedCurve) {return expectedCurve->IsSameStructureAndGeometry(*curve->GetCurve(), 0.1); }))
                << "Grid curve geometry is not as expected";
            }
        }
    }