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

void CompareCurves(ICurvePrimitivePtr lhs, ICurvePrimitivePtr rhs)
    {
    ASSERT_EQ(lhs.IsValid(), rhs.IsValid()) << "Both curves should be either valid or invalid";
    ASSERT_TRUE(lhs->IsSameStructureAndGeometry(*rhs, 0.1)) << "Curves should have same geometry";
    }

void ComparePoints(bvector<DPoint3d> lhs, bvector<DPoint3d> rhs)
    {
    ASSERT_EQ(lhs.size(), rhs.size()) << "There should be an equal amount of points";
    for (size_t i = 0; i < lhs.size(); ++i)
        {
        ASSERT_TRUE(lhs[i].AlmostEqual(rhs[i])) << "Points should be almost equal";
        }
    }

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
    ComparePoints({ { 0, 0, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 1 point";

    strategy->AddKeyPoint({ 5, 5, 0 });
    ComparePoints({ { 0, 0, 0 },{ 5, 5, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsValid()) << "Failed to create curve";

    ICurvePrimitivePtr expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 5, 5, 0 });
    CompareCurves(createdCurve, expected);

    strategy->AddKeyPoint({ 15, 15, 5 });
    ComparePoints({ { 0, 0, 0 },{ 5, 5, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    CompareCurves(createdCurve, expected);

    strategy->PopKeyPoint();
    ComparePoints({ {0, 0, 0} }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "No curve should be created with 1 point";

    strategy->AddDynamicKeyPoint({ 7, 5, 4 });
    ComparePoints({ {0, 0, 0}, {7, 5, 4} }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 7, 5, 4 });
    CompareCurves(createdCurve, expected);

    strategy->AddDynamicKeyPoint({ 1, 2, 3 });
    ComparePoints({ { 0, 0, 0 }, { 1, 2, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 1, 2, 3 });
    CompareCurves(createdCurve, expected);
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
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Length, length)) << "Initially, length should be accessible";
    ASSERT_EQ(0, length) << "Initial length should be 0";
    
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Angle, angle)) << "Initially, angle should be accessible";
    ASSERT_EQ(0, angle) << "Initial angle should be 0";

    ICurvePrimitivePtr createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 0 points";
    
    // Try setting properties
    strategy->AddKeyPoint({ 0, 0, 0 });
    ComparePoints({ { 0, 0, 0 },{ 0, 0, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ICurvePrimitivePtr expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 0, 0, 0 });
    CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Length, 2.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Length, length)) << "Getting length should not fail";
    ASSERT_DOUBLE_EQ(2.0, length) << "Length is incorrect";
    ComparePoints({ {0, 0, 0},{ 2, 0, 0 } }, strategy->GetKeyPoints());
    
    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 2, 0, 0 });
    CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Angle, msGeomConst_pi / 2);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Angle, angle)) << "Getting angle should not fail";
    ASSERT_DOUBLE_EQ(msGeomConst_pi / 2, angle) << "Angle is incorrect";
    ComparePoints({ {0, 0, 0}, {0, 2, 0} }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 0, 2, 0 });
    CompareCurves(createdCurve, expected);

    strategy->PopKeyPoint();
    ASSERT_EQ(0, strategy->GetKeyPoints().size()); // popping the key point should also pop the generated point
    
    strategy->AddDynamicKeyPoint({ 1, 2, 3 });
    ComparePoints({ { 1, 2, 3 },{ 1, 4, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 4, 3 });
    CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Length, 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Length, length)) << "Getting length should not fail";
    ASSERT_DOUBLE_EQ(5.0, length) << "Length is incorrect";
    ComparePoints({ { 1, 2, 3 },{ 1, 7, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 7, 3 });
    CompareCurves(createdCurve, expected);

    strategy->SetProperty(LinePlacementStrategy::prop_Angle, msGeomConst_pi / 4);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(LinePlacementStrategy::prop_Angle, angle)) << "Getting angle should not fail";
    ASSERT_DOUBLE_EQ(msGeomConst_pi / 4, angle) << "Angle is incorrect";
    ComparePoints({ { 1, 2, 3 },{ 1 + std::sqrt(25.0/2.0), 2 + std::sqrt(25.0/2.0), 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1 + sqrt(25.0 / 2.0), 2 + sqrt(25.0 / 2.0), 3 });
    CompareCurves(createdCurve, expected);

    DPlane3d otherPlane = DPlane3d::FromOriginAndNormal
    (
        DPoint3d::From(0, 0, 0),
        DVec3d::From(1, 0, 0)
    );

    strategy->SetWorkingPlane(otherPlane);
    ComparePoints({ { 1, 2, 3 },{ 1, 2 + sqrt(25.0 / 2.0), 3 - sqrt(25.0 / 2.0) } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 2 + sqrt(25.0 / 2.0), 3 - sqrt(25.0 / 2.0) });
    CompareCurves(createdCurve, expected);
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
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(BUILDINGSHARED_PROP_Length, length)) << "Initially, length should be accessible";
    ASSERT_EQ(0, length) << "Initial length should be 0";

    ICurvePrimitivePtr createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "no curve should be created with 0 points";

    // Try setting points and properties
    strategy->AddKeyPoint({ 0, 0, 0 });
    ComparePoints({ { 0, 0, 0 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    ASSERT_TRUE(createdCurve.IsNull()) << "Curve should not be created using only one point";

    strategy->AddKeyPoint({ 1, 2, 3 });
    ComparePoints({ { 0, 0, 0 },{ 0, 0, 0 } }, strategy->GetKeyPoints());
    createdCurve = strategy->FinishPrimitive();
    ICurvePrimitivePtr expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 0, 0, 0 });
    CompareCurves(createdCurve, expected);

    strategy->SetProperty(BUILDINGSHARED_PROP_Length, 2.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(BUILDINGSHARED_PROP_Length, length)) << "Getting length should not fail";
    ASSERT_DOUBLE_EQ(2.0, length) << "Length is incorrect";
    ComparePoints({ { 0, 0, 0 },{ 2.0 / std::sqrt(14.0), 4.0 / std::sqrt(14.0), 6.0 / std::sqrt(14.0) } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 0, 0, 0 }, { 2.0 / std::sqrt(14.0), 4.0 / std::sqrt(14.0), 6.0 / std::sqrt(14.0) });
    CompareCurves(createdCurve, expected);

    strategy->PopKeyPoint();
    strategy->PopKeyPoint();
    ASSERT_EQ(0, strategy->GetKeyPoints().size());

    strategy->AddKeyPoint({ 1, 2, 3 });
    ComparePoints({ { 1, 2, 3 } }, strategy->GetKeyPoints());

    strategy->AddDynamicKeyPoint({ 1, 22, 3 });
    ComparePoints({ {1, 2, 3}, {1, 4, 3} }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 4, 3 });
    CompareCurves(createdCurve, expected);

    strategy->SetProperty(BUILDINGSHARED_PROP_Length, 5.0);
    ASSERT_EQ(BentleyStatus::SUCCESS, strategy->TryGetProperty(BUILDINGSHARED_PROP_Length, length)) << "Getting length should not fail";
    ASSERT_DOUBLE_EQ(5.0, length) << "Length is incorrect";
    ComparePoints({ { 1, 2, 3 },{ 1, 7, 3 } }, strategy->GetKeyPoints());

    createdCurve = strategy->FinishPrimitive();
    expected = ICurvePrimitive::CreateLine({ 1, 2, 3 }, { 1, 7, 3 });
    CompareCurves(createdCurve, expected);
    }