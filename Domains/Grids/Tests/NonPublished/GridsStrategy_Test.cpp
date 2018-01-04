/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GridsStrategy_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeXml\BeXml.h>
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform\UnitTests\DgnDbTestUtils.h>
#include <DgnPlatform\UnitTests\ScopedDgnHost.h>
#include <Grids/GridsApi.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <sstream>
#include <DgnClientFx/DgnClientApp.h>
#include "GridsTestFixtureBase.h"
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX

//=======================================================================================
// Sets up environment for Grid placement strategy unit testing.
// @bsiclass                                    Haroldas.Vitunskas              12/2017
//=======================================================================================
struct GridsStrategyTests : public GridsTestFixtureBase
    {
    GridsStrategyTests() {};
    ~GridsStrategyTests() {};

    void SetUp() override;
    void TearDown() override;

    SpatialLocationModelPtr m_model;
    SketchGridPtr m_sketchGrid;

    void CheckGridSurface(GridPlanarSurfacePtr surface, DgnElementId gridId, DgnElementId axisId, double height, DPoint3d startingPoint, DPoint3d endingPoint);
    void CheckGridSurface(GridArcSurfacePtr surface, DgnElementId gridId, DgnElementId axisId, double height, DPoint3d centerPoint, DPoint3d startingPoint, DPoint3d endingPoint, bool ccw);
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GridsStrategyTests::CheckGridSurface(GridPlanarSurfacePtr surface, DgnElementId gridId, DgnElementId axisId, double height, DPoint3d startingPoint, DPoint3d endingPoint)
    {
    ASSERT_TRUE(surface.IsValid()) << "Grid surface is invalid";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Grid surface has not been inserted";
    ASSERT_EQ(gridId, surface->GetGridId()) << "Grid surface is not owned by the correct grid";
    ASSERT_EQ(axisId, surface->GetAxisId()) << "Grid surface is not owned by the correct axis";

    double actualHeight;
    ASSERT_EQ(BentleyStatus::SUCCESS, surface->TryGetHeight(actualHeight)) << "Failed to get surface height";
    ASSERT_EQ(actualHeight, height) << "surface's height is incorrect";

    CurveVectorPtr actualBase = surface->GetSurfaceVector();
    ASSERT_TRUE(actualBase.IsValid()) << "Failed to get grid's geometry";

    CurveVectorPtr expectedBase = CurveVector::CreateLinear({ startingPoint, endingPoint }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
    CurveVectorPtr expectedBaseReversed = CurveVector::CreateLinear({ endingPoint, startingPoint }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);

    ASSERT_TRUE(expectedBase.IsValid() && expectedBaseReversed.IsValid()) << "Failed to created expected surface geometry";
    ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase, 0.1) ||
                actualBase->IsSameStructureAndGeometry(*expectedBaseReversed, 0.1)) << "Grid surface geometry is incorrect";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GridsStrategyTests::CheckGridSurface(GridArcSurfacePtr surface, DgnElementId gridId, DgnElementId axisId, double height, DPoint3d centerPoint, DPoint3d startingPoint, DPoint3d endingPoint, bool ccw)
    {
    ASSERT_TRUE(surface.IsValid()) << "Grid surface is invalid";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Grid surface has not been inserted";
    ASSERT_EQ(gridId, surface->GetGridId()) << "Grid surface is not owned by the correct grid";
    ASSERT_EQ(axisId, surface->GetAxisId()) << "Grid surface is not owned by the correct axis";

    double actualHeight;
    ASSERT_EQ(BentleyStatus::SUCCESS, surface->TryGetHeight(actualHeight)) << "Failed to get surface height";
    ASSERT_EQ(actualHeight, height) << "surface's height is incorrect";

    CurveVectorPtr actualBase = surface->GetSurfaceVector();
    ASSERT_TRUE(actualBase.IsValid()) << "Failed to get grid's geometry";

    CurveVectorPtr expectedBase = CurveVector::Create(GeometryUtils::CreateArc(centerPoint, startingPoint, endingPoint, ccw));
    
    ASSERT_TRUE(expectedBase.IsValid()) << "Failed to created expected surface geometry";
    ASSERT_TRUE(actualBase->IsSameStructureAndGeometry(*expectedBase, 0.1)) << "Grid surface geometry is incorrect";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GridsStrategyTests::SetUp()
    {
    GridsTestFixtureBase::SetUp();
    DgnDbR db = *DgnClientApp::App().Project();
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SpatialLocationPartitionCPtr partition = SpatialLocationPartition::CreateAndInsert(*rootSubject, "GridSpatialPartition");
    m_model = SpatialLocationModel::CreateAndInsert(*partition);
    m_sketchGrid = SketchGrid::Create(*m_model.get(), partition->GetElementId(), "Sketch grid", 0.0, 10.0);
    m_sketchGrid->Insert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas                 12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
void GridsStrategyTests::TearDown()
    {
    m_sketchGrid = nullptr;
    m_model = nullptr;
    GridsTestFixtureBase::TearDown();
    }

//---------------------------------------------------------------------------------------
// @betest                                   Haroldas.Vitunskas                 12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsStrategyTests, LineGridPlacementStrategyTests)
    {
    //////////////////////////////////////
    // Try creating strategy object
    //////////////////////////////////////
    DgnDbR db = *DgnClientApp::App().Project();
    double elevation = 0;
    double height = 50;
    LineGridPlacementStrategyPtr strategy = LineGridPlacementStrategy::Create(db, elevation, height, m_sketchGrid);
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create line grid placement strategy object";

    //////////////////////////////////////
    // Check if initial strategy parameters are correct
    //////////////////////////////////////
    ASSERT_EQ(elevation, strategy->GetElevation()) << "Initial elevation is incorrect";
    ASSERT_EQ(height, strategy->GetHeight()) << "Initial height is incorrect";
    ASSERT_TRUE(0 == strcmp("Sketch grid", strategy->GetName().c_str())) << "Initial grid name is incorrect";
    ASSERT_EQ(m_sketchGrid->GetElementId(), strategy->GetGrid()->GetElementId()) << "Initial grid is incorrect";
    ASSERT_FALSE(strategy->GetAxis().IsValid()) << "Initial axis is incorrect";
    
    ASSERT_FALSE(strategy->GetGridSurface().IsValid()) << "Initial grid surface is incorrect";
    ASSERT_FALSE(strategy->GetAxis().IsValid()) << "Axis shouldn't be automatically created if there are no points to create grid surface with";

    ASSERT_FALSE(strategy->IsInDynamics()) << "Initially dynamics should be false and only start after successfully creating a grid surface";

    //////////////////////////////////////
    // Try modifying strategy parameters
    //////////////////////////////////////
    {
    // Try a valid bottom elevation modification
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(20).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(20, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(30, strategy->GetHeight()) << "Height incorrect after modifying";

    ASSERT_EQ(nullptr, strategy->SetBottomElevation(40).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(40, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(10, strategy->GetHeight()) << "Height incorrect after modifying";

    // Return to initial state
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(0).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(50, strategy->GetHeight()) << "Height incorrect after modifying";
    
    // Try invalid bottom elevation modification
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(70).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation shouldn't change if invalid value has been passed";
    ASSERT_EQ(50, strategy->GetHeight()) << "Height shouldn't change if invalid value has been passed";
    }
    {
    // Try a valid top elevation modification
    ASSERT_EQ(nullptr, strategy->SetTopElevation(20).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(20, strategy->GetHeight()) << "Height incorrect after modifying";

    ASSERT_EQ(nullptr, strategy->SetTopElevation(30).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(30, strategy->GetHeight()) << "Height incorrect after modifying";

    // Return to initial state
    ASSERT_EQ(nullptr, strategy->SetTopElevation(50).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(50, strategy->GetHeight()) << "Height incorrect after modifying";
    
    // Try invalid bottom elevation modification
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(30).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(nullptr, strategy->SetTopElevation(20).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(30, strategy->GetElevation()) << "Elevation shouldn't change if invalid value has been passed";
    ASSERT_EQ(20, strategy->GetHeight()) << "Height shouldn't change if invalid value has been passed";
    
    // Return to initial state
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(0).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(50, strategy->GetHeight()) << "Height incorrect after modifying";
    }
    GridAxisPtr axis, otherAxis, otherGridAxis;
    SketchGridPtr otherGrid = SketchGrid::Create(*m_model.get(), m_model->GetModeledElementId(), "Sketch grid 2", 0.0, 10.0);
    ASSERT_TRUE(otherGrid.IsValid()) << "Failed to create other grid";
    ASSERT_TRUE(otherGrid->Insert().IsValid()) << "Failed to insert other grid";
    {
    // Try a valid axis modification
    Dgn::DefinitionModelCR defModel = db.GetDictionaryModel();
    axis = GeneralGridAxis::CreateAndInsert(defModel, *m_sketchGrid);
    ASSERT_TRUE(axis.IsValid()) << "Failed to create grid axis";

    ASSERT_EQ(nullptr, strategy->SetAxis(axis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(axis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis is incorrect after modification";

    // Try invalid axis modification
    ASSERT_EQ(nullptr, strategy->SetAxis(nullptr).get()) << "Grid surface should not be created with 0 points";
    ASSERT_TRUE(strategy->GetAxis().IsValid()) << "Axis is incorrect after modification";

    otherGridAxis = GeneralGridAxis::CreateAndInsert(defModel, *otherGrid);
    ASSERT_TRUE(otherGridAxis.IsValid()) << "Failed to create axis";

    ASSERT_EQ(nullptr, strategy->SetAxis(otherGridAxis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_NE(otherGridAxis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis owned by other grid should not be accepted by strategy";

    // Try creating new axis
    otherAxis = strategy->CreateAndInsertNewAxis();
    ASSERT_TRUE(otherAxis.IsValid()) << "Failed to create grid axis";
    ASSERT_NE(otherAxis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis should not have been automatically changed";
    ASSERT_EQ(nullptr, strategy->SetAxis(otherAxis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(otherAxis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis is incorrect after modification";
    
    // Change axis back to default one
    ASSERT_EQ(nullptr, strategy->SetAxis(axis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(axis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis is incorrect after modification";
    }

    {
    // Try a valid grid modification
    ASSERT_EQ(nullptr, strategy->SetGridAndAxis(otherGrid, otherGridAxis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(otherGrid->GetElementId(), strategy->GetGrid()->GetElementId()) << "Grid is incorrect after modification";

    // Try invalid grid modification
    ASSERT_EQ(nullptr, strategy->SetGridAndAxis(nullptr, nullptr).get()) << "Grid surface should not be created with 0 points";
    ASSERT_TRUE(strategy->GetGrid().IsValid()) << "Grid is incorrect after modification";

    // Change grid back to default one
    ASSERT_EQ(nullptr, strategy->SetGridAndAxis(m_sketchGrid, axis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(m_sketchGrid->GetElementId(), strategy->GetGrid()->GetElementId()) << "Grid is incorrect after modification";
    }

    //////////////////////////////////////
    // Try adding points to create grid surface
    //////////////////////////////////////
    // Add single point to initialize surface
    GridPlanarSurfacePtr surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint({ 0, 0, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 0, 0, 0 });
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    DgnElementId surfaceId = surface->GetElementId(); // To check if surface is being reused rather than created from scratch

    // Try modifying the starting point. This should translate the grid surface
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint({ 5, 5, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try changing bottom elevation. This should change surface's elevation and height
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetBottomElevation(5).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 45, { 5, 5, 5 }, { 5, 5, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change bottom elevation back
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetBottomElevation(0).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change top elevation. This should change surface's height
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetTopElevation(45).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 45, { 5, 5, 0 }, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change top elevation back
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetTopElevation(50).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change axis
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetAxis(otherAxis).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), otherAxis->GetElementId(), 50, { 5, 5, 0 }, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change axis back
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetAxis(axis).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change grid and axis
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetGridAndAxis(otherGrid, otherGridAxis).get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 50, { 5, 5, 0 }, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change grid and axis back
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetGridAndAxis(m_sketchGrid, axis).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try adding another dynamic point. This should change the ending surface point
    surface = dynamic_cast<GridPlanarSurface*>(strategy->AcceptDynamicPoint().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint({ 10, 10, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 10, 10, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try adding another dynamic point. No changes should happen
    surface = dynamic_cast<GridPlanarSurface*>(strategy->AcceptDynamicPoint().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 10, 10, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint({ 15, 15, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 10, 10, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try finalizing grid surface. This should end dynamic mode
    surface = dynamic_cast<GridPlanarSurface*>(strategy->Finish().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 5, 5, 0 }, { 10, 10, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    ASSERT_FALSE(strategy->IsInDynamics()) << "Dynamics mode should have ended";

    // Try getting grid surface. This should restart dynamics mode and create a new grid surface with the old parameters but default points
    surface = dynamic_cast<GridPlanarSurface*>(strategy->GetGridSurface().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 0, 0, 0 });
    ASSERT_NE(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    ASSERT_TRUE(strategy->IsInDynamics()) << "Dynamics mode should have restarted";

    // Try modifying grid and accepting it
    surfaceId = surface->GetElementId();
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetBottomElevation(5).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 45, { 0, 0, 5 }, { 0, 0, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetTopElevation(45).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 40, { 0, 0, 5 }, { 0, 0, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetGridAndAxis(otherGrid, otherGridAxis).get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 40, { 0, 0, 5 }, { 0, 0, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Finalizing grid now should fail, because there's only one dynamic point
    surface = dynamic_cast<GridPlanarSurface*>(strategy->Finish().get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 40, { 0, 0, 5 }, { 0, 0, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    ASSERT_TRUE(strategy->IsInDynamics()) << "Dynamics mode should not have ended";

    // Try adding another point and finilizing the grid
    surface = dynamic_cast<GridPlanarSurface*>(strategy->AcceptDynamicPoint().get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 40, { 0, 0, 5 }, { 0, 0, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint({ 15, 15, 0 }).get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 40, { 0, 0, 5 }, { 15, 15, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    
    surface = dynamic_cast<GridPlanarSurface*>(strategy->Finish().get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 40, { 0, 0, 5 }, { 0, 0, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    ASSERT_FALSE(strategy->IsInDynamics()) << "Dynamics mode should have ended";

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                   Haroldas.Vitunskas                 12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsStrategyTests, CSEArcGridPlacementStrategyTests)
    {
    //////////////////////////////////////
    // Try creating strategy object
    //////////////////////////////////////
    DgnDbR db = *DgnClientApp::App().Project();
    double elevation = 0;
    double height = 50;
    CSEArcGridPlacementStrategyPtr strategy = CSEArcGridPlacementStrategy::Create(db, elevation, height, m_sketchGrid);
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create line grid placement strategy object";

    //////////////////////////////////////
    // Check if initial strategy parameters are correct
    //////////////////////////////////////
    ASSERT_EQ(elevation, strategy->GetElevation()) << "Initial elevation is incorrect";
    ASSERT_EQ(height, strategy->GetHeight()) << "Initial height is incorrect";
    ASSERT_TRUE(0 == strcmp("Sketch grid", strategy->GetName().c_str())) << "Initial grid name is incorrect";
    ASSERT_EQ(m_sketchGrid->GetElementId(), strategy->GetGrid()->GetElementId()) << "Initial grid is incorrect";
    ASSERT_FALSE(strategy->GetAxis().IsValid()) << "Initial axis is incorrect";
    
    ASSERT_FALSE(strategy->GetGridSurface().IsValid()) << "Initial grid surface is incorrect";
    ASSERT_FALSE(strategy->GetAxis().IsValid()) << "Axis shouldn't be automatically created if there are no points to create grid surface with";

    ASSERT_FALSE(strategy->IsInDynamics()) << "Initially dynamics should be false and only start after successfully creating a grid surface";

    //////////////////////////////////////
    // Try modifying strategy parameters
    //////////////////////////////////////
    {
    // Try a valid bottom elevation modification
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(20).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(20, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(30, strategy->GetHeight()) << "Height incorrect after modifying";

    ASSERT_EQ(nullptr, strategy->SetBottomElevation(40).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(40, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(10, strategy->GetHeight()) << "Height incorrect after modifying";

    // Return to initial state
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(0).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(50, strategy->GetHeight()) << "Height incorrect after modifying";
    
    // Try invalid bottom elevation modification
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(70).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation shouldn't change if invalid value has been passed";
    ASSERT_EQ(50, strategy->GetHeight()) << "Height shouldn't change if invalid value has been passed";
    }
    {
    // Try a valid top elevation modification
    ASSERT_EQ(nullptr, strategy->SetTopElevation(20).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(20, strategy->GetHeight()) << "Height incorrect after modifying";

    ASSERT_EQ(nullptr, strategy->SetTopElevation(30).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(30, strategy->GetHeight()) << "Height incorrect after modifying";

    // Return to initial state
    ASSERT_EQ(nullptr, strategy->SetTopElevation(50).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(50, strategy->GetHeight()) << "Height incorrect after modifying";
    
    // Try invalid bottom elevation modification
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(30).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(nullptr, strategy->SetTopElevation(20).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(30, strategy->GetElevation()) << "Elevation shouldn't change if invalid value has been passed";
    ASSERT_EQ(20, strategy->GetHeight()) << "Height shouldn't change if invalid value has been passed";
    
    // Return to initial state
    ASSERT_EQ(nullptr, strategy->SetBottomElevation(0).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(0, strategy->GetElevation()) << "Elevation incorrect after modifying";
    ASSERT_EQ(50, strategy->GetHeight()) << "Height incorrect after modifying";
    }
    GridAxisPtr axis, otherAxis, otherGridAxis;
    SketchGridPtr otherGrid = SketchGrid::Create(*m_model.get(), m_model->GetModeledElementId(), "Sketch grid 2", 0.0, 10.0);
    ASSERT_TRUE(otherGrid.IsValid()) << "Failed to create other grid";
    ASSERT_TRUE(otherGrid->Insert().IsValid()) << "Failed to insert other grid";
    {
    // Try a valid axis modification
    Dgn::DefinitionModelCR defModel = db.GetDictionaryModel();
    axis = GeneralGridAxis::CreateAndInsert(defModel, *m_sketchGrid);
    ASSERT_TRUE(axis.IsValid()) << "Failed to create grid axis";

    ASSERT_EQ(nullptr, strategy->SetAxis(axis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(axis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis is incorrect after modification";

    // Try invalid axis modification
    ASSERT_EQ(nullptr, strategy->SetAxis(nullptr).get()) << "Grid surface should not be created with 0 points";
    ASSERT_TRUE(strategy->GetAxis().IsValid()) << "Axis is incorrect after modification";

    otherGridAxis = GeneralGridAxis::CreateAndInsert(defModel, *otherGrid);
    ASSERT_TRUE(otherGridAxis.IsValid()) << "Failed to create axis";

    ASSERT_EQ(nullptr, strategy->SetAxis(otherGridAxis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_NE(otherGridAxis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis owned by other grid should not be accepted by strategy";

    // Try creating new axis
    otherAxis = strategy->CreateAndInsertNewAxis();
    ASSERT_TRUE(otherAxis.IsValid()) << "Failed to create grid axis";
    ASSERT_NE(otherAxis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis should not have been automatically changed";
    ASSERT_EQ(nullptr, strategy->SetAxis(otherAxis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(otherAxis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis is incorrect after modification";
    
    // Change axis back to default one
    ASSERT_EQ(nullptr, strategy->SetAxis(axis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(axis->GetElementId(), strategy->GetAxis()->GetElementId()) << "Axis is incorrect after modification";
    }

    {
    // Try a valid grid modification
    ASSERT_EQ(nullptr, strategy->SetGridAndAxis(otherGrid, otherGridAxis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(otherGrid->GetElementId(), strategy->GetGrid()->GetElementId()) << "Grid is incorrect after modification";

    // Try invalid grid modification
    ASSERT_EQ(nullptr, strategy->SetGridAndAxis(nullptr, nullptr).get()) << "Grid surface should not be created with 0 points";
    ASSERT_TRUE(strategy->GetGrid().IsValid()) << "Grid is incorrect after modification";

    // Change grid back to default one
    ASSERT_EQ(nullptr, strategy->SetGridAndAxis(m_sketchGrid, axis).get()) << "Grid surface should not be created with 0 points";
    ASSERT_EQ(m_sketchGrid->GetElementId(), strategy->GetGrid()->GetElementId()) << "Grid is incorrect after modification";
    }

    //////////////////////////////////////
    // Try adding points to create grid surface
    //////////////////////////////////////
    // Add initial points to initialize surface
    GridArcSurfacePtr surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 0, 0, 0 }).get());
    ASSERT_FALSE(strategy->IsInDynamics()) << "Creating grid surface should not have started dynamic mode";
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";
    surface = dynamic_cast<GridArcSurface*>(strategy->AcceptDynamicPoint().get());
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 5, 0, 0 }).get());
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";
    ASSERT_FALSE(strategy->IsInDynamics()) << "Creating grid surface should not have started dynamic mode";
    surface = dynamic_cast<GridArcSurface*>(strategy->AcceptDynamicPoint().get());
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 4.9, 0.1, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, {5, 0.1, 0}, true);
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    DgnElementId surfaceId = surface->GetElementId(); // To check if surface is being reused rather than created from scratch
    ASSERT_TRUE(surfaceId.IsValid()) << "Surface should have been added";

    // Try modifying the ending point towards {0, 5, 0}. This should make arc sweep ccw
    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 4.8, 0.2, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 5, 0.2, 0 }, true);
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 0.1, 4.9, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 0.1, 5, 0 }, true);
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 0, 5, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try modifying the ending point towards {5, 0, 0}. After the point is beyond the {5, 0, 0} this should make arc sweep cw
    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 4.8, 0.2, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 5, 0.2, 0 }, true);
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 4.9, -0.1, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 5, -0.1, 0 }, false);
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Return point back to {0, 5, 0}
    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 4.8, 0.2, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 5, 0.2, 0 }, true);
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 0, 5, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try changing bottom elevation. This should change surface's elevation and height
    surface = dynamic_cast<GridArcSurface*>(strategy->SetBottomElevation(5).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change bottom elevation back
    surface = dynamic_cast<GridArcSurface*>(strategy->SetBottomElevation(0).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change top elevation. This should change surface's height
    surface = dynamic_cast<GridArcSurface*>(strategy->SetTopElevation(45).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 45, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change top elevation back
    surface = dynamic_cast<GridArcSurface*>(strategy->SetTopElevation(50).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change axis
    surface = dynamic_cast<GridArcSurface*>(strategy->SetAxis(otherAxis).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), otherAxis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change axis back
    surface = dynamic_cast<GridArcSurface*>(strategy->SetAxis(axis).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change grid and axis
    surface = dynamic_cast<GridArcSurface*>(strategy->SetGridAndAxis(otherGrid, otherGridAxis).get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change grid and axis back
    surface = dynamic_cast<GridArcSurface*>(strategy->SetGridAndAxis(m_sketchGrid, axis).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try adding another dynamic point. No changes should happen
    surface = dynamic_cast<GridArcSurface*>(strategy->AcceptDynamicPoint().get()); 
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    
    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 15, 15, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try finalizing grid surface. This should end dynamic mode
    surface = dynamic_cast<GridArcSurface*>(strategy->Finish().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    ASSERT_FALSE(strategy->IsInDynamics()) << "Dynamics mode should have ended";

    // Try getting grid surface. This should return null
    surface = dynamic_cast<GridArcSurface*>(strategy->GetGridSurface().get());
    ASSERT_TRUE(surface.IsNull()) << "Surface should not be created with less than 3 points";
    ASSERT_FALSE(strategy->IsInDynamics()) << "Dynamics mode should not have restarted";

    // Try modifying grid and accepting it
    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 0, 0, 0 }).get());
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";
    surface = dynamic_cast<GridArcSurface*>(strategy->AcceptDynamicPoint().get());
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 5, 0, 0 }).get());
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    surface = dynamic_cast<GridArcSurface*>(strategy->AcceptDynamicPoint().get());
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 4.9, 0.1, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 5, 0.1, 0 }, true);
    ASSERT_TRUE(surface.IsNull()) << "Arc should not be created with less than 3 points";
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    surfaceId = surface->GetElementId(); // To check if surface is being reused rather than created from scratch
    ASSERT_TRUE(surfaceId.IsValid()) << "Surface should have been added";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetDynamicPoint({ 0, 5, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetBottomElevation(5).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 45, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetTopElevation(45).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 40, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridArcSurface*>(strategy->SetGridAndAxis(otherGrid, otherGridAxis).get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 45, { 0, 0, 5 }, { 5, 0, 5 }, { 0, 5, 5 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Finalizing grid now should fail, because there's the dynamic point has not been accepted
    surface = dynamic_cast<GridArcSurface*>(strategy->Finish().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    ASSERT_TRUE(strategy->IsInDynamics()) << "Dynamics mode should not have ended";

    // Try adding another point and finilizing the grid
    surface = dynamic_cast<GridArcSurface*>(strategy->AcceptDynamicPoint().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridArcSurface*>(strategy->Finish().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, { 0, 0, 0 }, { 5, 0, 0 }, { 0, 5, 0 }, true);
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    ASSERT_FALSE(strategy->IsInDynamics()) << "Dynamics mode should have ended";

    db.SaveChanges();
    }