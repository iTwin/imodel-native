/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/CurveStrategy_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeXml\BeXml.h>
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform\UnitTests\DgnDbTestUtils.h>
#include <DgnPlatform\UnitTests\ScopedDgnHost.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <sstream>
#include <DgnClientFx/DgnClientApp.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"
#include "TestUtils.h"

USING_NAMESPACE_BUILDING_SHARED

//=======================================================================================
// Sets up environment for Curve placement strategy unit testing.
// @bsiclass                                    Haroldas.Vitunskas              12/2017
//=======================================================================================
struct CurveStrategyTests : public BuildingSharedTestFixtureBase
    {
    CurveStrategyTests() {};
    ~CurveStrategyTests() {};
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveStrategyTests, LineByPointsTests)
    {
    LinePointsPlacementStrategyPtr strategy = LinePointsPlacementStrategy::Create();
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create strategy";

    // Check initial state
    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Strategy should be created with no initial points";

    ICurvePrimitivePtr createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 0 points";

    // Try adding key points
    strategy->AddKeyPoint({ 0, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 1 point";

    strategy->AddKeyPoint({ 5, 5, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 5, 5, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsValid()) << "Failed to create curve";

    ICurvePrimitivePtr expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 5, 5, 0 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->AddKeyPoint({ 15, 15, 5 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 5, 5, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->PopKeyPoint();
    TestUtils::ComparePoints({ {0, 0, 0} }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "No curve should be created with 1 point";

    strategy->AddDynamicKeyPoint({ 7, 5, 4 });
    TestUtils::ComparePoints({ {0, 0, 0}, {7, 5, 4} }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 7, 5, 4 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->AddDynamicKeyPoint({ 1, 2, 3 });
    TestUtils::ComparePoints({ { 0, 0, 0 }, { 1, 2, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 1, 2, 3 });
    TestUtils::CompareCurves(createdCurve, expected);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveStrategyTests, LinePointAngleLengthTests)
    {
    LinePointLengthAnglePlacementStrategyPtr strategy = LinePointLengthAnglePlacementStrategy::Create
    (
        DPlane3d::FromOriginAndNormal /*XY plane*/
        (
            DPoint3d::From(0, 0, 0),
            DVec3d::From(0, 0, 1)
        )
    );

    // Check initial state
    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Strategy should be created with no initial points";
    
    double length, angle;
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Length(), length)) << "Initially, length should be accessible";
    ASSERT_EQ(0, length) << "Initial length should be 0";
    
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Angle(), angle)) << "Initially, angle should be accessible";
    ASSERT_EQ(0, angle) << "Initial angle should be 0";

    ICurvePrimitivePtr createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 0 points";
    
    // Try setting properties
    strategy->AddKeyPoint({ 0, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 0, 0, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ICurvePrimitivePtr expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 0, 0, 0 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Length(), 2.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Length(), length)) << "Getting length should not fail";
    ASSERT_DOUBLE_EQ(2.0, length) << "Length is incorrect";
    TestUtils::ComparePoints({ {0, 0, 0},{ 2, 0, 0 } }, strategy->GetKeyPoints());
    
    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 2, 0, 0 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Angle(), msGeomConst_pi / 2);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Angle(), angle)) << "Getting angle should not fail";
    ASSERT_DOUBLE_EQ(msGeomConst_pi / 2, angle) << "Angle is incorrect";
    TestUtils::ComparePoints({ {0, 0, 0}, {0, 2, 0} }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 0, 2, 0 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->PopKeyPoint();
    ASSERT_EQ(0, strategy->GetKeyPoints().size()); // popping the key point should also pop the generated point
    
    strategy->AddDynamicKeyPoint({ 1, 2, 3 });
    TestUtils::ComparePoints({ { 1, 2, 3 },{ 1, 4, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 4, 3 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Length(), 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Length(), length)) << "Getting length should not fail";
    ASSERT_DOUBLE_EQ(5.0, length) << "Length is incorrect";
    TestUtils::ComparePoints({ { 1, 2, 3 },{ 1, 7, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 7, 3 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Angle(), msGeomConst_pi / 4);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Angle(), angle)) << "Getting angle should not fail";
    ASSERT_DOUBLE_EQ(msGeomConst_pi / 4, angle) << "Angle is incorrect";
    TestUtils::ComparePoints({ { 1, 2, 3 },{ 1 + std::sqrt(25.0/2.0), 2 + std::sqrt(25.0/2.0), 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1 + sqrt(25.0 / 2.0), 2 + sqrt(25.0 / 2.0), 3 });
    TestUtils::CompareCurves(createdCurve, expected);

    DPlane3d otherPlane = DPlane3d::FromOriginAndNormal
    (
        DPoint3d::From(0, 0, 0),
        DVec3d::From(1, 0, 0)
    );

    strategy->SetProperty(strategy->prop_WorkingPlane(), otherPlane);
    TestUtils::ComparePoints({ { 1, 2, 3 },{ 1, 2 + sqrt(25.0 / 2.0), 3 - sqrt(25.0 / 2.0) } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 2 + sqrt(25.0 / 2.0), 3 - sqrt(25.0 / 2.0) });
    TestUtils::CompareCurves(createdCurve, expected);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveStrategyTests, LinePointsLengthTests)
    {
    LinePointsLengthPlacementStrategyPtr strategy = LinePointsLengthPlacementStrategy::Create();

    // Check initial state
    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Strategy should be created with no initial points";

    double length;
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Length(), length)) << "Initially, length should be accessible";
    ASSERT_EQ(0, length) << "Initial length should be 0";

    ICurvePrimitivePtr createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 0 points";

    // Try setting points and properties
    strategy->AddKeyPoint({ 0, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "Curve should not be created using only one point";

    strategy->AddKeyPoint({ 1, 2, 3 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 0, 0, 0 } }, strategy->GetKeyPoints());
    createdCurve = strategy->FinishPrimitive();
    ICurvePrimitivePtr expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 0, 0, 0 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Length(), 2.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Length(), length)) << "Getting length should not fail";
    ASSERT_DOUBLE_EQ(2.0, length) << "Length is incorrect";
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 2.0 / std::sqrt(14.0), 4.0 / std::sqrt(14.0), 6.0 / std::sqrt(14.0) } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 2.0 / std::sqrt(14.0), 4.0 / std::sqrt(14.0), 6.0 / std::sqrt(14.0) });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->PopKeyPoint();
    strategy->PopKeyPoint();
    ASSERT_EQ(0, strategy->GetKeyPoints().size());

    strategy->AddKeyPoint({ 1, 2, 3 });
    TestUtils::ComparePoints({ { 1, 2, 3 } }, strategy->GetKeyPoints());

    strategy->AddDynamicKeyPoint({ 1, 22, 3 });
    TestUtils::ComparePoints({ {1, 2, 3}, {1, 4, 3} }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 4, 3 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Length(), 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Length(), length)) << "Getting length should not fail";
    ASSERT_DOUBLE_EQ(5.0, length) << "Length is incorrect";
    TestUtils::ComparePoints({ { 1, 2, 3 },{ 1, 7, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 7, 3 });
    TestUtils::CompareCurves(createdCurve, expected);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveStrategyTests, LinePointsAngleTests)
    {
    LinePointsAnglePlacementStrategyPtr strategy = LinePointsAnglePlacementStrategy::Create
    (
        DPlane3d::FromOriginAndNormal
        (
            DPoint3d::From(0, 0, 0),
            DVec3d::From(0, 0, 1)
        )
    );

    // Check initial state
    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Strategy should be created with no initial points";

    double angle;
    
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Angle(), angle)) << "Initially, angle should be accessible";
    ASSERT_EQ(0, angle) << "Initial angle should be 0";

    ICurvePrimitivePtr createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 0 points";

    // Try setting properties
    strategy->AddKeyPoint({ 0, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "Curve shouldn't be created with 0 points";

    strategy->AddKeyPoint({ 1, 2, 3 });
    TestUtils::ComparePoints({ { 0, 0, 0 }, {std::sqrt(14.0), 0, 0} }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ICurvePrimitivePtr expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { std::sqrt(14.0), 0, 0 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Angle(), msGeomConst_pi / 2);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Angle(), angle)) << "Getting angle should not fail";
    ASSERT_DOUBLE_EQ(msGeomConst_pi / 2, angle) << "Angle is incorrect";
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 0, std::sqrt(14.0), 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 0, std::sqrt(14.0), 0 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->PopKeyPoint();
    strategy->PopKeyPoint();
    ASSERT_EQ(0, strategy->GetKeyPoints().size());

    strategy->AddKeyPoint({ 1, 2, 3 });
    TestUtils::ComparePoints({ { 1, 2, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "Curve shouldn't be created with 0 points";

    strategy->AddDynamicKeyPoint({ 2, 2, 2 });
    TestUtils::ComparePoints({ { 1, 2, 3 }, {1, 2.0 + std::sqrt(2.0), 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();

    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 2.0 + std::sqrt(2.0), 3 });
    TestUtils::CompareCurves(createdCurve, expected);

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 2.0 + std::sqrt(2.0), 3 });
    TestUtils::CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Angle(), msGeomConst_pi / 4);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Angle(), angle)) << "Getting angle should not fail";
    ASSERT_DOUBLE_EQ(msGeomConst_pi / 4, angle) << "Angle is incorrect";
    TestUtils::ComparePoints({ { 1, 2, 3 },{ 2, 3, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 2, 3, 3 });
    TestUtils::CompareCurves(createdCurve, expected);

    DPlane3d otherPlane = DPlane3d::FromOriginAndNormal
    (
        DPoint3d::From(0, 0, 0),
        DVec3d::From(1, 0, 0)
    );

    strategy->SetProperty(strategy->prop_WorkingPlane(), otherPlane);
    TestUtils::ComparePoints({ { 1, 2, 3 },{ 1, 3, 2 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 3, 2 });
    TestUtils::CompareCurves(createdCurve, expected);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveStrategyTests, SplineThroughPointsStrategyTests)
    {
    SplineThroughPointsPlacementStrategyPtr strategy = SplineThroughPointsPlacementStrategy::Create();
    ASSERT_TRUE(strategy.IsValid()) << "strategy creation should not fail";

    // Check initial strategy state
    DVec3d startTangent = DVec3d::From(1, 0, 0), endTangent = DVec3d::From(1, 0, 0);
    ASSERT_TRUE(startTangent.AlmostEqual(strategy->GetStartTangent())) << "Start tangent is incorrect";
    ASSERT_TRUE(endTangent.AlmostEqual(strategy->GetEndTangent())) << "End tangent is incorrect";

    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Strategy should have no points";
    ASSERT_TRUE(strategy->FinishPrimitive().IsNull()) << "No curve should be created with 0 points";

    // Try adding points
    strategy->AddKeyPoint({ 0, 0, 0 });
    TestUtils::ComparePoints({ {0, 0, 0} }, strategy->GetKeyPoints());
    ASSERT_TRUE(strategy->FinishPrimitive().IsNull()) << "No curve should be created with 0 points";

    strategy->AddKeyPoint({ 1, 0, 0 });
    TestUtils::ComparePoints({ {0, 0, 0}, {1, 0, 0} }, strategy->GetKeyPoints());

    ICurvePrimitivePtr expectedCurve = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 0, 0 } });
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    strategy->AddKeyPoint({ 0, 1, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 }, {0, 1, 0} }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 } });
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    strategy->AddKeyPoint({ 2, 5, 6 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 }, {2, 5, 6} }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 },{ 2, 5, 6 } });
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    startTangent = DVec3d::From(1, 2, 3);
    strategy->SetStartTangent(startTangent);
    startTangent.Normalize();
    ASSERT_TRUE(startTangent.AlmostEqual(strategy->GetStartTangent())) << "Start tangent is incorrect";

    expectedCurve = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 },{ 2, 5, 6 } }, startTangent);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    endTangent = DVec3d::From(3, 2, 1);
    endTangent.Normalize();
    strategy->SetEndTangent(endTangent);
    ASSERT_TRUE(endTangent.AlmostEqual(strategy->GetEndTangent())) << "End tangent is incorrect";

    expectedCurve = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 },{ 2, 5, 6 } }, startTangent, endTangent);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    startTangent.Zero();
    startTangent.Normalize();
    strategy->RemoveStartTangent();
    ASSERT_TRUE(startTangent.AlmostEqual(strategy->GetStartTangent())) << "Start tangent is incorrect";

    expectedCurve = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 },{ 2, 5, 6 } }, startTangent, endTangent);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    strategy = SplineThroughPointsPlacementStrategy::Create();
    ASSERT_TRUE(strategy.IsValid()) << "strategy creation should not fail";

    strategy->AddKeyPoint({ 0, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 } }, strategy->GetKeyPoints());
    ASSERT_TRUE(strategy->FinishPrimitive().IsNull()) << "No curve should be created with 0 points";

    strategy->AddKeyPoint({ 1, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 } }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 0, 0 } });
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    strategy->AddDynamicKeyPoint({ 1, 2, 3 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 2, 3 } }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 2, 3 } });
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    strategy->AddDynamicKeyPoint({ 5, 6, 7 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 },{ 5, 6, 7 } }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 0, 0 },{ 5, 6, 7 } });
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveStrategyTests, SplineControlPointsStrategyTests)
    {
    int order = 3;

    SplineControlPointsPlacementStrategyPtr strategy = SplineControlPointsPlacementStrategy::Create(order);
    ASSERT_TRUE(strategy.IsValid()) << "strategy creation should not fail";

    // Check initial strategy state
    int actualOrder;
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(SplineControlPointsPlacementStrategy::prop_Order(), actualOrder)) << "Getting order should not fail";
    ASSERT_EQ(order, actualOrder) << "Incorrect strategy order";

    ASSERT_EQ(0, strategy->GetKeyPoints().size()) << "Strategy should have no points";
    ASSERT_TRUE(strategy->FinishPrimitive().IsNull()) << "No curve should be created with 0 points";

    // Try adding points
    strategy->AddKeyPoint({ 0, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 } }, strategy->GetKeyPoints());
    ASSERT_TRUE(strategy->FinishPrimitive().IsNull()) << "No curve should be created with 0 points";

    strategy->AddKeyPoint({ 1, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 } }, strategy->GetKeyPoints());

    ICurvePrimitivePtr expectedCurve = TestUtils::CreateSpline({ { 0, 0, 0 },{ 1, 0, 0 } }, order);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    strategy->AddKeyPoint({ 0, 1, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 } }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateSpline({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 } }, order);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    strategy->AddKeyPoint({ 2, 5, 6 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 },{ 2, 5, 6 } }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateSpline({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 },{ 2, 5, 6 } }, order);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    order = 4;
    strategy->SetProperty(SplineControlPointsPlacementStrategy::prop_Order(), order);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(SplineControlPointsPlacementStrategy::prop_Order(), actualOrder)) << "Getting order should not fail";
    ASSERT_EQ(order, actualOrder) << "Incorrect strategy order";

    expectedCurve = TestUtils::CreateSpline({ { 0, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 },{ 2, 5, 6 } }, order);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    order = 0;
    strategy->SetProperty(SplineControlPointsPlacementStrategy::prop_Order(), order);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(SplineControlPointsPlacementStrategy::prop_Order(), actualOrder)) << "Getting order should not fail";
    ASSERT_EQ(order, actualOrder) << "Incorrect strategy order";

    ASSERT_TRUE(strategy->FinishPrimitive().IsNull()) << "No curve should be created with invalid order";

    order = 3;
    strategy = SplineControlPointsPlacementStrategy::Create(order);
    ASSERT_TRUE(strategy.IsValid()) << "strategy creation should not fail";

    strategy->AddKeyPoint({ 0, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 } }, strategy->GetKeyPoints());
    ASSERT_TRUE(strategy->FinishPrimitive().IsNull()) << "No curve should be created with 0 points";

    strategy->AddKeyPoint({ 1, 0, 0 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 } }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateSpline({ { 0, 0, 0 },{ 1, 0, 0 } }, order);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    strategy->AddDynamicKeyPoint({ 1, 2, 3 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 2, 3 } }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateSpline({ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 2, 3 } }, order);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());

    strategy->AddDynamicKeyPoint({ 5, 6, 7 });
    TestUtils::ComparePoints({ { 0, 0, 0 },{ 1, 0, 0 },{ 5, 6, 7 } }, strategy->GetKeyPoints());
    expectedCurve = TestUtils::CreateSpline({ { 0, 0, 0 },{ 1, 0, 0 },{ 5, 6, 7 } }, order);
    TestUtils::CompareCurves(expectedCurve, strategy->FinishPrimitive());
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveStrategyTests, SplineStrategiesCreationTest) 
    {
    SplinePlacementStrategyType typeC = SplinePlacementStrategyType::ControlPoints;
    SplineManipulationStrategyPtr controlStrategy = SplineManipulationStrategy::Create(typeC);
    ASSERT_TRUE(controlStrategy.IsValid()) << "control points strategy creation should not fail";
    ASSERT_EQ(SplinePlacementStrategyType::ControlPoints, controlStrategy->GetType()) << "Failed to get Control points Type";
    SplinePlacementStrategyPtr cPlacement = controlStrategy->CreatePlacement();
    ASSERT_TRUE(cPlacement.IsValid()) << "control points strategy failed to create";
    ASSERT_EQ(SplinePlacementStrategyType::ControlPoints, cPlacement->GetType()) << "Placement Strategy type should be Control Points";

    SplinePlacementStrategyType typeP = SplinePlacementStrategyType::ThroughPoints;
    SplineManipulationStrategyPtr pointsStrategy = SplineManipulationStrategy::Create(typeP);
    ASSERT_TRUE(pointsStrategy.IsValid()) << "through points strategy creation should not fail";
    ASSERT_EQ(SplinePlacementStrategyType::ThroughPoints, pointsStrategy->GetType()) << "Failed to get Through points Type";
    SplinePlacementStrategyPtr pPlacement = pointsStrategy->CreatePlacement();
    ASSERT_TRUE(pPlacement.IsValid()) << "through points strategy failed to create";
    ASSERT_EQ(SplinePlacementStrategyType::ThroughPoints, pPlacement->GetType()) << "Placement Strategy type should be Through Points";
    }