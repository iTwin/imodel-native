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
        OrthogonalGridPortion::StandardCreateParams GetCreateParamsForOrthogonalGridUnconstrained();

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
        OrthogonalGridPortion::StandardCreateParams GetCreateParamsForOrthogonalGridConstrained();

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
        OrthogonalGridPortion::StandardCreateParams GetCreateParamsForOrthogonalGridUnconstrainedExtended();

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
        OrthogonalGridPortion::StandardCreateParams GetCreateParamsForOrthogonalGridConstrainedExtended();

        ////! Utility for testing
        ////! returns create params for radial grid with values:
        ////! Plane count = 7
        ////! Circular count = 5
        ////! Plane iteration angle = 7
        ////! Circular interval = 13
        ////! Length = 70
        ////! Height = 50
        ////! Extension = false;
        //RadialGridPortion::CreateParams GetCreateParamsForRadialGrid();
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
OrthogonalGridPortion::StandardCreateParams GridsTestFixture::GetCreateParamsForOrthogonalGridUnconstrained()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    DVec3d normal = DVec3d::From(1.0, 0.0, 0.0);
    DVec3d horizExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    return OrthogonalGridPortion::StandardCreateParams(m_model.get(),
                                                       db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                       5, /*horizontal count*/
                                                       4, /*vertical count*/
                                                       15, /*horizontal interval*/
                                                       10, /*vertical interval*/
                                                       50, /*length*/
                                                       70, /*height*/
                                                       normal,
                                                       horizExtTrans,
                                                       vertExtTrans,
                                                       false, /*create dimensions*/
                                                       false, /*extebd height*/
                                                       "Unconstrained Grid");
    }

//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGridPortion::StandardCreateParams GridsTestFixture::GetCreateParamsForOrthogonalGridConstrained()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    DVec3d normal = DVec3d::From(1.0, 0.0, 0.0);
    DVec3d horizExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 0.0, 0.0);
    return OrthogonalGridPortion::StandardCreateParams(m_model.get(),
                                                       db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                       5, /*horizontal count*/
                                                       4, /*vertical count*/
                                                       15, /*horizontal interval*/
                                                       10, /*vertical interval*/
                                                       50, /*length*/
                                                       70, /*height*/
                                                       normal,
                                                       horizExtTrans,
                                                       vertExtTrans,
                                                       true, /*create dimensions*/
                                                       false, /*extebd height*/
                                                       "Constrained Grid");
    }


//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGridPortion::StandardCreateParams GridsTestFixture::GetCreateParamsForOrthogonalGridUnconstrainedExtended()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    DVec3d normal = DVec3d::From(1.0, 0.0, 0.0);
    DVec3d horizExtTrans = DVec3d::From(1.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 1.0, 0.0);
    return OrthogonalGridPortion::StandardCreateParams(m_model.get(),
                                                       db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                       5, /*horizontal count*/
                                                       4, /*vertical count*/
                                                       15, /*horizontal interval*/
                                                       10, /*vertical interval*/
                                                       50, /*length*/
                                                       70, /*height*/
                                                       normal,
                                                       horizExtTrans,
                                                       vertExtTrans,
                                                       false, /*create dimensions*/
                                                       true, /*extend height*/
                                                       "Unconstrained Grid");
    }

//---------------------------------------------------------------------------------------
// @bemethod                                      Haroldas.Vitunskas              11/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
OrthogonalGridPortion::StandardCreateParams GridsTestFixture::GetCreateParamsForOrthogonalGridConstrainedExtended()
    {
    DgnDbR db = *DgnClientApp::App().Project();
    DVec3d normal = DVec3d::From(1.0, 0.0, 0.0);
    DVec3d horizExtTrans = DVec3d::From(1.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From(0.0, 1.0, 0.0);
    return OrthogonalGridPortion::StandardCreateParams(m_model.get(),
                                                       db.Elements().GetRootSubject()->GetElementId(), /*parent element*/
                                                       5, /*horizontal count*/
                                                       4, /*vertical count*/
                                                       15, /*horizontal interval*/
                                                       10, /*vertical interval*/
                                                       50, /*length*/
                                                       70, /*height*/
                                                       normal,
                                                       horizExtTrans,
                                                       vertExtTrans,
                                                       true, /*create dimensions*/
                                                       true, /*extend height*/
                                                       "Constrained Grid");
    }

////---------------------------------------------------------------------------------------
//// @bemethod                                      Haroldas.Vitunskas              11/2017
////--------------+---------------+---------------+---------------+---------------+-------- 
//RadialGridPortion::CreateParams GetCreateParamsForRadialGrid()
//    {
//
//    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, OrthogonalGrid_Unconstrained_Created)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPortionPtr orthogonalGridUnconstrained = OrthogonalGridPortion::CreateAndInsert (createParams);

    db.SaveChanges ();

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE (orthogonalGridUnconstrained.IsValid()) << "Failed to create orthogonal grid";

    ASSERT_TRUE (orthogonalGridUnconstrained->GetSurfacesModel().IsValid()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridUnconstrained->MakeIterator ().BuildIdList<DgnElementId> ().size ();
    ASSERT_TRUE (numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";


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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr plane = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        ASSERT_TRUE(plane.IsValid()) << "horizontal element invalid";
        horizontalElements.push_back(plane);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr plane = db.Elements().Get<GridPlaneSurface>(verticalElementId);
        ASSERT_TRUE(plane.IsValid()) << "vertical element invalid";
        verticalElements.push_back(plane);
        }

    /////////////////////////////////////////////////////////////
    // Check if axes elements parallel and perpendicular
    /////////////////////////////////////////////////////////////
    // for any two elements e_1 and e_2 : if both elements are in same axis, they must be parallel, otherwise they must be perpendicular
    bvector<GridPlaneSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), horizontalElements.begin(), horizontalElements.end());
    allElements.insert(allElements.end(), verticalElements.begin(), verticalElements.end());

    for (GridPlaneSurfaceCPtr firstElem : allElements)
        {
        for (GridPlaneSurfaceCPtr secondElem : allElements)
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
    for (GridPlaneSurfaceCPtr plane : allElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlaneSurfaceCPtr plane : allElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPortionPtr orthogonalGridUnconstrained = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPortionPtr orthogonalGridUnconstrained = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridUnconstrained();

    OrthogonalGridPortionPtr orthogonalGridUnconstrained = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Unconstrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridUnconstrainedExtended();

    OrthogonalGridPortionPtr orthogonalGridUnconstrainedExtended = OrthogonalGridPortion::CreateAndInsert(createParams);

    db.SaveChanges();

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(orthogonalGridUnconstrainedExtended.IsValid()) << "Failed to create orthogonal grid";

    ASSERT_TRUE(orthogonalGridUnconstrainedExtended->GetSurfacesModel().IsValid()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridUnconstrainedExtended->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";


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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr plane = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        ASSERT_TRUE(plane.IsValid()) << "horizontal element invalid";
        horizontalElements.push_back(plane);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr plane = db.Elements().Get<GridPlaneSurface>(verticalElementId);
        ASSERT_TRUE(plane.IsValid()) << "vertical element invalid";
        verticalElements.push_back(plane);
        }

    /////////////////////////////////////////////////////////////
    // Check if axes elements parallel and perpendicular
    /////////////////////////////////////////////////////////////
    // for any two elements e_1 and e_2 : if both elements are in same axis, they must be parallel, otherwise they must be perpendicular
    bvector<GridPlaneSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), horizontalElements.begin(), horizontalElements.end());
    allElements.insert(allElements.end(), verticalElements.begin(), verticalElements.end());

    for (GridPlaneSurfaceCPtr firstElem : allElements)
        {
        for (GridPlaneSurfaceCPtr secondElem : allElements)
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
    for (GridPlaneSurfaceCPtr plane : allElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlaneSurfaceCPtr plane : allElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridUnconstrainedExtended();

    OrthogonalGridPortionPtr orthogonalGridUnconstrainedExtended = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridUnconstrainedExtended();

    OrthogonalGridPortionPtr orthogonalGridUnconstrainedExtended = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_UnconstrainedExtended_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPortionPtr orthogonalGridConstrained = OrthogonalGridPortion::CreateAndInsert(createParams);

    db.SaveChanges();

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(orthogonalGridConstrained.IsValid()) << "Failed to create orthogonal grid";

    ASSERT_TRUE(orthogonalGridConstrained->GetSurfacesModel().IsValid()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridConstrained->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";


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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr plane = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        ASSERT_TRUE(plane.IsValid()) << "horizontal element invalid";
        horizontalElements.push_back(plane);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr plane = db.Elements().Get<GridPlaneSurface>(verticalElementId);
        ASSERT_TRUE(plane.IsValid()) << "vertical element invalid";
        verticalElements.push_back(plane);
        }

    /////////////////////////////////////////////////////////////
    // Check if axes elements parallel and perpendicular
    /////////////////////////////////////////////////////////////
    // for any two elements e_1 and e_2 : if both elements are in same axis, they must be parallel, otherwise they must be perpendicular
    bvector<GridPlaneSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), horizontalElements.begin(), horizontalElements.end());
    allElements.insert(allElements.end(), verticalElements.begin(), verticalElements.end());

    for (GridPlaneSurfaceCPtr firstElem : allElements)
        {
        for (GridPlaneSurfaceCPtr secondElem : allElements)
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
    for (GridPlaneSurfaceCPtr plane : allElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlaneSurfaceCPtr plane : allElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPortionPtr orthogonalGridConstrained = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPortionPtr orthogonalGridConstrained = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridConstrained();

    OrthogonalGridPortionPtr orthogonalGridConstrained = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_Constrained_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridConstrainedExtended();

    OrthogonalGridPortionPtr orthogonalGridConstrainedExtended = OrthogonalGridPortion::CreateAndInsert(createParams);

    db.SaveChanges();

    /////////////////////////////////////////////////////////////
    // Check if grid is valid and has correct number of elements
    /////////////////////////////////////////////////////////////
    ASSERT_TRUE(orthogonalGridConstrainedExtended.IsValid()) << "Failed to create orthogonal grid";

    ASSERT_TRUE(orthogonalGridConstrainedExtended->GetSurfacesModel().IsValid()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridConstrainedExtended->MakeIterator().BuildIdList<DgnElementId>().size();
    ASSERT_TRUE(numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";


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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr plane = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        ASSERT_TRUE(plane.IsValid()) << "horizontal element invalid";
        horizontalElements.push_back(plane);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr plane = db.Elements().Get<GridPlaneSurface>(verticalElementId);
        ASSERT_TRUE(plane.IsValid()) << "vertical element invalid";
        verticalElements.push_back(plane);
        }

    /////////////////////////////////////////////////////////////
    // Check if axes elements parallel and perpendicular
    /////////////////////////////////////////////////////////////
    // for any two elements e_1 and e_2 : if both elements are in same axis, they must be parallel, otherwise they must be perpendicular
    bvector<GridPlaneSurfaceCPtr> allElements;
    allElements.insert(allElements.end(), horizontalElements.begin(), horizontalElements.end());
    allElements.insert(allElements.end(), verticalElements.begin(), verticalElements.end());

    for (GridPlaneSurfaceCPtr firstElem : allElements)
        {
        for (GridPlaneSurfaceCPtr secondElem : allElements)
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
    for (GridPlaneSurfaceCPtr plane : allElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    /////////////////////////////////////////////////////////////
    // Check if grid elements have correct length and height
    /////////////////////////////////////////////////////////////
    for (GridPlaneSurfaceCPtr plane : allElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridConstrainedExtended();

    OrthogonalGridPortionPtr orthogonalGridConstrainedExtended = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(0, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
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
    OrthogonalGridPortion::StandardCreateParams createParams = GetCreateParamsForOrthogonalGridConstrainedExtended();

    OrthogonalGridPortionPtr orthogonalGridConstrainedExtended = OrthogonalGridPortion::CreateAndInsert(createParams);

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
    bvector<GridPlaneSurfaceCPtr> horizontalElements;
    for (DgnElementId horizontalElementId : horizontalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(horizontalElementId);
        if (surface.IsNull())
            {
            ASSERT_TRUE(false) << "Grid surface is invalid. See GridsTestFixture:OrthogonalGrid_ConstrainedExtended_Created";
            return;
            }

        horizontalElements.push_back(surface);
        }

    bvector<DgnElementId> verticalElementIds = verticalAxis->MakeIterator().BuildIdList<DgnElementId>();
    bvector<GridPlaneSurfaceCPtr> verticalElements;
    for (DgnElementId verticalElementId : verticalElementIds)
        {
        GridPlaneSurfaceCPtr surface = db.Elements().Get<GridPlaneSurface>(verticalElementId);
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
    for (GridPlaneSurfaceCPtr plane : horizontalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }

    for (GridPlaneSurfaceCPtr plane : verticalElements)
        {
        ASSERT_EQ(newAngle, GeometryUtils::PlacementToAngleXY(plane->GetPlacement())) << "Grid plane rotation angle is incorrect";
        }
    }

