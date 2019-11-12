/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct SolidPrimitivePlacementStrategyTests : public BuildingSharedTestFixtureBase
    {};

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(SolidPrimitivePlacementStrategyTests, CreateExtrusion)
    {
    SolidPrimitivePlacementStrategyPtr sut = ExtrusionPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ISolidPrimitivePtr expectedSolidPrimitive = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(CurveVector::CreateLinear({{0,0,0},{100,0,0},{100,100,0},{0,100,0}}, CurveVector::BOUNDARY_TYPE_Open),
                                                                                                       DVec3d::FromStartEnd(DPoint3d::From(0,100,0), DPoint3d::From(0,50,100)),
                                                                                                       true));

    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({100,0,0});
    sut->AddKeyPoint({100,100,0});
    sut->AddKeyPoint({0,100,0});
    sut->SetProperty(SolidPrimitivePlacementStrategy::prop_BaseComplete(), true);
    
    sut->AddKeyPoint({0,0,100});
    double height;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionPlacementStrategy::prop_Height(), height));
    ASSERT_DOUBLE_EQ(height, 100);
    
    sut->AddKeyPoint({0,50,0});

    ISolidPrimitivePtr solidPrimitive = sut->FinishSolidPrimitive();
    ASSERT_TRUE(solidPrimitive.IsValid());
    ASSERT_TRUE(solidPrimitive->IsSameStructureAndGeometry(*expectedSolidPrimitive));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(SolidPrimitivePlacementStrategyTests, CreateExtrusion_BaseContainsArc)
    {
    SolidPrimitivePlacementStrategyPtr sut = ExtrusionPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ISolidPrimitivePtr expectedSolidPrimitive = ISolidPrimitive::CreateDgnExtrusion(
        DgnExtrusionDetail(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open,
                                {ICurvePrimitive::CreateLineString({{100,100,0}, {0,100,0}, {0,0,0},{300,0,0},{300,100,0},{200,100,0}}),
                                 ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({200,100,0},{150,150,0},{100,100,0}))}),
                           DVec3d::From(0, 0, 100), true));

    sut->AddKeyPoint({100,100,0});
    sut->AddKeyPoint({0,100,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({300,0,0});
    sut->AddKeyPoint({300,100,0});
    sut->AddKeyPoint({200,100,0});
    sut->SetProperty(ExtrusionPlacementStrategy::prop_ContinuousBaseShapePrimitiveComplete(), true);
    sut->SetProperty(ExtrusionPlacementStrategy::prop_BaseShapeStrategy(), BaseShapeStrategyChangeProperty(DefaultNewGeometryType::Arc));
    sut->SetProperty(ExtrusionPlacementStrategy::prop_BaseShapeStrategy(), BaseShapeStrategyChangeProperty(ArcPlacementMethod::StartMidEnd));
    sut->AddKeyPoint({150,150,0});
    sut->AddKeyPoint({100,100,0});

    sut->SetProperty(SolidPrimitivePlacementStrategy::prop_BaseComplete(), true);

    sut->AddKeyPoint({0,0,100});
    double height;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionPlacementStrategy::prop_Height(), height));
    ASSERT_DOUBLE_EQ(height, 100);

    sut->AddKeyPoint({100,100,100});

    ISolidPrimitivePtr solidPrimitive = sut->FinishSolidPrimitive();
    ASSERT_TRUE(solidPrimitive.IsValid());
    ASSERT_TRUE(solidPrimitive->IsSameStructureAndGeometry(*expectedSolidPrimitive));
    }