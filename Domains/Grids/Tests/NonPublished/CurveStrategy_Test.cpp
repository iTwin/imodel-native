/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/CurveStrategy_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct CurveStrategyTests : public GridsTestFixtureBase
    {
    CurveStrategyTests() {};
    ~CurveStrategyTests() {};
    };

void CompareCurves(ICurvePrimitivePtr lhs, ICurvePrimitivePtr rhs)
    {
    ASSERT_EQ(lhs.IsValid(), rhs.IsValid()) << "Both curves should be either valid or invalid";
    ASSERT_TRUE(lhs->IsSameStructureAndGeometry(*rhs, 0.1)) << "Curves should have same geometry";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveStrategyTests, LineByPointsTests)
    {
    LinePointsPlacementStrategyPtr strategy = LinePointsPlacementStrategy::Create();
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create strategy";

    // Check initial state
    DPoint3d startPoint, endPoint;
    ASSERT_EQ(BentleyStatus::ERROR, strategy->GetStartPoint(startPoint)) << "Start point should not be initially set";
    ASSERT_EQ(BentleyStatus::ERROR, strategy->GetEndPoint(endPoint)) << "End point should not be initially set";

    ICurvePrimitivePtr createdCurve = strategy->GetCurvePrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 0 points";

    // Try adding points
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->AddPoint({ 0, 0, 0 })) << "Adding point should not fail";

    createdCurve = strategy->GetCurvePrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 1 point";

    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->AddPoint({ 5, 5, 0 })) << "Adding point should not fail";

    createdCurve = strategy->GetCurvePrimitive();
    ASSERT_TRUE(createdCurve.IsValid()) << "Failed to create curve";

    ICurvePrimitivePtr expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 5, 5, 0 });
    CompareCurves(createdCurve, expected);

    ASSERT_EQ(BentleyStatus::ERROR, strategy->AddPoint({ 15, 15, 5 })) << "Adding 3rd point should fail";

    createdCurve = strategy->Reset();
    ASSERT_TRUE(createdCurve.IsValid()) << "Failed to create final curve";
    CompareCurves(createdCurve, expected);
    ASSERT_EQ(BentleyStatus::ERROR, strategy->GetStartPoint(startPoint)) << "Start point should not be initially set";
    ASSERT_EQ(BentleyStatus::ERROR, strategy->GetEndPoint(endPoint)) << "End point should not be initially set";
    ASSERT_TRUE(strategy->GetCurvePrimitive().IsNull()) << "no curve should be created with 0 points";

    // Try adding points with properties interface
    ASSERT_EQ(BentleyStatus::ERROR, strategy->SetEndPoint({ 15, 15, 15 })) << "Setting end point without setting strating point should fail";
    
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->SetStartPoint({ 7, 32, 4 })) << "Setting start point should not fail";
    ASSERT_TRUE(strategy->GetCurvePrimitive().IsNull()) << "no curve should be created with 1 point";

    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->SetEndPoint({ 17, 37, 6 })) << "Setting end point should not fail";
    
    createdCurve = strategy->GetCurvePrimitive();
    ASSERT_TRUE(createdCurve.IsValid()) << "Failed to create curve";

    expected = ICurvePrimitive::CreateLine({ 7, 32, 4 }, { 17, 37, 6 });
    CompareCurves(createdCurve, expected);

    // Try modifying points
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->SetStartPoint({ 13, 34, 0 })) << "Setting start point should not fail";
    createdCurve = strategy->GetCurvePrimitive();
    ASSERT_TRUE(createdCurve.IsValid()) << "Failed to create curve";

    expected = ICurvePrimitive::CreateLine({ 13, 34, 0 }, { 17, 37, 6 });
    CompareCurves(createdCurve, expected);

    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->SetEndPoint({ 57, 46, 23 })) << "Setting end point should not fail";

    createdCurve = strategy->GetCurvePrimitive();
    ASSERT_TRUE(createdCurve.IsValid()) << "Failed to create curve";

    expected = ICurvePrimitive::CreateLine({ 13, 34, 0 }, { 57, 46, 23 });
    CompareCurves(createdCurve, expected);

    createdCurve = strategy->Reset();
    ASSERT_TRUE(createdCurve.IsValid()) << "Failed to create final curve";
    CompareCurves(createdCurve, expected);
    ASSERT_EQ(BentleyStatus::ERROR, strategy->GetStartPoint(startPoint)) << "Start point should not be initially set";
    ASSERT_EQ(BentleyStatus::ERROR, strategy->GetEndPoint(endPoint)) << "End point should not be initially set";
    ASSERT_TRUE(strategy->GetCurvePrimitive().IsNull()) << "no curve should be created with 0 points";
    }