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

#define ASSERT_EQ_Plane(lhs, rhs)                                                            \
            ASSERT_TRUE(lhs.origin.AlmostEqual(rhs.origin)) << "Plane origin is incorrect";  \
            ASSERT_TRUE(lhs.normal.AlmostEqual(rhs.normal)) << "Plane normal is incorrect";

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
// @betest                                   Haroldas.Vitunskas                 02/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsStrategyTests, LineGridSurfacePlacementStrategy_ByPoints)
    {
    //////////////////////////////////////
    // Try creating strategy object
    //////////////////////////////////////
    DgnDbR db = *DgnClientApp::App().Project();

    // Create strategy
    LineGridSurfacePlacementStrategyPtr strategy = LineGridSurfacePlacementStrategy::Create(LinePlacementStrategyType::Points);
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create strategy";
    
    double botElevation, topElevation;
    GridAxisCPtr axis;
    Utf8String gridName;
    DPlane3d workingPlane;

    // Check initial bottom elevation
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(0, botElevation) << "Bottom elevation is incorrect";

    // Check initial top elevation
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(0, topElevation) << "Top elevation is incorrect";

    // Check initial working plane
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_WorkingPlane, workingPlane)) << "Failed to get working plane";
    ASSERT_EQ_Plane(DPlane3d::FromOriginAndNormal({ 0, 0, 0 }, DVec3d::From(0, 0, 1)), workingPlane);

    // Check initial axis
    /*
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Axis, axis));
    ASSERT_TRUE(axis.IsNull());
    */ // TODO DgnElement->DgnElementPtr

    // Check initial name
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_TRUE(gridName.empty()) << "Grid name is incorrect";

    //////////////////////////////////////
    // Try modifying sketch properties
    //////////////////////////////////////
    // Try setting bottom elevation
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(10, botElevation) << "Bottom elevation is incorrect";

    // Return bottom elevation to default
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 0.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(0, botElevation) << "Bottom elevation is incorrect";

    // Try setting top elevation
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(10, topElevation) << "Top elevation is incorrect";

    // Return top elevation to default
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 0.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(0, topElevation) << "Top elevation is incorrect";

    // Try setting working plane
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_WorkingPlane, DPlane3d::FromOriginAndNormal({ 5, 0, 0 }, DVec3d::From(1, 0, 0)));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_WorkingPlane, workingPlane)) << "Failed to get working plane";
    ASSERT_EQ_Plane(DPlane3d::FromOriginAndNormal({ 5, 0, 0 }, DVec3d::From(1, 0, 0)), workingPlane);

    // Try setting grid axis
    /*
    Dgn::DefinitionModelCR defModel = db.GetDictionaryModel();
    SketchAxisPtr newAxis = GeneralGridAxis::CreateAndInsert(defModel, *m_sketchGrid);
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Axis, newAxis);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Axis, axis));
    ASSERT_TRUE(axis.IsValid());
    ASSERT_EQ(axis->GetElementId(), newAxis->GetElementId());
    */ // TODO DgnElement->DgnElementPtr
    
    // Return axis to default
    /*
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Axis, nullptr);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Axis, axis));
    ASSERT_TRUE(axis.IsNull());
    */ // TODO DgnElement->DgnElementPtr


    // Try setting grid name
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String("Test grid name"));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_EQ(0, strcmp("Test grid name", gridName.c_str())) << "Grid name is incorrect";

    // Return grid name to default
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String(""));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_TRUE(gridName.empty()) << "Grid name is incorrect";

    //////////////////////////////////////
    // Try adding key points
    //////////////////////////////////////
    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Incorrect number of key points";

    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_EQ(1, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 5, 0, 0 })) << "Incorrect first key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    strategy->AddKeyPoint({ 1, 2, 0 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 5, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 5, 2, 0 })) << "Incorrect second key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    //////////////////////////////////////
    // Try finishing a grid surface
    //////////////////////////////////////
    // Set minimal required properties
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(10, topElevation) << "Top elevation is incorrect";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String("Test grid name"));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_EQ(0, strcmp("Test grid name", gridName.c_str())) << "Grid name is incorrect";

    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    // Finish surface
    GridPlanarSurfacePtr surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    DgnElementId surfaceId = surface->GetElementId();
    ASSERT_TRUE(surfaceId.IsValid()) << "Failed to insert surface";

    GridCPtr grid = Grid::TryGet(db, m_model->GetModeledElementId(), "Test grid name");
    ASSERT_TRUE(grid.IsValid()) << "Failed to create/insert grid";
    ASSERT_TRUE(grid->GetElementId().IsValid());

    bvector<DgnElementId> axesIds = grid->MakeAxesIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(1, axesIds.size());

    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 10, { 5, 0, 0 }, { 5, 2, 0 });

    //////////////////////////////////////
    // Try modifying the grid surface
    //////////////////////////////////////
    // Change bottom elevation. Finished surface should also change
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(5.0, botElevation) << "Bottom elevation is incorrect";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 10, 0, 0 }, { 10, 2, 0 });

    // Change top elevation. Finished surface should also change
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 15.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(15.0, topElevation) << "Top elevation is incorrect";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 10, { 10, 0, 0 }, { 10, 2, 0 });

    // TODO: Change axis

    //////////////////////////////////////
    // Try using dynamic points
    //////////////////////////////////////
    strategy = LineGridSurfacePlacementStrategy::Create(LinePlacementStrategyType::Points);
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create strategy";

    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Incorrect number of key points";

    // Add key point
    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_EQ(1, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    // Add dynamic key point
    strategy->AddDynamicKeyPoint({ 1, 2, 0 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 1, 2, 0 })) << "Incorrect second key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    // Set minimal required properties
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bot elevation";
    ASSERT_EQ(5, botElevation) << "Bot elevation is incorrect";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(10, topElevation) << "Top elevation is incorrect";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String("Test grid name 2"));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_EQ(0, strcmp("Test grid name 2", gridName.c_str())) << "Grid name is incorrect";

    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    // Finish surface
    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    surfaceId = surface->GetElementId();
    ASSERT_TRUE(surfaceId.IsValid()) << "Failed to insert surface";

    grid = Grid::TryGet(db, m_model->GetModeledElementId(), "Test grid name 2");
    ASSERT_TRUE(grid.IsValid()) << "Failed to create/insert grid";
    ASSERT_TRUE(grid->GetElementId().IsValid());

    axesIds = grid->MakeAxesIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(1, axesIds.size());

    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 0, 0, 5 }, { 1, 2, 5 });

    // Change dynamic key point
    strategy->AddDynamicKeyPoint({ 5, 6, 0 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 5, 6, 0 })) << "Incorrect second key point";
    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 0, 0, 5 }, { 5, 6, 5 });
    
    // Try unprojected point. It should be projected onto grid working plane
    strategy->AddDynamicKeyPoint({ 1, 2, 3 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 1, 2, 0 })) << "Incorrect second key point";
    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 0, 0, 5 }, { 1, 2, 5 });

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                   Haroldas.Vitunskas                 02/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsStrategyTests, LineGridSurfacePlacementStrategy_ByPointsLength)
    {
    //////////////////////////////////////
    // Try creating strategy object
    //////////////////////////////////////
    DgnDbR db = *DgnClientApp::App().Project();

    // Create strategy
    LineGridSurfacePlacementStrategyPtr strategy = LineGridSurfacePlacementStrategy::Create(LinePlacementStrategyType::PointsLength);
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create strategy";

    double botElevation, topElevation, length;
    GridAxisCPtr axis;
    Utf8String gridName;
    DPlane3d workingPlane;

    // Check initial bottom elevation
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(0, botElevation) << "Bottom elevation is incorrect";

    // Check initial top elevation
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(0, topElevation) << "Top elevation is incorrect";

    // Check initial working plane
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_WorkingPlane, workingPlane)) << "Failed to get working plane";
    ASSERT_EQ_Plane(DPlane3d::FromOriginAndNormal({ 0, 0, 0 }, DVec3d::From(0, 0, 1)), workingPlane);

    // Check initial axis
    /*
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Axis, axis));
    ASSERT_TRUE(axis.IsNull());
    */ // TODO DgnElement->DgnElementPtr

    // Check initial name
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_TRUE(gridName.empty()) << "Grid name is incorrect";

    // Check initial length
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Length, length)) << "Failed to get grid length";
    ASSERT_EQ(0, length) << "Grid length is incorrect";

    //////////////////////////////////////
    // Try modifying sketch properties
    //////////////////////////////////////
    // Try setting bottom elevation
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(10, botElevation) << "Bottom elevation is incorrect";

    // Return bottom elevation to default
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 0.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(0, botElevation) << "Bottom elevation is incorrect";

    // Try setting top elevation
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(10, topElevation) << "Top elevation is incorrect";

    // Return top elevation to default
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 0.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(0, topElevation) << "Top elevation is incorrect";

    // Try setting working plane
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_WorkingPlane, DPlane3d::FromOriginAndNormal({ 5, 0, 0 }, DVec3d::From(1, 0, 0)));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_WorkingPlane, workingPlane)) << "Failed to get working plane";
    ASSERT_EQ_Plane(DPlane3d::FromOriginAndNormal({ 5, 0, 0 }, DVec3d::From(1, 0, 0)), workingPlane);

    // Try setting grid axis
    /*
    Dgn::DefinitionModelCR defModel = db.GetDictionaryModel();
    SketchAxisPtr newAxis = GeneralGridAxis::CreateAndInsert(defModel, *m_sketchGrid);
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Axis, newAxis);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Axis, axis));
    ASSERT_TRUE(axis.IsValid());
    ASSERT_EQ(axis->GetElementId(), newAxis->GetElementId());
    */ // TODO DgnElement->DgnElementPtr

    // Return axis to default
    /*
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Axis, nullptr);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Axis, axis));
    ASSERT_TRUE(axis.IsNull());
    */ // TODO DgnElement->DgnElementPtr

    // Try setting grid name
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String("Test grid name"));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_EQ(0, strcmp("Test grid name", gridName.c_str())) << "Grid name is incorrect";

    // Return grid name to default
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String(""));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_TRUE(gridName.empty()) << "Grid name is incorrect";

    //////////////////////////////////////
    // Try adding key points and setting length
    //////////////////////////////////////
    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Incorrect number of key points";

    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_EQ(1, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 5, 0, 0 })) << "Incorrect first key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    strategy->AddKeyPoint({ 1, 2, 0 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 5, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 5, 0, 0 })) << "Incorrect second key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Length, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Length, length)) << "Failed to get grid length";
    ASSERT_EQ(10.0, length) << "Grid length is incorrect";

    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 5, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 5, 10, 0 })) << "Incorrect second key point";

    //////////////////////////////////////
    // Try finishing a grid surface
    //////////////////////////////////////
    // Set minimal required properties
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(10, topElevation) << "Top elevation is incorrect";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String("Test grid name"));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_EQ(0, strcmp("Test grid name", gridName.c_str())) << "Grid name is incorrect";

    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    // Finish surface
    GridPlanarSurfacePtr surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    DgnElementId surfaceId = surface->GetElementId();
    ASSERT_TRUE(surfaceId.IsValid()) << "Failed to insert surface";

    GridCPtr grid = Grid::TryGet(db, m_model->GetModeledElementId(), "Test grid name");
    ASSERT_TRUE(grid.IsValid()) << "Failed to create/insert grid";
    ASSERT_TRUE(grid->GetElementId().IsValid());

    bvector<DgnElementId> axesIds = grid->MakeAxesIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(1, axesIds.size());

    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 10, { 5, 0, 0 }, { 5, 10, 0 });

    //////////////////////////////////////
    // Try modifying the grid surface
    //////////////////////////////////////
    // Change bottom elevation. Finished surface should also change
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(5.0, botElevation) << "Bottom elevation is incorrect";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 10, 0, 0 }, { 10, 10, 0 });

    // Change top elevation. Finished surface should also change
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 15.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(15.0, topElevation) << "Top elevation is incorrect";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 10, { 10, 0, 0 }, { 10, 10, 0 });

    // TODO: Change axis

    //////////////////////////////////////
    // Try using dynamic points
    //////////////////////////////////////
    strategy = LineGridSurfacePlacementStrategy::Create(LinePlacementStrategyType::PointsLength);
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create strategy";

    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Incorrect number of key points";

    // Add key point
    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_EQ(1, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    // Add dynamic key point
    strategy->AddDynamicKeyPoint({ 1, 2, 0 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 0, 0, 0 })) << "Incorrect second key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    // Set minimal required properties
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bot elevation";
    ASSERT_EQ(5, botElevation) << "Bot elevation is incorrect";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(10, topElevation) << "Top elevation is incorrect";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String("Test grid name 2"));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_EQ(0, strcmp("Test grid name 2", gridName.c_str())) << "Grid name is incorrect";

    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Length, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Length, length)) << "Failed to get grid length";
    ASSERT_EQ(10.0, length) << "Grid length is incorrect";

    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ std::sqrt(20.0), 2*std::sqrt(20), 0 })) << "Incorrect second key point";

    // Finish surface
    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    surfaceId = surface->GetElementId();
    ASSERT_TRUE(surfaceId.IsValid()) << "Failed to insert surface";

    grid = Grid::TryGet(db, m_model->GetModeledElementId(), "Test grid name 2");
    ASSERT_TRUE(grid.IsValid()) << "Failed to create/insert grid";
    ASSERT_TRUE(grid->GetElementId().IsValid());

    axesIds = grid->MakeAxesIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(1, axesIds.size());

    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 0, 0, 5 }, { std::sqrt(20.0), 2 * std::sqrt(20), 5 });

    // Change dynamic key point
    strategy->AddDynamicKeyPoint({ 5, 6, 0 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 50 / std::sqrt(61), 60 / std::sqrt(61), 0 })) << "Incorrect second key point";
    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 0, 0, 5 }, { 50 / std::sqrt(61), 60 / std::sqrt(61), 5 });

    // Try unprojected point. It should be projected onto grid working plane
    strategy->AddDynamicKeyPoint({ 1, 2, 3 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ std::sqrt(20.0), 2 * std::sqrt(20), 0 })) << "Incorrect second key point";
    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 0, 0, 5 }, { std::sqrt(20.0), 2 * std::sqrt(20), 5 });

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @betest                                   Haroldas.Vitunskas                 02/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsStrategyTests, LineGridSurfacePlacementStrategy_ByPointsAngle)
    {
    //////////////////////////////////////
    // Try creating strategy object
    //////////////////////////////////////
    DgnDbR db = *DgnClientApp::App().Project();

    // Create strategy
    LineGridSurfacePlacementStrategyPtr strategy = LineGridSurfacePlacementStrategy::Create(LinePlacementStrategyType::PointsAngle);
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create strategy";

    double botElevation, topElevation, angle;
    GridAxisCPtr axis;
    Utf8String gridName;
    DPlane3d workingPlane;

    // Check initial bottom elevation
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(0, botElevation) << "Bottom elevation is incorrect";

    // Check initial top elevation
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(0, topElevation) << "Top elevation is incorrect";

    // Check initial working plane
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_WorkingPlane, workingPlane)) << "Failed to get working plane";
    ASSERT_EQ_Plane(DPlane3d::FromOriginAndNormal({ 0, 0, 0 }, DVec3d::From(0, 0, 1)), workingPlane);

    // Check initial axis
    /*
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Axis, axis));
    ASSERT_TRUE(axis.IsNull());
    */ // TODO DgnElement->DgnElementPtr

    // Check initial name
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_TRUE(gridName.empty()) << "Grid name is incorrect";

    // Check initial length
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Angle, angle)) << "Failed to get grid angle";
    ASSERT_EQ(0, angle) << "Grid angle is incorrect";

    //////////////////////////////////////
    // Try modifying sketch properties
    //////////////////////////////////////
    // Try setting bottom elevation
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(10, botElevation) << "Bottom elevation is incorrect";

    // Return bottom elevation to default
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 0.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(0, botElevation) << "Bottom elevation is incorrect";

    // Try setting top elevation
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(10, topElevation) << "Top elevation is incorrect";

    // Return top elevation to default
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 0.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(0, topElevation) << "Top elevation is incorrect";

    // Try setting working plane
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_WorkingPlane, DPlane3d::FromOriginAndNormal({ 5, 0, 0 }, DVec3d::From(1, 0, 0)));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_WorkingPlane, workingPlane)) << "Failed to get working plane";
    ASSERT_EQ_Plane(DPlane3d::FromOriginAndNormal({ 5, 0, 0 }, DVec3d::From(1, 0, 0)), workingPlane);

    // Try setting grid axis
    /*
    Dgn::DefinitionModelCR defModel = db.GetDictionaryModel();
    SketchAxisPtr newAxis = GeneralGridAxis::CreateAndInsert(defModel, *m_sketchGrid);
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Axis, newAxis);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Axis, axis));
    ASSERT_TRUE(axis.IsValid());
    ASSERT_EQ(axis->GetElementId(), newAxis->GetElementId());
    */ // TODO DgnElement->DgnElementPtr

    // Return axis to default
    /*
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Axis, nullptr);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Axis, axis));
    ASSERT_TRUE(axis.IsNull());
    */ // TODO DgnElement->DgnElementPtr

    // Try setting grid name
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String("Test grid name"));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_EQ(0, strcmp("Test grid name", gridName.c_str())) << "Grid name is incorrect";

    // Return grid name to default
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String(""));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_TRUE(gridName.empty()) << "Grid name is incorrect";

    //////////////////////////////////////
    // Try adding key points and setting angle
    //////////////////////////////////////
    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Incorrect number of key points";

    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_EQ(1, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 5, 0, 0 })) << "Incorrect first key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    strategy->AddKeyPoint({ 0, 1, 0 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 5, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 5, 0, -1 })) << "Incorrect second key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Angle, msGeomConst_pi / 4);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Angle, angle)) << "Failed to get grid angle";
    ASSERT_DOUBLE_EQ(msGeomConst_pi / 4, angle) << "Grid angle is incorrect";

    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 5, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ 5, std::sqrt(2.0) / 2, -std::sqrt(2.0) / 2 })) << "Incorrect second key point";

    //////////////////////////////////////
    // Try finishing a grid surface
    //////////////////////////////////////
    // Set minimal required properties
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(10, topElevation) << "Top elevation is incorrect";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String("Test grid name"));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_EQ(0, strcmp("Test grid name", gridName.c_str())) << "Grid name is incorrect";

    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    // Finish surface
    GridPlanarSurfacePtr surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    DgnElementId surfaceId = surface->GetElementId();
    ASSERT_TRUE(surfaceId.IsValid()) << "Failed to insert surface";

    GridCPtr grid = Grid::TryGet(db, m_model->GetModeledElementId(), "Test grid name");
    ASSERT_TRUE(grid.IsValid()) << "Failed to create/insert grid";
    ASSERT_TRUE(grid->GetElementId().IsValid());

    bvector<DgnElementId> axesIds = grid->MakeAxesIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(1, axesIds.size());

    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 10, { 5, 0, 0 }, { 5, std::sqrt(2.0) / 2, -std::sqrt(2.0) / 2 });

    //////////////////////////////////////
    // Try modifying the grid surface
    //////////////////////////////////////
    // Change bottom elevation. Finished surface should also change
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bottom elevation";
    ASSERT_EQ(5.0, botElevation) << "Bottom elevation is incorrect";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 10, 0, 0 }, { 10, std::sqrt(2.0) / 2, -std::sqrt(2.0) / 2 });

    // Change top elevation. Finished surface should also change
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 15.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(15.0, topElevation) << "Top elevation is incorrect";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 10, { 10, 0, 0 }, { 10, std::sqrt(2.0) / 2, -std::sqrt(2.0) / 2 });

    // TODO: Change axis

    //////////////////////////////////////
    // Try using dynamic points
    //////////////////////////////////////
    strategy = LineGridSurfacePlacementStrategy::Create(LinePlacementStrategyType::PointsAngle);
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create strategy";

    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Incorrect number of key points";

    // Add key point
    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_EQ(1, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    // Add dynamic key point
    strategy->AddDynamicKeyPoint({ 1, 2, 0 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ std::sqrt(5.0), 0, 0 })) << "Incorrect second key point";
    ASSERT_FALSE(strategy->IsComplete()) << "Incorrect strategy state";

    // Set minimal required properties
    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_BottomElevation, botElevation)) << "Failed to get bot elevation";
    ASSERT_EQ(5, botElevation) << "Bot elevation is incorrect";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, 10.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_TopElevation, topElevation)) << "Failed to get top elevation";
    ASSERT_EQ(10, topElevation) << "Top elevation is incorrect";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Name, Utf8String("Test grid name 2"));
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Name, gridName)) << "Failed to get grid name";
    ASSERT_EQ(0, strcmp("Test grid name 2", gridName.c_str())) << "Grid name is incorrect";

    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    strategy->SetProperty(LineGridSurfacePlacementStrategy::prop_Angle, msGeomConst_pi / 4);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LineGridSurfacePlacementStrategy::prop_Angle, angle)) << "Failed to get grid angle";
    ASSERT_DOUBLE_EQ(msGeomConst_pi / 4, angle) << "Grid angle is incorrect";

    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ std::sqrt(2.5), std::sqrt(2.5), 0 })) << "Incorrect second key point";

    // Finish surface
    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    surfaceId = surface->GetElementId();
    ASSERT_TRUE(surfaceId.IsValid()) << "Failed to insert surface";

    grid = Grid::TryGet(db, m_model->GetModeledElementId(), "Test grid name 2");
    ASSERT_TRUE(grid.IsValid()) << "Failed to create/insert grid";
    ASSERT_TRUE(grid->GetElementId().IsValid());

    axesIds = grid->MakeAxesIterator().BuildIdList<DgnElementId>();
    ASSERT_EQ(1, axesIds.size());

    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 0, 0, 5 }, { std::sqrt(2.5), std::sqrt(2.5), 5 });

    // Change dynamic key point
    strategy->AddDynamicKeyPoint({ 5, 6, 0 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ std::sqrt(30.5), std::sqrt(30.5), 0 })) << "Incorrect second key point";
    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 0, 0, 5 }, { std::sqrt(30.5), std::sqrt(30.5), 5 });

    // Try unprojected point. It should be projected onto grid working plane
    strategy->AddDynamicKeyPoint({ 1, 2, 3 });
    ASSERT_EQ(2, strategy->GetKeyPoints().size()) << "Incorrect number of key points";
    ASSERT_TRUE(strategy->GetKeyPoints()[0].AlmostEqual({ 0, 0, 0 })) << "Incorrect first key point";
    ASSERT_TRUE(strategy->GetKeyPoints()[1].AlmostEqual({ std::sqrt(2.5), std::sqrt(2.5), 0 })) << "Incorrect second key point";
    ASSERT_TRUE(strategy->IsComplete()) << "Incorrect strategy state";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->FinishElement(*m_model.get()).get());
    ASSERT_TRUE(surface.IsValid()) << "Failed to create surface";
    ASSERT_TRUE(surface->GetElementId().IsValid()) << "Failed to insert surface";
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Surface ID should not have changed";
    CheckGridSurface(surface, grid->GetElementId(), axesIds[0], 5, { 0, 0, 5 }, { std::sqrt(2.5), std::sqrt(2.5), 5 });

    db.SaveChanges();
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
    DPoint3d staPt = DPoint3d::From( 0, 0, 0 );
    GridPlanarSurfacePtr surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint(staPt).get());

    // Try modifying the starting point. This should translate the grid surface
    strategy->AcceptDynamicPoint();
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint({ 5, 5, 0 }).get());
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, staPt, { 5, 5, 0 });
    DgnElementId surfaceId = surface->GetElementId(); // To check if surface is being reused rather than created from scratch

    // Try changing bottom elevation. This should change surface's elevation and height
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetBottomElevation(5).get());
    staPt.z = 5; // Change test point bottom elevation
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 45, staPt, { 5, 5, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change bottom elevation back
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetBottomElevation(0).get());
    staPt.z = 0;
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change top elevation. This should change surface's height
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetTopElevation(45).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 45, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change top elevation back
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetTopElevation(50).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change axis
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetAxis(otherAxis).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), otherAxis->GetElementId(), 50, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change axis back
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetAxis(axis).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change grid and axis
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetGridAndAxis(otherGrid, otherGridAxis).get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 50, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Change grid and axis back
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetGridAndAxis(m_sketchGrid, axis).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try adding another dynamic point. This should change the ending surface point
    surface = dynamic_cast<GridPlanarSurface*>(strategy->AcceptDynamicPoint().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    
    // Try adding another dynamic point. No changes should happen
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint({ 10, 10, 0 }).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try finalizing grid surface. This should end dynamic mode
    surface = dynamic_cast<GridPlanarSurface*>(strategy->Finish().get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, staPt, { 5, 5, 0 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";
    ASSERT_FALSE(strategy->IsInDynamics()) << "Dynamics mode should have ended";

    // Try getting grid surface. This should return invalid grid
    surface = dynamic_cast<GridPlanarSurface*>(strategy->GetGridSurface().get());
    ASSERT_TRUE(surface.IsNull()) << "Finalizing grid should clear up grid strategy parameters so no grid should be created";
    ASSERT_FALSE(strategy->IsInDynamics()) << "Restarted strategy should not be in dynamics mode";

    // Try modifying grid and accepting it
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint(staPt).get());
    ASSERT_TRUE(surface.IsNull()) << "Finalizing grid should clear up grid strategy parameters so no grid should be created";

    strategy->AcceptDynamicPoint();
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetDynamicPoint({ 5, 5, 0 }).get());
    ASSERT_TRUE(strategy->IsInDynamics()) << "Creating grid surface should have started dynamic mode";
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 50, staPt, { 5, 5, 0 });

    surfaceId = surface->GetElementId();
    staPt.z = 5; // Change test point bottom elevation
    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetBottomElevation(5).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 45, staPt, { 5, 5, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetTopElevation(45).get());
    CheckGridSurface(surface, m_sketchGrid->GetElementId(), axis->GetElementId(), 40, staPt, { 5, 5, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    surface = dynamic_cast<GridPlanarSurface*>(strategy->SetGridAndAxis(otherGrid, otherGridAxis).get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 40, staPt, { 5, 5, 5 });
    ASSERT_EQ(surfaceId, surface->GetElementId()) << "Grid surface should have same element id";

    // Try adding another point and finilizing the grid
    surface = dynamic_cast<GridPlanarSurface*>(strategy->AcceptDynamicPoint().get());
    surface = dynamic_cast<GridPlanarSurface*>(strategy->Finish().get());
    CheckGridSurface(surface, otherGrid->GetElementId(), otherGridAxis->GetElementId(), 40, staPt, { 5, 5, 5 });
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